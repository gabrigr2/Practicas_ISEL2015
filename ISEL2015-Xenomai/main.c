#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <wiringPi.h>
#include "fsm.h"
#include <stdio.h>
#include "tasks.h"

#define GPIO_BUTTON	2
#define GPIO_LED	3
#define GPIO_CUP	4
#define GPIO_COFFEE	5
#define GPIO_MILK	6
//Añado GPIO para detectar que se ha introducido una moneda
#define GPIO_MONEDA 7

#define CUP_TIME	250
#define COFFEE_TIME	3000
#define MILK_TIME	3000

//Fijamos el precio del cafe
#define PRECIOCAFE	50

//Definimos los periodos en nanosegundos
#define SECONDARY_PERIOD_1 400000000
#define SECONDARY_PERIOD_2 800000000

//Variables compartidas
static int acumulado = 0;
static int puedoDevolver=0;

//Variables de tiempo
static struct timespec start, stop;
int timediff = 0;

//Definicion maquinas de estado
fsm_t* cofm_fsm;
fsm_t* monedero_fsm;

//Threads de las variables compartidas
static pthread_mutex_t mutex_acumulado, mutex_puedoDevolver;

//Definimos los estados de la maquina cafe
enum cofm_state {
  COFM_WAITING,
  COFM_CUP,
  COFM_COFFEE,
  COFM_MILK,
};

//Definimos los estados de la maquina monedero
enum monedero_state {
  MONEDERO,
};

static int button = 0; //global
//static void button_isr (void) { button = 1; }
static int timer = 0; //global
static void timer_isr (union sigval arg) { timer = 1; }

static int moneda=0;

static void timer_start (int ms)
{
  timer_t timerid;
  struct itimerspec value;
  struct sigevent se;
  se.sigev_notify = SIGEV_THREAD;
  se.sigev_value.sival_ptr = &timerid;
  se.sigev_notify_function = timer_isr;
  se.sigev_notify_attributes = NULL;
  value.it_value.tv_sec = ms / 1000;
  value.it_value.tv_nsec = (ms % 1000) * 1000000;
  value.it_interval.tv_sec = 0;
  value.it_interval.tv_nsec = 0;
  timer_create (CLOCK_REALTIME, &se, &timerid);
  timer_settime (timerid, 0, &value, NULL);
}


static int button_pressed (fsm_t* this)
{
  int ret = button;
    button=0;
    pthread_mutex_lock (&mutex_puedoDevolver);
    puedoDevolver=0;
    pthread_mutex_unlock (&mutex_puedoDevolver);


    pthread_mutex_lock (&mutex_acumulado);
    //Si acumulado es suficiente cambia de estado
    if((acumulado >= PRECIOCAFE)&&(ret == 1)){
      ret = 1;
      pthread_mutex_lock (&mutex_puedoDevolver);
      puedoDevolver=1; //Devuelve dinero una vez el boton ha sido pulsado y tenemos dinero suficiente
      pthread_mutex_unlock (&mutex_puedoDevolver);
    }else{
      ret = 0;
    }
    pthread_mutex_unlock (&mutex_acumulado); 
  return ret;
}

static int timer_finished (fsm_t* this)
{
  int ret = timer;
  timer = 0;
  return ret;
}

static void cup (fsm_t* this)
{
  digitalWrite (GPIO_LED, LOW);
  digitalWrite (GPIO_CUP, HIGH);
  timer_start (CUP_TIME);
    printf("CUP \n");
}

static void coffee (fsm_t* this)
{
  digitalWrite (GPIO_CUP, LOW);
  digitalWrite (GPIO_COFFEE, HIGH);
  timer_start (COFFEE_TIME);
     printf("COFFEE \n");
}

static void milk (fsm_t* this)
{
  digitalWrite (GPIO_COFFEE, LOW);
  digitalWrite (GPIO_MILK, HIGH);
  timer_start (MILK_TIME);
     printf("MILK \n");
}

