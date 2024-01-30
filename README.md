| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- |

# Medidor gavimétrico de consumo

Implementación de un sistema embebido en tiempo real para medir el consumo de un depósito de combustible.

El depósito posee una báscula para medir su peso, y un sensor de desbordamiento para detectar exceso de combustible.
Los actuadores son una bomba eléctrica de llenado, una electroválvula de llenado, y una electroválvula de vaciado.
El sistema se puede operar en modo manual desde una consola de operador.
En modo automático, se envían medidas de consumo a un sistema remoto siempre que la señal de petición de medidas esté activa.
Al activar la parada de emergencia, por desbordamiento o por pulsador de emergencia, el sistema se detiene en un estado seguro.

El sistema se compone de 6 tareas periódicas, y una interrupción activada por dos señales posibles.

## Tareas e interrupciones

### Tarea medidasNivel
Toma medidas de pesaje desde la báscula. Comprueba si superan el nivel máximo o mínimo del depósito.
Cada vez que se cumpla el periodo de toma de medidas, esta se envía al buffer Medidas siempre que la espera de estabilización no esté activa.

### Tarea calculaConsumo
Toma medidas del buffer Medidas, y calcula el consumo medio entre cada par de medidas disponibles.
Dependiendo de si el sistema se encuentra en modo manual o automático, se envía al buffer consumoConsola o consumoRemoto.

### Tarea comandosConsola
Recibe el último comando enviado por la consola, y su valor asociado si lo tuviera (para modificar parámetros de configuración).
En función del comando recibido, ejecuta la acción correspondiente sobre el sistema.

### Tarea comunicacionConsola
Comprueba el estado actual del sistema y las medidas de consumo almacenadas en el buffer consumoConsola.
Actualiza la información mostrada en el display de la consola.
Si se activa la parada de emergencia, muestra una notificación de emergencia en la consola.

### Tarea comunicacionRemoto
Si la señal digital de petición de medidas está activa, envía las medidas almacenadas en el buffer consumoRemoto.
Las medidas se envían en forma de señal analógica de entre 0 y 10 voltios, donde los 10 corresponden al consumo máximo.

### Tarea controlDeposito
Máquina de estados que controla el estado actual de funcionamiento del depósito: normal, llenado o vaciado.
Cada ejecución, comprueba el estado del sistema y actúa en consecuencia, activando o desactivando los actuadores.
Controla la bomba eléctrica de llenado y las electroválvulas.
Al detectar un nivel mínimo, inicia el proceso de llenado; al detectar el máximo, lo detiene.
Al concluir un proceso de llenado, se activa una espera de estabilización en la cual no se pueden tomar medidas de consumo.

### Interrupciones de emergencia
Al detectar un flanco de subida en el sensor de desbordamiento o el pulsador de emergencia, activa la interrupción.
La interrupción accede al estado compartido de parada de emergencia y lo activa.

## Recursos compartidos
Se utilizan varios recursos compartidos en el sistema, de diferentes tipos:

### Buffer circular
Existen tres buffers: medidas, consumoConsola y consumoRemoto. Se utilizan para almacenar las medidas de pesaje y el consumo medio calculado,
y posteriormente enviarlas al display o al sistema remoto.

### Estado del sistema
Almacena variables relacionadas con el estado actual del sistema: nivel del depósito, estado de funcionamiento del depósito,
último comando de medidas de consumo recibido, espera de estabilización.

### Configuración del sistema
Almacena los parámetros del sistema configurables por el operador: nivel máximo y mínimo del depósito, tiempo de periodo de toma de medidas,
nivel máximo de consumo (correspondiente a la señal analógica de 10v para el sistema remoto), tiempo de espera de estabilización.

### Parada de emergencia
Almacena el estado de la parada de emergencia del sistema: activo o inactivo.

## Implementación de hardware

Las secciones de código que contienen esta indicación incluyen funciones de interacción con el hardware del sistema.
A la hora de implementar el sistema real, deben sustituirse con todo el código y configuración necesarios para comunicarse con el hardware utilizado.
Las funciones describen de qué manera deben actuar y qué valores deben devolver para funcionar correctamente.
