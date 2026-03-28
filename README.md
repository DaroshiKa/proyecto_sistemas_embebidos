# Proyecto SE
Este repositorio es parte del proyecto del curso de Sistemas Embebidos (2026-1), cuyo objetivo es crear un sistema funcional que aplique conceptos de hardware y firmware.

## Introduccion
El desarrollo de interfaces entre el cuerpo humano y sistemas robóticos es un área de gran importancia dentro de la ingeniería, especialmente en aplicaciones relacionadas con prótesis, asistencia motora y control de dispositivos mediante señales biológicas. Uno de los enfoques para lograr esta interacción es la captura de señales eléctricas generadas por la actividad muscular, conocidas como señales electromiográficas (EMG).

Estas señales, aunque de baja intensidad, contienen suficiente información para identificar patrones de activación muscular que pueden ser capatados por un sistema electrónico para generar comandos de control. Sin embargo, muchos sistemas existentes requieren procesamiento complejo de señales o hardware especializado, lo que aumenta la dificultad de implementación y el costo del sistema.

Este proyecto piensa establecer un sistema embebido capaz de captar impulsos eléctricos musculares generados por el usuario y traducirlos en comandos de movimiento para una estructura robótica con forma de mano. El sistema deberá interpretar estos impulsos y determinar qué tipo de movimiento debe ejecutarse en distintas partes del mecanismo.

A diferencia de sistemas más complejos que permiten control proporcional continuo, el sistema propuesto utilizará movimientos discretos predefinidos. Esto significa que las señales detectadas no indicarán la magnitud exacta del movimiento, sino que activarán acciones específicas como mover el codo en una dirección determinada, abrir o cerrar la mano, o mover la muñeca en alguno de sus ejes permitidos.

Para lograr esto, el sistema deberá integrar sensores capaces de detectar los impulsos eléctricos musculares, un microcontrolador encargado del procesamiento de la señal y la toma de decisiones, y un conjunto de actuadores que permitan ejecutar los movimientos mecánicos correspondientes. Además, el sistema incluirá una interfaz de usuario mediante una pantalla que permitirá ajustar parámetros de funcionamiento como la velocidad de los movimientos y la sensibilidad con la que se detectan los impulsos eléctricos.

El resultado esperado es un prototipo funcional que demuestre la capacidad de un sistema embebido para interpretar señales biológicas simples y convertirlas en acciones mecánicas dentro de una estructura robótica.

## Objetivo General

Diseñar un sistema embebido capaz de interpretar señales eléctricas musculares generadas por el usuario.

## Objetivos específicos

- Diseñar el sistema de adquisición de señales electromiográficas que permita captar impulsos eléctricos musculares generados por el usuario.

- Ejecutar un sistema de procesamiento de señales que permita identificar distintos tipos de impulsos musculares y asociarlos con comandos de movimiento específicos.

- Desarrollar el sistema de control de actuadores encargado de ejecutar los movimientos mecánicos del codo, la muñeca y el mecanismo de apertura y cierre de la mano.

- Implementar una interfaz de usuario mediante una pantalla que permita ajustar parámetros del sistema, como la velocidad de los movimientos y la sensibilidad de detección de las señales musculares.

- Integrar el hardware y el firmware del sistema para garantizar la correcta comunicación entre sensores, unidad de procesamiento y actuadores.

- Realizar pruebas funcionales que permitan validar el correcto funcionamiento del sistema y la interpretación adecuada de los impulsos musculares.

- Controlar los movimientos discretos de una mano robótica, incorporando una arquitectura estructurada de hardware y firmware, así como una interfaz de configuración para el ajuste de parámetros de operación.

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

## Requerimientos Funcionales
1. Control de movimiento del codo mediante giroscopio

El sistema deberá detectar la orientación del antebrazo mediante un sensor 
giroscópico y generar movimientos del codo en los planos XY, XZ y YZ, de acuerdo con 
las variaciones angulares registradas. 

2. Control del metacarpo mediante señales EMG 

El sistema deberá procesar señales electromiográficas (EMG) del usuario para 
ejecutar movimientos discretos de flexión y extensión del metacarpo, asociados a 
patrones de activación muscular previamente definidos. 

3. Definición de posición inicial 

El sistema deberá permitir al usuario establecer una posición inicial de referencia del 
brazo robótico mediante el panel de control. 

4. Retorno a posición inicial 

