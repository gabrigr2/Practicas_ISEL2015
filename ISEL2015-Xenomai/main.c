#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <wiringPi.h>
#include "fsm.h"
#include <stdio.h>
#include <task.h>

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
    pthread_mutex_unlock (&mutex_acumulado);

    //Si acumulado es suficiente cambia de estado
    if((acumulado >= PRECIOCAFE)&&(ret == 1)){
      ret = 1;
      pthread_mutex_lock (&mutex_puedoDevolver);
      puedoDevolver=1; //Devuelve dinero una vez el boton ha sido pulsado y tenemos dinero suficiente
      pthread_mutex_unlock (&mutex_puedoDevolver);
    }else{
      ret = 0;
    } 
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
    //printf("CUP \n");
}

static void coffee (fsm_t* this)
{
  digitalWrite (GPIO_CUP, LOW);
  digitalWrite (GPIO_COFFEE, HIGH);
  timer_start (COFFEE_TIME);
     //printf("COFFEE \n");
}

static void milk (fsm_t* this)
{
  digitalWrite (GPIO_COFFEE, LOW);
  digitalWrite (GPIO_MILK, HIGH);
  timer_start (MILK_TIME);
     //printf("MILK \n");
}

static void finish (fsm_t* this)
{
  digitalWrite (GPIO_MILK, LOW);
  digitalWrite (GPIO_LED, HIGH);
     //printf("FINISH \n");
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
    pthread_mutex_lock (&mutex_acumulado);
    acumulado+=moneda;
    pthread_mutex_unlock (&mutex_acumulado);
    
    //Si ha terminado de servir el cafe pasamos a devolver
    //en caso contrario seguimos en este estado.
    if((puedoDevolver==1)){
        pthread_mutex_lock (&mutex_puedoDevolver);
        puedoDevolver=0;
        pthread_mutex_unlock (&mutex_puedoDevolver);

        return 1;
    }else{
      return 0;
    }    
}

static void devolver (fsm_t* this){
    //int devuelto = acumulado - PRECIOCAFE;//sacar las monedas
    //printf("Devuelto %d \n",devuelto);
    pthread_mutex_lock (&mutex_acumulado);
    acumulado=0;
    pthread_mutex_unlock (&mutex_acumulado);

}

//Transiciones de la máquina de estados del monedero
static fsm_trans_t monedero[] = {
  { MONEDERO, calcula_valor, MONEDERO, devolver},
  {-1, NULL, -1, NULL },
};


// Utility functions, should be elsewhere

// res = a - b
void
timeval_sub (struct timeval *res, struct timeval *a, struct timeval *b)
{
  res->tv_sec = a->tv_sec - b->tv_sec;
  res->tv_usec = a->tv_usec - b->tv_usec;
  if (res->tv_usec < 0) {
    --res->tv_sec;
    res->tv_usec += 1000000;
  }
}

// res = a + b
void
timeval_add (struct timeval *res, struct timeval *a, struct timeval *b)
{
  res->tv_sec = a->tv_sec + b->tv_sec
    + a->tv_usec / 1000000 + b->tv_usec / 1000000; 
  res->tv_usec = a->tv_usec % 1000000 + b->tv_usec % 1000000;
}

// wait until next_activation (absolute time)
void delay_until (struct timeval* next_activation)
{
  struct timeval now, timeout;
  gettimeofday (&now, NULL);
  timeval_sub (&timeout, next_activation, &now);
  select (0, NULL, NULL, NULL, &timeout);
}

static void * tarea_cafe (void* arg)
{
  struct timeval next_activation;
  struct timeval now, timeout;
  gettimeofday (&next_activation, NULL);
  while (1) 
  {
    struct timeval *period = task_get_period (pthread_self());
    timeval_add (&next_activation, &next_activation, period);
    gettimeofday (&now, NULL);
    timeval_sub (&timeout, &next_activation, &now);
    select (0, NULL, NULL, NULL, &timeout) ;

    fsm_fire(cofm_fsm);
  }  
}

static void * tarea_monedero (void* arg)
{

  struct timeval next_activation;
  struct timeval now, timeout;
  gettimeofday (&next_activation, NULL);
  while (1) 
  {
    struct timeval *period = task_get_period (pthread_self());
    timeval_add (&next_activation, &next_activation, period);
    gettimeofday (&now, NULL);
    timeval_sub (&timeout, &next_activation, &now);
    select (0, NULL, NULL, NULL, &timeout) ;

    fsm_fire(monfm_fsm);
  }
  
}

int main ()
{

  struct timeval clk_period = { 0, 250 * 1000 };
  struct timeval next_activation;
  fsm_t* cofm_fsm = fsm_new (cofm);
  fsm_t* monedero_fsm = fsm_new (monedero);

  wiringPiSetup();

  gettimeofday (&next_activation, NULL);

  init_mutex (&mutex_acumulado, 2);
  init_mutex (&mutex_puedoDevolver, 2);

  pthread_t tcafe = task_new ("tarea_cafe", tarea_cafe, SECONDARY_PERIOD_1, SECONDARY_PERIOD_1, 2, 1024);
  pthread_t tmonedero = task_new ("tarea_monedero", tarea_monedero, SECONDARY_PERIOD_2, SECONDARY_PERIOD_2, 1, 1024);
  
  //Mientras tengamos 3 dastos en el fichero de inputs
  while (scanf("%d %d %d", &button, &moneda, &timer)==3) {
   
    timeval_add (&next_activation, &next_activation, &clk_period);
    delay_until (&next_activation);
  }

  return 0;
}
