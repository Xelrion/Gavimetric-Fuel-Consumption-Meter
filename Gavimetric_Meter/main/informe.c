/* Implmentación del */
/* Módulo para gestionar el cálculo de valores medios bajo exclusión mutua */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

#include "myTaskConfig.h"
#include "bufferCircular.h"
#include "informe.h"

/* Configuración de la tarea de cálculo de medias */
void tareaInformeSet(tareaInformeInfo_t* pTaskInfo, bufferCircular_t* pMedias)
{
    pTaskInfo->pMedias   = pMedias;
}

/* Punto de entrada de la tarea de informe de valores medios */
void tareaInforme(void* pParametros)
{
    /* Estructuras para intercambio de información */
    /* Entre la tarea y la aplicación principal */
    taskConfig_t* pConfig = ((taskInfo_t *)pParametros)->pConfig;
    void*           pData = ((taskInfo_t *)pParametros)->pData;
    bufferCircular_t*  pMedias   = ((tareaInformeInfo_t*)pData)->pMedias;

    ESP_LOGI(pConfig->tag, "Periodo de planificación: %lu ms", pConfig->periodo);
    ESP_LOGI(pConfig->tag, "Número inicial de activaciones: %lu", pConfig->numActivaciones);

    /* Prepara la tarea */
    const TickType_t periodo = (pConfig->periodo) / portTICK_PERIOD_MS;
    TickType_t activacionPrevia = xTaskGetTickCount();

    /* Bucle de presentación de medias */

    bool continuar = true;
    double valorMedio;
    int numValores;

    while( continuar )
    {
        /* Espera a la siguiente activación */  
        xTaskDelayUntil(&activacionPrevia, periodo);
        pConfig->numActivaciones++;
        ESP_LOGI(pConfig->tag, "Numero de activaciones: %lu", pConfig->numActivaciones);

        valorMedio = 0.0;
        numValores = 0;

        /* Muestra las medias tomadas hasta este momento */
        while (!bufferCircularVacio(pMedias)) {
            bufferCircularSaca(pMedias, &valorMedio);
            ESP_LOGI(pConfig->tag, "elemento: %03d\t valor: %6.2f ", numValores, valorMedio);

            numValores++;
        }
    }

}