El sistema deberá permitir que el brazo robótico regrese automáticamente a la 
posición inicial previamente definida, mediante una opción en el panel de control. 

5. Ajuste de velocidad de movimiento 

El sistema deberá permitir modificar la velocidad de ejecución de los movimientos 
del brazo robótico a través del panel de control. 

6. Ajuste de sensibilidad del giroscopio 

El sistema deberá permitir configurar la sensibilidad del sensor giroscópico, 
modificando la respuesta del sistema ante los movimientos del antebrazo del 
usuario, mediante el panel de control. 

7. Visualización de posición en el espacio 

El sistema deberá mostrar en el panel de control la posición del brazo robótico en un 
sistema de coordenadas tridimensional (3D), tomando como referencia la posición 
inicial definida (origen). 

8. Bloqueo del movimiento del codo 

El sistema deberá permitir bloquear y desbloquear el movimiento del codo mediante 
un control en el panel de usuario. 

9. Bloqueo del movimiento del metacarpo 

El sistema deberá permitir bloquear y desbloquear la flexión y extensión del 
metacarpo mediante un control en el panel de usuario. 

10. Visualización de velocidad 

El sistema deberá mostrar en el panel de control la velocidad actual de los 
movimientos del brazo robótico. 

11. Visualización de sensibilidad 

El sistema deberá mostrar en el panel de control los valores actuales de sensibilidad 
configurados para el giroscopio y las señales EMG.

## Requerimientos no funcionales
1. Tiempo de respuesta 

El sistema deberá responder a las señales EMG y del giroscopio en un tiempo máximo 
de 100 ms. 

2. Precisión de detección EMG 

El sistema deberá detectar señales EMG con una precisión mínima del 95% en la 
identificación de movimientos predefinidos. 

3. Seguridad del usuario 

El sistema deberá garantizar aislamiento eléctrico entre el usuario (EMG) y los 
circuitos de potencia para evitar riesgos. 

4. Tolerancia a fallos 

El sistema deberá detectar y notificar fallos de sensores en un tiempo menor a 1 
segundo.

## Prueba de requerimientos funcionales
Prueba requerimiento 1:  Control de movimiento del codo mediante giroscopio

Con un goniómetro, se medirá el ángulo que el usuario está haciendo con su brazo desde una posición cero y posteriormente se medirá el ángulo del brazo mecánico en ese mismo cero con el fin de comprobar el movimiento del codo. Se considerará satisfactorio un error igual o menor a 5 grados. 

Prueba requerimiento 2:  Control del metacarpo mediante señales EMG 

Para el requerimiento funcional 02, se utilizará un objeto redondo y se verá si la mano al cerrarse deja caer el objeto. Esto se repetirá 20 veces y se calculará el porcentaje de aciertos considerando como criterio de aceptación una precisión igual o mayor a 95%. 

Prueba requerimiento 3:  Definición de posición inicial

Para probar el requerimiento funcional 03, se deberá establecer una posición inicial y se deberá verificar que la posición sí quedó guardada. Posteriormente, se deberá verificar si al mover la mano la posición sigue siendo la misma.

Prueba requerimiento 4:  Retorno a posición inicial

Para la prueba del requerimiento funcional 4, se deberá establecer una posición inicial en el panel de control, tomar las medidas de la posición, y luego se desplazará el brazo mecánico. Después, se deberá apagar el dispositivo y volverlo a prender; cuando este se vuelva a prender, se deberá observar si el dispositivo vuelve a la posición inicial previamente establecida. 

Prueba requerimiento 5:  Ajuste de velocidad de movimiento

Para el 5, se deberá configurar una velocidad base y definir un movimiento recreable. Ya teniendo estos dos, se deberá medir con un cronómetro cuánto se demora el brazo mecánico en hacer el movimiento. Luego se deberá cambiar la velocidad a una velocidad más rápida que la velocidad base, y se deberá repetir el mismo movimiento tomando el tiempo que se demoró en hacerse. Si el tiempo que se demoró en hacer el movimiento es mayor, se considerará como que el requerimiento funciona. 

Prueba requerimiento 6: Ajuste de sensibilidad del giroscopio

Se utilizará un goniómetro para medir el ángulo del codo del brazo del usuario, mientras éste ejecuta distintos movimientos en diferentes ángulos; a la par, se medirá el ángulo correspondiente en el brazo robótico. Dicho proceso se realizará en al menos cinco posiciones diferentes, calculando el error absoluto entre el ángulo del brazo del usuario y el brazo robótico (satisfactorio si el error absoluto es menor o igual al 5%).

