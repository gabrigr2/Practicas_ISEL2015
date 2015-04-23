# ISEL2015-Reactor

Práctica4: Reactor
Autor: Gabriel García Ramos


De las prácticas anteriores recordamos que tenemos una implementación de una máquina de cafe que funciona con máquinas de estados y una implementación con codigo cíclico definiendo un hiperperiodo de 0,8 segundos y un periodo secundario de 0,4 segundos que seguiremos utilizando para esta nueva implementación con Reactor. 

Además, en esta práctica controlaremos el café y el monedero como 2 eventos diferentes utilizando reactor. Cada evento tendrá una prioridad distinta para asegurarnos de que funciona correctamente. En este caso he implementado el evento cafe con mayor prioridad, ya que considero que es más lógico controlar el cafe en todo momento para que no se produzcan errores de desbordamiento de cafe o de leche, que estar pendiente de que introduzcan moneda que puede volver a meterse sin problemas.

Cada evento llamará a una funcion en la que ejecuta el fsm_fire de cada máquina de estados de forma que podemos medir los tiempos con la ayuda de la función clock_gettime() antes y después de cada llamada a los fsm_fire.

Cabe destacar los problemas encontrados en la implementación del código con la llamada a las funciones timeval y timespec ya que la primera función utiliza como argumentos segundos y microsegundos y la segunda utiliza como argumentos segundos y nanosegundos.

A partir de la máquina de cafe de la practica 1, he optado por elegir la máquina de estados de la cafetera con un periodo de 0,4 segundos y la máquina de estados del monedero con un periodo de 0,8 segundos. 

Con esto tenemos un hiperperiodo de 0,8 segundos (minimo comun multimplo de los periodos citados anteriormente) y un periodo secundario de 0,4 segundos (maximo comun divisor de los periodos citados anteriormente).

Sabiendo esto, implementamos una estructura switch en el metodo main() en la que para el caso1 hacemos un fire a ambas maquinas de estados y para el caso2 solo para la maquina de estados de la cafetera. 

Con la ayuda del metodo clock_gettime(), introduciendo una llamada al metodo antes y otra despues de los disparos a las maquinas de estados podemos medir el tiempo que pasamos ejecutando calculando la diferencia de las llamadas a clock _gettime(). Despues, mediante una sencilla resta al periodo secundario de esta diferencia de las llamadas resultante, obtenemos el tiempo que debemos esperar para que seguro se produzca la siguiente ejecucion correctamente.


