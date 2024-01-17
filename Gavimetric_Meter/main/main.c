/* Aplicación principal de demostración del uso de buffers para compartir información */

// #include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"

// Recursos y estructuras comunes
#include "myTaskConfig.h"
#include "bufferCircular.h"
#include "fsm.h"

// Módulo de control del depósito
#include "controlDeposito.h"

// Módulo de monitorización del depósito
#include "medidasNivel.h"

// Constantes para la planificación de cada tarea
// Nombre, periodo, tamaño de stack y prioridad

static const char* TSKMEDIDASNIV_TAG = "MedidasNivel";
#define TSKMEDIDASNIV_PERIODO_MS 500
#define TSKMEDIDASNIV_STACK_WD  2048
#define TSKMEDIDASNIV_PRIORIDAD    6

static const char* TSKCONTROLDEP_TAG = "ControlDeposito";
#define TSKCONTROLDEP_PERIODO_MS 500
#define TSKCONTROLDEP_STACK_WD  2048
#define TSKCONTROLDEP_PRIORIDAD    7

// Información de intercambio de cada tarea
tareaMedidasNivelInfo_t tskMedidasNivel_data;
tareaControlDepositoInfo_t   tskControlDeposito_data;

// Información necesaria para planificar cada tarea
taskConfig_t tskMedidasNivel_config, tskControlDeposito_config;
taskInfo_t tskMedidasNivel_info, tskControlDeposito_info;

TaskHandle_t tskMedidasNivel, tskControlDeposito;

bufferCircular_t medidas;
const char* tagMedidas = "Buffer_Medidas";

// extern "C" void app_main(void)
void app_main(void)
{
    // Preparamos los buffers de intercambio de información
    bufferCircularCrea(&medidas, tagMedidas);

    // Prepara la estructura con la información que intercambia cada taera
    tareaLecturaSet(&tskMedidasNivel_data, &medidas);

    // Completamos las estructuras de paso de información
    // Configuración de cada tarea
    taskConfigSet(&tskMedidasNivel_config, TSKMEDIDASNIV_PERIODO_MS, TSKMEDIDASNIV_TAG);
    taskConfigSet(&tskControlDeposito_config,   TSKCONTROLDEP_PERIODO_MS,   TSKCONTROLDEP_TAG);

    // Información completa de cada tarea
    taskInfoSet(&tskMedidasNivel_info, &tskMedidasNivel_config, (void*)&tskMedidasNivel_data);
    taskInfoSet(&tskControlDeposito_info,   &tskControlDeposito_config,   (void*)&tskControlDeposito_data);

    // Planifica cada tarea
    xTaskCreate(tareaMedidasNivel, TSKMEDIDASNIV_TAG, TSKMEDIDASNIV_STACK_WD, &tskMedidasNivel_info, TSKMEDIDASNIV_PRIORIDAD,  &tskMedidasNivel); tskMedidasNivel_config.activa = 1; // numTsk++;
    xTaskCreate(tareaControlDeposito,   TSKCONTROLDEP_TAG,   TSKCONTROLDEP_STACK_WD,   &tskMedidasNivel_info,   TSKMEDIDASNIV_PRIORIDAD,    &tskMedidasNivel);   tskControlDeposito_config.activa = 1;   // numTsk++;

    vTaskDelay(10);

}
