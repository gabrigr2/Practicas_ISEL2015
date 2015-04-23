#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <reactor.h>
#include <signal.h>
#include <stdio.h>
#include <wiringPi.h>
#include "fsm.h"

#define GPIO_BUTTON 2
#define GPIO_LED  3
#define GPIO_CUP  4
#define GPIO_COFFEE 5
#define GPIO_MILK 6
//Añado GPIO para detectar que se ha introducido una moneda
#define GPIO_MONEDA 7

#define CUP_TIME  250
#define COFFEE_TIME 3000
#define MILK_TIME 3000

//Fijamos el precio del cafe
#define PRECIOCAFE  50

//Definimos los periodos en nanosegundos
#define SECONDARY_PERIOD_1 400000000
#define SECONDARY_PERIOD_2 800000000

//Variables de tiempo
static struct timespec start, stop;
int timediff=0;

//Punteros maquinas de estado
fsm_t* cofm_fsm;
fsm_t* monedero_fsm;

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

static int acumulado = 0; //global
static int moneda=0;
static int puedoDevolver=0;

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
    puedoDevolver=0;
    //Si acumulado es suficiente cambia de estado
    if((acumulado >= PRECIOCAFE)&&(ret == 1)){
      ret = 1;
      puedoDevolver=1; //Devuelve dinero una vez el boton ha sido pulsado y tenemos dinero suficiente
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
    acumulado+=moneda;
    
    //Si ha terminado de servir el cafe pasamos a devolver
    //en caso contrario seguimos en este estado.
    if((puedoDevolver==1)){
        puedoDevolver=0;
        return 1;
    }else{
      return 0;
    }    
}

static void devolver (fsm_t* this){
    //int devuelto = acumulado - PRECIOCAFE;//sacar las monedas
    //printf("Devuelto %d \n",devuelto);
    acumulado=0;
    
}

//Transiciones de la máquina de estados del monedero
static fsm_trans_t monedero[] = {
  { MONEDERO, calcula_valor, MONEDERO, devolver},
  {-1, NULL, -1, NULL },
};



static void tarea_cafe (EventHandler* eh)
{
  //printf("Entro a tarea cafe \n");
  static struct timeval period = { 0, (SECONDARY_PERIOD_1/1000) };
  clock_gettime(CLOCK_REALTIME, &start);
  fsm_fire(cofm_fsm);
  clock_gettime(CLOCK_REALTIME, &stop);
  timediff = stop.tv_nsec-start.tv_nsec;
  printf("%d \n", timediff);
  timeval_add (&eh->next_activation, &eh->next_activation, &period);
  //printf("Salgo de tarea cafe \n");
}

static void tarea_monedero (EventHandler* eh)
{
  //printf("Entro de tarea monedero \n");
  static struct timeval period = { 0, (SECONDARY_PERIOD_2/1000) }; 
  clock_gettime(CLOCK_REALTIME, &start);
  fsm_fire(monedero_fsm);
  clock_gettime(CLOCK_REALTIME, &stop);
  timediff = stop.tv_nsec-start.tv_nsec;
  printf("%d \n", timediff);
  timeval_add (&eh->next_activation, &eh->next_activation, &period);
  //printf("Salgo de tarea monedero \n");
}

int
main (int argc, char *argv[])
{
  //printf("Entro al main \n");
  /*
  int cnt = 0;
  double timediff_saved [10];
  int i;

  for(i=0; i<10 ; i++){
     timediff_saved[i]=0;
  }
  */

  cofm_fsm = fsm_new (cofm);
  monedero_fsm = fsm_new (monedero);

  EventHandler tmonedero, tcafe;

  //wiringPiSetup () ;
  //pinMode (0, OUTPUT) ;

  reactor_init ();

  event_handler_init (&tmonedero, 1, (eh_func_t) tarea_monedero);
  reactor_add_handler (&tmonedero);

  event_handler_init (&tcafe, 2, (eh_func_t) tarea_cafe);
  reactor_add_handler (&tcafe);

  while (scanf("%d %d %d", &button, &moneda, &timer)==3) {
    clock_gettime(CLOCK_REALTIME, &start);
    //printf("Entramos while \n");
    reactor_handle_events ();
    //printf("Salimos while \n");
    clock_gettime(CLOCK_REALTIME, &stop);
    //timediff_saved[cnt] = stop.tv_nsec-start.tv_nsec;
    //cnt++;
  }
 
  /*
  //Imprimimos los tiempos almacenados
  for(i=0; i<10 ; i++){
     printf("%f \n", timediff_saved[i]);
  }
  */
  
  //printf("Salgo del main \n");
  return 0;
}