/* Aplicación principal de demostración del uso de buffers para compartir información */

// #include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"

#include "myTaskConfig.h"
#include "bufferCircular.h"

#include "analogInput.h"
#include "calculaMedias.h"
#include "informe.h"

// Constantes para la planificación de cada tarea
// Nombre, periodo, tamaño de stack y prioridad

static const char* TSKLECTURA_TAG = "LecturaSensor";
#define TSKLECTURA_PERIODO_MS 500
#define TSKLECTURA_STACK_WD  2048
#define TSKLECTURA_PRIORIDAD    7

static const char* TSKMEDIA_TAG = "CalculoMedia";
#define TSKMEDIA_PERIODO_MS 6000
#define TSKMEDIA_STACK_WD  2048
#define TSKMEDIA_PRIORIDAD    4

static const char* TSKINFORME_TAG = "Informe";
#define TSKINFORME_PERIODO_MS 60000
#define TSKINFORME_STACK_WD  2048
#define TSKINFORME_PRIORIDAD    2

// Información de intercambio de cada tarea
tareaLecturaInfo_t tskLectura_data;
tareaMediaInfo_t   tskMedia_data;
tareaInformeInfo_t tskInforme_data;

// Información necesaria para planificar cada tarea
taskConfig_t tskLectura_config, tskMedia_config, tskInforme_config;
taskInfo_t tskLectura_info, tskMedia_info, tskInforme_info;

TaskHandle_t tskLectura, tskMedia, tskInforme;

bufferCircular_t lecturas;
const char* tagLecturas = "Buffer_Lecturas";

bufferCircular_t medias;
const char* tagMedias = "Buffer_Medias";

// extern "C" void app_main(void)
void app_main(void)
{
    // Preparamos los buffers de intercambio de información
    bufferCircularCrea(&lecturas, tagLecturas);
    bufferCircularCrea(&medias, tagMedias);

    // Prepara la estructura con la información que intercambia cada taera
    tareaLecturaSet(&tskLectura_data, &lecturas);
    tareaMediaSet(&tskMedia_data, &lecturas, &medias);
    tareaInformeSet(&tskInforme_data, &medias);

    // Completamos las estructuras de paso de información
    // Configuración de cada tarea
    taskConfigSet(&tskLectura_config, TSKLECTURA_PERIODO_MS, TSKLECTURA_TAG);
    taskConfigSet(&tskMedia_config,   TSKMEDIA_PERIODO_MS,   TSKMEDIA_TAG);
    taskConfigSet(&tskInforme_config, TSKINFORME_PERIODO_MS, TSKINFORME_TAG);

    // Información completa de cada tarea
    taskInfoSet(&tskLectura_info, &tskLectura_config, (void*)&tskLectura_data);
    taskInfoSet(&tskMedia_info,   &tskMedia_config,   (void*)&tskMedia_data);
    taskInfoSet(&tskInforme_info, &tskInforme_config, (void*)&tskInforme_data);

    // Planifica cada tarea
    xTaskCreate(tareaLectura, TSKLECTURA_TAG, TSKLECTURA_STACK_WD, &tskLectura_info, TSKLECTURA_PRIORIDAD,  &tskLectura); tskLectura_config.activa = 1; // numTsk++;
    xTaskCreate(tareaMedia,   TSKMEDIA_TAG,   TSKMEDIA_STACK_WD,   &tskMedia_info,   TSKMEDIA_PRIORIDAD,    &tskMedia);   tskMedia_config.activa = 1;   // numTsk++;
    xTaskCreate(tareaInforme, TSKINFORME_TAG, TSKINFORME_STACK_WD, &tskInforme_info, TSKINFORME_PRIORIDAD,  &tskInforme); tskInforme_config.activa = 1; // numTsk++;

    vTaskDelay(10);

}
