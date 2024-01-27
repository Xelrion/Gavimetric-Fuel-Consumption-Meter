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

// Estados compartidos
#include "estadoSistema.h"
#include "configSistema.h"
#include "paradaEmergencia.h"

// Módulo de control del depósito
#include "controlDeposito.h"

// Módulo de monitorización del depósito
#include "medidasNivel.h"
#include "calculaConsumo.h"

// Módulo de comunicación con la consola
#include "comandosConsola.h"
#include "comunicacionDisplay.h"

// Módulo de comunicación con el sistema remoto
#include "comunicacionRemoto.h"

// Constantes para la planificación de cada tarea
// Nombre, periodo, tamaño de stack y prioridad

static const char* TSKCONTROLDEP_TAG = "ControlDeposito";
#define TSKCONTROLDEP_PERIODO_MS 500
#define TSKCONTROLDEP_STACK_WD  2048
#define TSKCONTROLDEP_PRIORIDAD    7

static const char* TSKMEDIDASNIV_TAG = "MedidasNivel";
#define TSKMEDIDASNIV_PERIODO_MS 500
#define TSKMEDIDASNIV_STACK_WD  2048
#define TSKMEDIDASNIV_PRIORIDAD    6

static const char* TSKCONSUMO_TAG = "CalculaConsumo";
#define TSKCONSUMO_PERIODO_MS 500
#define TSKCONSUMO_STACK_WD  2048
#define TSKCONSUMO_PRIORIDAD    5

static const char* TSKCOMANDOSCONS_TAG = "ComandosConsola";
#define TSKCOMANDOSCONS_PERIODO_MS 500
#define TSKCOMANDOSCONS_STACK_WD  2048
#define TSKCOMANDOSCONS_PRIORIDAD    4

static const char* TSKCOMUNDISPLAY_TAG = "ComunicacionDisplay";
#define TSKCOMUNDISPLAY_PERIODO_MS 500
#define TSKCOMUNDISPLAY_STACK_WD  2048
#define TSKCOMUNDISPLAY_PRIORIDAD    3

static const char* TSKCOMUNREMOTO_TAG = "ComunicacionRemoto";
#define TSKCOMUNREMOTO_PERIODO_MS 500
#define TSKCOMUNREMOTO_STACK_WD  2048
#define TSKCOMUNREMOTO_PRIORIDAD    2

// Información de intercambio de cada tarea
tareaMedidasNivelInfo_t        tskMedidasNivel_data;
tareaConsumoInfo_t             tskConsumo_data;
tareaControlDepositoInfo_t     tskControlDeposito_data;
tareaComandosConsolaInfo_t     tskComandosConsola_data;
tareaComunicacionDisplayInfo_t tskComunicacionDisplay_data;
tareaComunicacionRemotoInfo_t  tskComunicacionRemoto_data;

// Información necesaria para planificar cada tarea
taskConfig_t tskControlDeposito_config, tskMedidasNivel_config, tskConsumo_config, tskComandosConsola_config, tskComunicacionDisplay_config, tskComunicacionRemoto_config;
taskInfo_t tskControlDeposito_info, tskMedidasNivel_info, tskConsumo_info, tskComandosConsola_info, tskComunicacionDisplay_info, tskComunicacionRemoto_info;

// Referencias a tareas
TaskHandle_t tskControlDeposito, tskMedidasNivel, tskConsumo, tskComandosConsola, tskComunicacionDisplay, tskComunicacionRemoto;

// Referencias a estructuras protegidas
bufferCircular_t medidas;
const char* tagMedidas = "Buffer_Medidas";
bufferCircular_t consumoConsola;
const char* tagConsumoConsola = "Buffer_Consumo_Consola";
bufferCircular_t consumoRemoto;
const char* tagConsumoRemoto = "Buffer_Consumo_Remoto";
estadoSistema_t estadoSistema;
const char* tagEstadoSistema = "Struct_Estado_Sistema";
configSistema_t configSistema;
const char* tagConfigSistema = "Struct_Config_Sistema";
paradaEmergencia_t paradaEmergencia;
const char* tagParadaEmergencia = "Struct_Parada_Emergencia";