Prueba requerimiento 7: Visualización de posición en el espacio

Verificación de la posición del brazo robótico a través de la interfaz de usuario (pantalla). Se colocará el brazo en varias posiciones conocidas y estos datos se registrarán, y se compararán con la posición real del brazo; la posición real del brazo se medirá con referencias físicas o ángulos conocidos de las articulaciones. Este protocolo se repetirá al menos en 5 posiciones diferentes del brazo (satisfactorio si el error absoluto es menor o igual al 5%).

Prueba requerimiento 8: Bloqueo del movimiento del codo

Se activará el mecanismo de bloqueo del codo del brazo definido en la interfaz del usuario. Una vez esté activado, el usuario ejecutará movimientos del brazo que produzcan cambios en el codo. Se analizará si el codo del brazo robótico permanece fijo sin reaccionar a las entradas del giroscopio. A continuación, se desactivará el bloqueo y se comprobará que el movimiento del codo se restablezca con normalidad. La prueba será satisfactoria si el codo se mantiene inmóvil en el bloqueo y si responde normalmente al desactivarlo.

Prueba requerimiento 9: Bloqueo del movimiento del metacarpo

Se activará el mecanismo de bloqueo del metacarpo definido en la interfaz del usuario. Una vez activado, el usuario realizará movimientos de la mano que produzcan cambios en el metacarpo. Se observará si el metacarpo del brazo robótico permanece fijo sin reaccionar a las entradas del giroscopio. Posteriormente, se desactivará el bloqueo y se comprobará que el movimiento del metacarpo se restablezca normalmente. La prueba será satisfactoria si el metacarpo se mantiene inmóvil en el bloqueo y si responde normalmente al desactivarlo.

Prueba requerimiento 10: Visualización de velocidad. 

Se comprobará que el mecanismo muestre la velocidad de movimiento del brazo robótico en el interfaz del usuario. Se realizarán movimientos del brazo a distintas velocidades (rápida, mediana y lenta) y se compararán  estos valores con una estimación de la velocidad del brazo real, calculada con cambios de posición en un intervalo de tiempo ya conocido. Se reiterará este proceso en al menos 5 pruebas diferentes. La prueba se considerará satisfactoria si la velocidad del brazo robótico es coherente con lo esperado.

Prueba requerimiento 11:  Visualización de sensibilidad

En el RF-11, se modificarán los valores de sensibilidad en el panel de control y se verificará que estos cambios se reflejen correctamente en la interfaz. Se validará la coherencia entre los valores configurados y visualizados.

## Pruebas de requerimientos no funcionales.

Prueba requerimiento 1.  Tiempo de respuesta.

Se medirá el intervalo entre la ejecución del movimiento del usuario (identificado por el giroscopio) y la ejecución del movimiento correspondiente en el brazo robótico; se usará un cronómetro, y se efectuarán pruebas en movimientos lentos, medios y rápidos. Se calculará la media del tiempo de respuesta del sistema; será satisfactorio si este tiempo está dentro de los 100 ms.

Prueba requerimiento 2. Precisión de detección EMG.

Se compararán las contracciones musculares efectuadas por el usuario con la respuesta generada por el brazo robótico; el usuario realizará diferentes tipos de activaciones musculares (contracción, flexión, extensión, etc), mientras sus señales EMG y la acción del brazo robótico se registran. Se evaluará si el sistema detecta bien cada movimiento muscular y si el movimiento es el esperado. Se juzgará si la prueba es satisfactoria si el sistema identifica correctamente las señales EMG en su mayoría.

Prueba requerimiento 3:  Seguridad del usuario

Para el requerimiento no funcional 3, se realizarán mediciones con un multímetro para verificar el aislamiento eléctrico entre el usuario y los circuitos de potencia. Se comprobará que no exista paso de corriente. 

Prueba requerimiento 4: Tolerancia a fallos

Para la prueba se desconectarán intencionalmente los sensores del sistema y con un cronómetro se medirá el tiempo que se tarda en detectar el fallo y generar una alerta. Se considerará adecuado un tiempo de detección inferior o igual a 1 segundo. 

## Plantilla para los test case (la misma recomendada en clase)



