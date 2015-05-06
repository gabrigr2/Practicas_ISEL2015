# ISEL2015-Xenomai

Práctica3: Xenomai
Autor: Gabriel García Ramos

Para esta práctica optamos por implementar en programa basándonos en pthreads. Esto es, que a cada máquina de estados le asignamos un thread o hebra con una prioridad. Para seguir siendo coherentes con las prácticas anteriores hacemos que la prioridad de la hebra de la máquina de cafe sea mayor que la hebra de la máquina monedero. Además también seguiremos utilizando los tiempos que usábamos en el ejecutivo cíclico: 0,4 s para el café y 0,8 s para el monedero. 

Estas hebrás accederán a variables de nustro programa para modificarlas y en algunos casos accederán a variables globales compartidas entre ambas máquinas. Haciendo uso de muttex, cada vez que accedamos a una variable compartida la bloquearemos para que así no pueda acceder la otra hebra y se produzcan escrituras a destiempo que provoquen solapamientos y malentendidos.


