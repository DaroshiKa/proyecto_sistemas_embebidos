# Proyecto SE
Este repositorio es parte del proyecto del curso de Sistemas Embebidos (2026-1), cuyo objetivo es crear un sistema funcional que aplique conceptos de hardware y firmware.

## Introdccion
El desarrollo de interfaces entre el cuerpo humano y sistemas robóticos es un área de gran importancia dentro de la ingeniería, especialmente en aplicaciones relacionadas con prótesis, asistencia motora y control de dispositivos mediante señales biológicas. Uno de los enfoques para lograr esta interacción es la captura de señales eléctricas generadas por la actividad muscular, conocidas como señales electromiográficas (EMG).

Estas señales, aunque de baja intensidad, contienen suficiente información para identificar patrones de activación muscular que pueden ser capatados por un sistema electrónico para generar comandos de control. Sin embargo, muchos sistemas existentes requieren procesamiento complejo de señales o hardware especializado, lo que aumenta la dificultad de implementación y el costo del sistema.

Este proyecto piensa establecer un sistema embebido capaz de captar impulsos eléctricos musculares generados por el usuario y traducirlos en comandos de movimiento para una estructura robótica con forma de mano. El sistema deberá interpretar estos impulsos y determinar qué tipo de movimiento debe ejecutarse en distintas partes del mecanismo.

A diferencia de sistemas más complejos que permiten control proporcional continuo, el sistema propuesto utilizará movimientos discretos predefinidos. Esto significa que las señales detectadas no indicarán la magnitud exacta del movimiento, sino que activarán acciones específicas como mover el codo en una dirección determinada, abrir o cerrar la mano, o mover la muñeca en alguno de sus ejes permitidos.

Para lograr esto, el sistema deberá integrar sensores capaces de detectar los impulsos eléctricos musculares, un microcontrolador encargado del procesamiento de la señal y la toma de decisiones, y un conjunto de actuadores que permitan ejecutar los movimientos mecánicos correspondientes. Además, el sistema incluirá una interfaz de usuario mediante una pantalla que permitirá ajustar parámetros de funcionamiento como la velocidad de los movimientos y la sensibilidad con la que se detectan los impulsos eléctricos.

El resultado esperado es un prototipo funcional que demuestre la capacidad de un sistema embebido para interpretar señales biológicas simples y convertirlas en acciones mecánicas dentro de una estructura robótica.

## Objetivo General

Diseñar e implementar un sistema embebido capaz de interpretar señales eléctricas musculares generadas por el usuario y utilizarlas para controlar los movimientos discretos de una mano robótica, incorporando una arquitectura estructurada de hardware y firmware, así como una interfaz de configuración para el ajuste de parámetros de operación.

## Objetivos específicos

-Diseñar el sistema de adquisición de señales electromiográficas que permita captar impulsos eléctricos musculares generados por el usuario.

-Implementar un sistema de procesamiento de señales que permita identificar distintos tipos de impulsos musculares y asociarlos con comandos de movimiento específicos.

-Desarrollar el sistema de control de actuadores encargado de ejecutar los movimientos mecánicos del codo, la muñeca y el mecanismo de apertura y cierre de la mano.

-Implementar una interfaz de usuario mediante una pantalla que permita ajustar parámetros del sistema como la velocidad de los movimientos y la sensibilidad de detección de las señales musculares.

-Integrar el hardware y el firmware del sistema para garantizar la correcta comunicación entre sensores, unidad de procesamiento y actuadores.

-Realizar pruebas funcionales que permitan validar el correcto funcionamiento del sistema y la interpretación adecuada de los impulsos musculares.

## Alcance del proyecto

El proyecto comprende el diseño, desarrollo e integración de un sistema embebido capaz de controlar una mano robótica mediante la detección de impulsos eléctricos musculares generados por el usuario.

El sistema estará compuesto por un módulo de adquisición de señales musculares, un sistema de procesamiento basado en un microcontrolador, un conjunto de actuadores encargados de realizar los movimientos mecánicos y una interfaz de configuración para el usuario.

Los movimientos de la mano robótica serán discretos y predefinidos, por lo que el sistema no realizará control proporcional de posición o fuerza. En su lugar, cada tipo de impulso eléctrico detectado activará un movimiento específico dentro del sistema.

Entre los movimientos contemplados en el proyecto se encuentran:

-Movimiento del codo en el eje XY.

-Movimiento del codo en el eje YZ.

-Movimiento del codo en una posición intermedia entre ambos ejes.

-Apertura y cierre de la mano representado en dos posiciones posibles.

-Movimiento de la muñeca en el eje XY.

Adicionalmente, el sistema contará con una interfaz de usuario mediante una pantalla que permitirá configurar ciertos parámetros de funcionamiento del sistema, tales como la velocidad de ejecución de los movimientos y la sensibilidad con la que se detectan las señales musculares.

El alcance del proyecto incluye la implementación del hardware necesario para la captura de señales musculares, el desarrollo del firmware encargado del procesamiento de las señales y del control de los actuadores, la integración del sistema completo y la validación del funcionamiento mediante pruebas experimentales.

## Roles 

### Technical Lead (Líder por desgracia)
Jaime Landinez Ortiz 

### Firmware Engineer 
Juan David Botero Cubides

### Hardware Integration Engineer
Susana Del Toro Castaño

### Verification & Testing Engineer
Clara Isabel Botero Peréz
