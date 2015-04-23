# ISEL2015-Codigo_ciclico

Práctica2: Codigo Ciclico
Autor: Gabriel García Ramos

A partir de la máquina de cafe de la practica 1, he optado por elegir la máquina de estados de la cafetera con un periodo de 0,4 segundos y la máquina de estados del monedero con un periodo de 0,8 segundos. 

Con esto tenemos un hiperperiodo de 0,8 segundos (minimo comun multimplo de los periodos citados anteriormente) y un periodo secundario de 0,4 segundos (maximo comun divisor de los periodos citados anteriormente).

Sabiendo esto, implementamos una estructura switch en el metodo main() en la que para el caso1 hacemos un fire a ambas maquinas de estados y para el caso2 solo para la maquina de estados de la cafetera. 

Con la ayuda del metodo clock_gettime(), introduciendo una llamada al metodo antes y otra despues de los disparos a las maquinas de estados podemos medir el tiempo que pasamos ejecutando calculando la diferencia de las llamadas a clock _gettime(). Despues, mediante una sencilla resta al periodo secundario de esta diferencia de las llamadas resultante, obtenemos el tiempo que debemos esperar para que seguro se produzca la siguiente ejecucion correctamente.


