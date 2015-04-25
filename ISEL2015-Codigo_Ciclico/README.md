# ISEL2015-Codigo_ciclico

Práctica2: Codigo Ciclico
Autor: Gabriel García Ramos

A partir de la máquina de cafe de la practica 1, he optado por elegir la máquina de estados de la cafetera con un periodo de 0,4 segundos y la máquina de estados del monedero con un periodo de 0,8 segundos. Me parece más lógico dar prioridad al servicio del café frente al de las monedas ya que en caso de que no se introduzca correctamente el usuario puede volver a introducirla sin ningún esfuerzo.

Con esto tenemos un hiperperiodo de 0,8 segundos (minimo comun multimplo de los periodos citados anteriormente) y un periodo secundario de 0,4 segundos (maximo comun divisor de los periodos citados anteriormente).

Sabiendo esto, implementamos una estructura switch en el metodo main() en la que para el caso1 hacemos un fsm_fire a ambas maquinas de estados y para el caso2 solo hacemos fsm_fire para la maquina de estados de la cafetera. 

Con la ayuda del metodo clock_gettime(), introduciendo una llamada al metodo antes y otra despues de los disparos a las maquinas de estados podemos medir el tiempo que pasamos ejecutando calculando la diferencia de las llamadas a clock _gettime(). Despues, mediante una sencilla resta al periodo secundario de esta diferencia de las llamadas resultante, obtenemos el tiempo que debemos esperar para que seguro se produzca la siguiente ejecucion correctamente.

No he querido implementar un menú con la función de interfaz para el usuario mediante la utilización de llamadas al método printf("") ya que para ejecutar el programa siempre introduzco los parámetros de entrada mediante un fichero inputs que tiene en 3 columnas para los distintos argumentos y obtengo la salida en un fichero outputs mediante el comando ./main <inputs >outputs. Además, en esta práctica que medimos tiempos, estos se verían afectados por el retraso que produce la llamada al método printf(). 

Los resultados se mostrarán en dos columnas donde la primera hace referencia al caso1 del switch y la segunda al caso2.
