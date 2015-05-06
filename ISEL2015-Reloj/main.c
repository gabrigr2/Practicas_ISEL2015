#include <stdio.h>
#include <time.h>
#include "fsm.h"


//Variables para guardar la hora
int horas;
int minutos;
int segundos;
int hora[8];

//Variables para medida de tiempos
static struct timespec start,stop;

//Señales entrada de los sensores
int sensor;
int llegada;

int i;

/*
//Variables para mostrar los resultados
int tmp[24];
int tmp2[24];
int cnt =0;
int cnt2 =0;
*/

enum estados {
  START,
  PAINT,
};

void actualizarHora(){

        time_t tInicial = time(NULL); //epoch 00:00 del 1 de enero de 1970 cuenta los segundos desde entonces.

        struct tm tiempo = *localtime(&tInicial); // Separa minutos horas etc..

        horas = tiempo.tm_hour;
        minutos = tiempo.tm_min;
        segundos = tiempo.tm_sec;
        

        hora[0] = '0' + horas/10;
        hora[1] = '0' + horas%10;
        hora[2] = ':';
        hora[3] = '0' + minutos/10;
        hora[4] = '0' + minutos%10;
        hora[5] = ':';
        hora[6] = '0' + segundos/10;
        hora[7] = '0' + segundos%10;

}

static void numeros(char c){
    switch(c){
        case '0':
            printf("1 1 1 1 1 1 1 1\n");
            printf("1 0 0 0 0 0 0 1\n");
            printf("1 1 1 1 1 1 1 1\n");
            printf("0 0 0 0 0 0 0 0\n");
        break;
        
        case '1':
            printf("0 0 0 0 0 1 0 0\n");
            printf("0 0 0 0 0 0 1 0\n");
            printf("1 1 1 1 1 1 1 1\n");
            printf("0 0 0 0 0 0 0 0\n");
        break;
        
        case '2':
            printf("1 1 1 1 1 0 0 1\n");
            printf("1 0 0 0 1 0 0 1\n");
            printf("1 0 0 0 1 1 1 1\n");
            printf("0 0 0 0 0 0 0 0\n");
        break;
        
        case '3':
            printf("1 0 0 0 1 0 0 1\n");
            printf("1 0 0 0 1 0 0 1\n");
            printf("1 1 1 1 1 1 1 1\n");
            printf("0 0 0 0 0 0 0 0\n");
        break;
        
        case '4':
            printf("0 0 0 0 1 1 1 1\n");
            printf("0 0 0 0 1 0 0 0\n");
            printf("1 1 1 1 1 1 1 1\n");
            printf("0 0 0 0 0 0 0 0\n");
        break;
        
        case '5':
            printf("1 0 0 0 1 1 1 1\n");
            printf("1 0 0 0 1 0 0 1\n");
            printf("1 1 1 1 1 0 0 1\n");
            printf("0 0 0 0 0 0 0 0\n");
        break;
            
        case '6':
            printf("1 1 1 1 1 1 1 1\n");
            printf("1 0 0 0 1 0 0 1\n");
            printf("1 1 1 1 1 0 0 1\n");
            printf("0 0 0 0 0 0 0 0\n");
        break;
        
        case '7':
            printf("0 0 0 0 0 0 0 1\n");
            printf("0 0 0 0 0 0 0 1\n");
            printf("1 1 1 1 1 1 1 1\n");
            printf("0 0 0 0 0 0 0 0\n");
        break;
        
        case '8':
            printf("1 1 1 1 1 1 1 1\n");
            printf("1 0 0 0 1 0 0 1\n");
            printf("1 1 1 1 1 1 1 1\n");
            printf("0 0 0 0 0 0 0 0\n");
        break;
        
        case '9':
            printf("1 0 0 0 1 1 1 1\n");
            printf("1 0 0 0 1 0 0 1\n");
            printf("1 1 1 1 1 1 1 1\n");
            printf("0 0 0 0 0 0 0 0\n");
        break;
        
        default:
            clock_gettime(CLOCK_REALTIME, &start);
            printf("0 0 1 0 0 1 0 0\n");
            clock_gettime(CLOCK_REALTIME, &stop);
            printf("0 0 0 0 0 0 0 0\n");
            //tmp[cnt]=( stop.tv_nsec - start.tv_nsec );
            //cnt++;
        break;
    }
 
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
        struct timespec tEspera = {0, 65000000};
        nanosleep(&tEspera, NULL);

}

static void pintar ()
{
        char c;

        actualizarHora();

        //Pintamos la hora de forma visible
        for(i=0; i<8; i++){
                espera();
                printf("%c", hora[i]);
                fflush(stdout);
        }

        //Dejamos 1 espacio de separacion para que se vea mejor cada representacion
        printf("\n");

        //Pintamos la hora con 0 y 1 simulando LEDs de los GPIOs
        for(i=0; i<8; i++){
                espera();
                c = hora[i];                
                numeros(c);
        }

        llegada = 1;

}

static int sensorLlegada ()
{
        if (llegada == 1){
                llegada = 0;
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
  { START, sensorSalida, PAINT, pintar },
  { PAINT,  sensorLlegada, START, fin },
  {-1, NULL, -1, NULL },
};

int main (){
        fsm_t* reloj_fsm = fsm_new(reloj);

        while(scanf("%d", &sensor)==1){
            clock_gettime(CLOCK_REALTIME, &start);
            fsm_fire(reloj_fsm); 
            clock_gettime(CLOCK_REALTIME, &stop);
            //tmp2[cnt2]=( stop.tv_nsec - start.tv_nsec );
            //cnt2++;
        }   

        /*for(i=0;i<24; i++){
                printf("%d ",tmp[i]);
                printf("%d ",tmp2[i]);
                printf("\n");
        }*/

return 0;
}