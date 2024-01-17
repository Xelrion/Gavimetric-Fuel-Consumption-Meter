/***********************************************************************************************************
 * MEDIDOR GAVIMÉTRICO DE CONSUMO DE COMBUSTIBLE
 * Mide el consumo del depósito y envía las medidas a un sistema remoto o a un sistema de control.
 * Opera sobre el estado del depósito para operaciones de llenado, vaciado y detención.
 * Permite configurar los parámetros de funcionamiento del sistema desde el sistema de control.
 ***********************************************************************************************************/

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
#include "calculaConsumo.h"

// Constantes para la planificación de cada tarea
// Nombre, periodo, tamaño de stack y prioridad

static const char* TSKMEDIDASNIV_TAG = "MedidasNivel";
#define TSKMEDIDASNIV_PERIODO_MS 500
#define TSKMEDIDASNIV_STACK_WD  2048
#define TSKMEDIDASNIV_PRIORIDAD    6

static const char* TSKCONSUMO_TAG = "CalculaConsumo";
#define TSKCONSUMO_PERIODO_MS 500
#define TSKCONSUMO_STACK_WD  2048
#define TSKCONSUMO_PRIORIDAD    6

static const char* TSKCONTROLDEP_TAG = "ControlDeposito";
#define TSKCONTROLDEP_PERIODO_MS 500
#define TSKCONTROLDEP_STACK_WD  2048
#define TSKCONTROLDEP_PRIORIDAD    7

// Información de intercambio de cada tarea
tareaMedidasNivelInfo_t tskMedidasNivel_data;
tareaControlDepositoInfo_t   tskControlDeposito_data;
tareaConsumoInfo_t   tskConsumo_data;

// Información necesaria para planificar cada tarea
taskConfig_t tskMedidasNivel_config, tskControlDeposito_config, tskConsumo_config;
taskInfo_t tskMedidasNivel_info, tskControlDeposito_info, tskConsumo_info;

// Referencias a tareas
TaskHandle_t tskMedidasNivel, tskControlDeposito, tskConsumo;

// Referencias a estructuras protegidas
bufferCircular_t medidas;
const char* tagMedidas = "Buffer_Medidas";
bufferCircular_t consumo;
const char* tagConsumo = "Buffer_Consumo";

// extern "C" void app_main(void)
void app_main(void)
{
    // Preparamos los buffers de intercambio de información
    bufferCircularCrea(&medidas, tagMedidas);
    bufferCircularCrea(&consumo, tagConsumo);

    // Prepara la estructura con la información que intercambia cada taera
    tareaMedidasNivelSet(&tskMedidasNivel_data, &medidas);
    tareaControlDepositoSet(&tskMedidasNivel_data, &medidas);
    tareaConsumoSet(&tskConsumo_data, &medidas, &consumo);

    // Completamos las estructuras de paso de información
    // Configuración de cada tarea
    taskConfigSet(&tskMedidasNivel_config, TSKMEDIDASNIV_PERIODO_MS, TSKMEDIDASNIV_TAG);
    taskConfigSet(&tskControlDeposito_config,   TSKCONTROLDEP_PERIODO_MS,   TSKCONTROLDEP_TAG);
    taskConfigSet(&tskConsumo_config,   TSKCONSUMO_PERIODO_MS,   TSKCONSUMO_TAG);

    // Información completa de cada tarea
    taskInfoSet(&tskMedidasNivel_info, &tskMedidasNivel_config, (void*)&tskMedidasNivel_data);
    taskInfoSet(&tskControlDeposito_info,   &tskControlDeposito_config,   (void*)&tskControlDeposito_data);
    taskInfoSet(&tskConsumo_info,   &tskConsumo_config,   (void*)&tskConsumo_data);

    // Planifica cada tarea
    xTaskCreate(tareaMedidasNivel, TSKMEDIDASNIV_TAG, TSKMEDIDASNIV_STACK_WD, &tskMedidasNivel_info, TSKMEDIDASNIV_PRIORIDAD,  &tskMedidasNivel); tskMedidasNivel_config.activa = 1; // numTsk++;
    xTaskCreate(tareaControlDeposito,   TSKCONTROLDEP_TAG,   TSKCONTROLDEP_STACK_WD,   &tskMedidasNivel_info,   TSKMEDIDASNIV_PRIORIDAD,    &tskMedidasNivel);   tskControlDeposito_config.activa = 1;   // numTsk++;
    xTaskCreate(tareaConsumo,   TSKCONSUMO_TAG,   TSKCONSUMO_STACK_WD,   &tskConsumo_info,   TSKCONSUMO_PRIORIDAD,    &tskConsumo);   tskConsumo_config.activa = 1;   // numTsk++;

    vTaskDelay(10);

}
