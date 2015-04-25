# ISEL2015-Cafetera

Práctica1: CAFETERA
Autor: Gabriel García Ramos

En el enunciado de la práctica se plantea el problema de crear una cafetera con dos máquinas de estados, una que controle el proceso del café y otra diferente que controle el dinero. La primera ya viene implementada y definida con 4 estados: Waiting, Cup, Coffee y Milk. Para el diseño de la segunda máquina de estados he considerado un único estado Monedero que llama constantemente a una función calcula_valor() hasta que devuelve 1 cuando se cumple que se ha introducido el dinero suficiente y se ha pulsado el botón, entonces volvemos al mismo estado pero permitimos devolver el dinero sobrante en caso de que se haya introducido más dinero del necesario. El dinero total introducido se almacena en una variable global compartida por ambas máquinas de estados y para el diseño se ha considerado que la vuelta se devuelve en cuanto empieza el proceso de servir el café, justo después de poner el vaso.

Una vez compilado y ejecutado el programa hay que introducir por la consola tres argumentos para que funcione. El primer argumento es el estado del botón (1 pulsado, 0 en reposo), el segundo el valor la moneda introducida (puede introducirse cualquier valor númerico), y el tercero el valor del timer que siempre será 1. 

No he querido implementar un menú con la función de interfaz para el usuario mediante la utilización de llamadas al método printf("") ya que para ejecutar el programa siempre introduzco los parámetros de entrada mediante un fichero inputs que tiene en 3 columnas para los distintos argumentos y obtengo la salida en un fichero outputs mediante el comando ./main <inputs >outputs