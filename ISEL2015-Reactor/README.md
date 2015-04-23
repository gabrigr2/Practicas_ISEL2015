# ISEL2015-Reactor

Práctica4: Reactor
Autor: Gabriel García Ramos


De las prácticas anteriores recordamos que tenemos una implementación de una máquina de cafe que funciona con máquinas de estados y una implementación con codigo cíclico definiendo un hiperperiodo de 0,8 segundos y un periodo secundario de 0,4 segundos que seguiremos utilizando para esta nueva implementación con Reactor. 

Además, en esta práctica controlaremos el café y el monedero como 2 eventos diferentes utilizando reactor. Cada evento tendrá una prioridad distinta para asegurarnos de que funciona correctamente. En este caso he implementado el evento cafe con mayor prioridad, ya que considero que es más lógico controlar el cafe en todo momento para que no se produzcan errores de desbordamiento de cafe o de leche, que estar pendiente de que introduzcan moneda que puede volver a meterse sin problemas.

Cada evento llamará a una funcion en la que ejecuta el fsm_fire de cada máquina de estados de forma que podemos medir los tiempos con la ayuda de la función clock_gettime() antes y después de cada llamada a los fsm_fire.

Cabe destacar los problemas encontrados en la implementación del código con la llamada a las funciones timeval y timespec ya que la primera función utiliza como argumentos segundos y microsegundos y la segunda utiliza como argumentos segundos y nanosegundos.