// extern "C" void app_main(void)
void app_main(void)
{
    // Preparamos los buffers de intercambio de información
    bufferCircularCrea(&medidas, tagMedidas);
    bufferCircularCrea(&consumoConsola, tagConsumoConsola);
    bufferCircularCrea(&consumoRemoto, tagConsumoRemoto);
    estadoSistemaCrea(&estadoSistema, tagEstadoSistema);
    configSistemaCrea(&configSistema, tagConfigSistema);
    paradaEmergenciaCrea(&paradaEmergencia, tagParadaEmergencia);

    // Prepara la estructura con la información que intercambia cada tarea
    tareaControlDepositoSet(&tskControlDeposito_data, &estadoSistema, &paradaEmergencia);
    tareaMedidasNivelSet(&tskMedidasNivel_data, &medidas, &estadoSistema, &configSistema, &paradaEmergencia);
    tareaConsumoSet(&tskConsumo_data, &medidas, &consumoRemoto, &consumoConsola, &estadoSistema, &configSistema);
    tareaComandosConsolaSet(&tskComandosConsola_data, &estadoSistema, &configSistema, &paradaEmergencia);
    tareaComunicacionDisplaySet(&tskComunicacionDisplay_data, &consumoConsola, &estadoSistema, &paradaEmergencia);
    tareaComunicacionRemotoSet(&tskComunicacionRemoto_data, &consumoRemoto, &estadoSistema, &configSistema, &paradaEmergencia);

    // Completamos las estructuras de paso de información
    // Configuración de cada tarea
    taskConfigSet(&tskControlDeposito_config,   TSKCONTROLDEP_PERIODO_MS,   TSKCONTROLDEP_TAG);
    taskConfigSet(&tskMedidasNivel_config, TSKMEDIDASNIV_PERIODO_MS, TSKMEDIDASNIV_TAG);
    taskConfigSet(&tskConsumo_config,   TSKCONSUMO_PERIODO_MS,   TSKCONSUMO_TAG);
    taskConfigSet(&tskComandosConsola_config,   TSKCOMANDOSCONS_PERIODO_MS,   TSKCOMANDOSCONS_TAG);
    taskConfigSet(&tskComunicacionDisplay_config,   TSKCOMUNDISPLAY_PERIODO_MS,   TSKCOMUNDISPLAY_TAG);
    taskConfigSet(&tskComunicacionRemoto_config,   TSKCOMUNREMOTO_PERIODO_MS,   TSKCOMUNREMOTO_TAG);

    // Información completa de cada tarea
    taskInfoSet(&tskControlDeposito_info,   &tskControlDeposito_config,   (void*)&tskControlDeposito_data);
    taskInfoSet(&tskMedidasNivel_info, &tskMedidasNivel_config, (void*)&tskMedidasNivel_data);
    taskInfoSet(&tskConsumo_info,   &tskConsumo_config,   (void*)&tskConsumo_data);
    taskInfoSet(&tskComandosConsola_info,   &tskComandosConsola_config,   (void*)&tskComandosConsola_data);
    taskInfoSet(&tskComunicacionDisplay_info,   &tskComunicacionDisplay_config,   (void*)&tskComunicacionDisplay_data);
    taskInfoSet(&tskComunicacionRemoto_info,   &tskComunicacionRemoto_config,   (void*)&tskComunicacionRemoto_data);

    // Planifica cada tarea
    xTaskCreate(tareaMedidasNivel, TSKMEDIDASNIV_TAG, TSKMEDIDASNIV_STACK_WD, &tskMedidasNivel_info, TSKMEDIDASNIV_PRIORIDAD,  &tskMedidasNivel); tskMedidasNivel_config.activa = 1; // numTsk++;
    xTaskCreate(tareaControlDeposito,   TSKCONTROLDEP_TAG,   TSKCONTROLDEP_STACK_WD,   &tskMedidasNivel_info,   TSKMEDIDASNIV_PRIORIDAD,    &tskMedidasNivel);   tskControlDeposito_config.activa = 1;   // numTsk++;
    xTaskCreate(tareaConsumo,   TSKCONSUMO_TAG,   TSKCONSUMO_STACK_WD,   &tskConsumo_info,   TSKCONSUMO_PRIORIDAD,    &tskConsumo);   tskConsumo_config.activa = 1;   // numTsk++;
    xTaskCreate(tareaComandosConsola,   TSKCOMANDOSCONS_TAG,   TSKCOMANDOSCONS_STACK_WD,   &tskComandosConsola_info,   TSKCOMANDOSCONS_PRIORIDAD,    &tskComandosConsola);   tskComandosConsola_config.activa = 1;   // numTsk++;
    xTaskCreate(tareaComunicacionDisplay,   TSKCOMUNDISPLAY_TAG,   TSKCOMUNDISPLAY_STACK_WD,   &tskComunicacionDisplay_info,   TSKCOMUNDISPLAY_PRIORIDAD,    &tskComunicacionDisplay);   tskComunicacionDisplay_config.activa = 1;   // numTsk++;
    xTaskCreate(tareaComunicacionRemoto,   TSKCOMUNREMOTO_TAG,   TSKCOMUNREMOTO_STACK_WD,   &tskComunicacionRemoto_info,   TSKCOMUNREMOTO_PRIORIDAD,    &tskComunicacionRemoto);   tskComunicacionRemoto_config.activa = 1;   // numTsk++;

    vTaskDelay(10);

}
