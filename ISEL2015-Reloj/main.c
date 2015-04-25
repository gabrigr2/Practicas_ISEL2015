#include <stdio.h>
#include <time.h>
#include "fsm.h"

int segundos;
int minutos;
int horas;


int sensor;
int flagLlegada;
int arrayFecha[8];

enum reloj {
  INICIAR,
  PINTAR,
  FINALIZAR,
};

void actualizarFecha(){

        time_t fechaOrigen = time(NULL); //epoch 00:00 del 1 de enero de 1970 cuenta los segundos desde entonces.

        struct tm fecha = *localtime(&fechaOrigen); // Seprara minutos horas etc..

        segundos = fecha.tm_sec;
        minutos = fecha.tm_min;
        horas = fecha.tm_hour;

        arrayFecha[0] = '0' + horas/10;
        arrayFecha[1] = '0' + horas%10;
        arrayFecha[2] = ':';
        arrayFecha[3] = '0' + minutos/10;
        arrayFecha[4] = '0' + minutos%10;
        arrayFecha[5] = ':';
        arrayFecha[6] = '0' + segundos/10;
        arrayFecha[7] = '0' + segundos%10;

}

static int sensorSalida ()
{
        if (sensor == 1){
                return 1;
        } else{
                return 0;
        }
}
static void espera ()
{
        struct timespec tiempoEspera = {0, 500000000};
        nanosleep(&tiempoEspera, NULL);

}

static void pinta ()
{
        int i;
        char n;
        //struct timespec tiempoEspera = {0, 500000000};
        actualizarFecha();

        for(i=0; i<8; i++){
                espera();
                n = arrayFecha[i];
                printf("%c", n);
                fflush(stdout);
        }

        flagLlegada = 1;

}

static int sensorLlegada ()
{
        if (flagLlegada == 1){
                flagLlegada = 0;
                return 1;
        } else{
                return 0;
        }

}

static void fin ()
{
        printf("\n");
}

static fsm_trans_t reloj[] = {
  { INICIAR, sensorSalida, PINTAR,     pinta    },
  { PINTAR,  sensorLlegada, INICIAR,    fin   },
  {-1, NULL, -1, NULL },
};

int main (){
        fsm_t* reloj_fsm = fsm_new(reloj);

        while(scanf("%d", &sensor)==1){
                fsm_fire(reloj_fsm);
        }
return 0;
}