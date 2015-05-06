# ISEL2015-Reloj

Práctica5: Reloj
Autor: Gabriel García Ramos

Esta práctica trata de implementar un reloj que se ve en un array de 8 leds encapsulados en un péndulo que oscila a una frecuencia de 9 Hz. Para su implementación he diseñado una máquina de estados tal que cuando el sensor de infrarrojos detecta que el péndulo ha llegado al inicio empieza a pintar la hora actual.

Por falta de tiempo y por simplificación no he utilizado los GPIOs de la Raspberry si no que para visualizar la hora la he impreso por consola en formato de hora digital y a continuacion en un formato matriz de 1 y 0 mediante printf. Cada fila de la matriz de 1 y ceros simula lo que serían los 8 leds que están en el péndulo.

Imprimir con printf para simular los GPIOs implica que la función de pintar tarda más que la de calcular el tiempo actual y por lo tanto es una opción de planificación más crítica. Esto se puede comprobar descomentando las zonas del código que obtienen las diferencias de tiempos y las pinta.