static void finish (fsm_t* this)
{
  digitalWrite (GPIO_MILK, LOW);
  digitalWrite (GPIO_LED, HIGH);
     printf("FINISH \n");
}

// Explicit FSM description
static fsm_trans_t cofm[] = {
  { COFM_WAITING, button_pressed, COFM_CUP,     cup    },
  { COFM_CUP,     timer_finished, COFM_COFFEE,  coffee },
  { COFM_COFFEE,  timer_finished, COFM_MILK,    milk   },
  { COFM_MILK,    timer_finished, COFM_WAITING, finish },
  {-1, NULL, -1, NULL },
};

static int calcula_valor (fsm_t* this)
{
    clock_gettime(CLOCK_MONOTONIC, &start); 
    pthread_mutex_lock (&mutex_acumulado);
    acumulado+=moneda;
    pthread_mutex_unlock (&mutex_acumulado);
    clock_gettime(CLOCK_MONOTONIC, &stop); 
    timediff = stop.tv_nsec-start.tv_nsec;
    printf("%d \n", timediff);

    //Si ha terminado de servir el cafe pasamos a devolver
    //en caso contrario seguimos en este estado.
    pthread_mutex_lock (&mutex_puedoDevolver);
    if((puedoDevolver==1)){
        puedoDevolver=0;
        return 1;
    }
    pthread_mutex_unlock (&mutex_puedoDevolver);
    return 0; 
}

static void devolver (fsm_t* this){
    pthread_mutex_lock (&mutex_acumulado);
    int devuelto = acumulado - PRECIOCAFE;//sacar las monedas
    pthread_mutex_unlock (&mutex_acumulado);
    printf("Devuelto %d \n",devuelto);
    pthread_mutex_lock (&mutex_acumulado);
    acumulado=0;
    pthread_mutex_unlock (&mutex_acumulado);

}

//Transiciones de la máquina de estados del monedero
static fsm_trans_t monedero[] = {
  { MONEDERO, calcula_valor, MONEDERO, devolver},
  {-1, NULL, -1, NULL },
};

// wait until next_activation (absolute time)
void delay_until (struct timeval* next_activation)
{
  struct timeval now, timeout;
  gettimeofday (&now, NULL);
  timeval_sub (&timeout, next_activation, &now);
  select (0, NULL, NULL, NULL, &timeout);
}

static void tarea_cafe (void* arg)
{
    static const struct timeval period = { 0,  SECONDARY_PERIOD_1/1000};
    fsm_fire(cofm_fsm);
}

static void tarea_monedero (void* arg)
{
    static const struct timeval period = {0, SECONDARY_PERIOD_2/1000 };
    fsm_fire(monedero_fsm);  
}

int main ()
{

  pthread_t tcafe;
  pthread_t tmonedero;

  wiringPiSetup();

  init_mutex (&mutex_acumulado, 2);
  init_mutex (&mutex_puedoDevolver, 2);

  pinMode (GPIO_COFFEE, OUTPUT);
  pinMode (GPIO_MILK, OUTPUT);
  pinMode (GPIO_LED, OUTPUT);
  digitalWrite (GPIO_LED, HIGH);
  
  cofm_fsm = fsm_new (cofm);
  monedero_fsm = fsm_new (monedero);

  //Mientras tengamos 3 dastos en el fichero de inputs
  while (scanf("%d %d %d", &button, &moneda, &timer)==3) {
   
    clock_gettime(CLOCK_REALTIME, &start);
    create_task (&tcafe, tarea_cafe, NULL, SECONDARY_PERIOD_1/1000, 2, 1024);
    create_task (&tmonedero, tarea_monedero, NULL, SECONDARY_PERIOD_2/1000, 1, 1024);
    clock_gettime(CLOCK_REALTIME, &stop);
    timediff = stop.tv_nsec - start.tv_nsec;
    printf("%d \n",timediff);
    }

  return 0;
}
