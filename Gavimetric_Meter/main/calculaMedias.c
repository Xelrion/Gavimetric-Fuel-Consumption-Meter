/* Implmentación del */
/* Módulo para gestionar el cálculo de valores medios bajo exclusión mutua */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

#include "myTaskConfig.h"
#include "bufferCircular.h"
#include "calculaMedias.h"

/* Configuración de la tarea de cálculo de medias */
void tareaMediaSet(tareaMediaInfo_t* pTaskInfo, bufferCircular_t* pLecturas, bufferCircular_t* pMedias)
{
    pTaskInfo->pLecturas = pLecturas;
    pTaskInfo->pMedias   = pMedias;
}

/* Punto de entrada de la tarea de cálculo de valores medios */
void tareaMedia(void* pParametros)
{
    /* Estructuras para intercambio de información */
    /* Entre la tarea y la aplicación principal */
    taskConfig_t* pConfig = ((taskInfo_t *)pParametros)->pConfig;
    void*           pData = ((taskInfo_t *)pParametros)->pData;
    bufferCircular_t*  pLecturas = ((tareaMediaInfo_t*)pData)->pLecturas;
    bufferCircular_t*  pMedias   = ((tareaMediaInfo_t*)pData)->pMedias;

    ESP_LOGD(pConfig->tag, "Periodo de planificación: %lu ms", pConfig->periodo);
    ESP_LOGD(pConfig->tag, "Número inicial de activaciones: %lu", pConfig->numActivaciones);
    // ESP_LOGI(pConfig->tag, "LED en pin %u. Nivel inicial: %s", pLED->pin, (pLED->nivel==0)? "LOW":"HIGH");
    // ESP_LOGI(pConfig->tag, "Mutex en %x", (unsigned int)(pLED->sem));

    /* Prepara la tarea */
    const TickType_t periodo = (pConfig->periodo) / portTICK_PERIOD_MS;
    TickType_t activacionPrevia = xTaskGetTickCount();

    /* Bucle de calculo de medias */

    bool continuar = true;
    double medida, valorMedio;
    int numValores;

    while( continuar )
    {
        /* Espera a la siguiente activación */  
        xTaskDelayUntil(&activacionPrevia, periodo);
        pConfig->numActivaciones++;
        ESP_LOGD(pConfig->tag, "Numero de activaciones: %lu", pConfig->numActivaciones);

        /* Prepara el cálculo de la nueva media */
        valorMedio = 0.0;
        numValores = 0;

        /* Lee y acumula las medidas tomadas hasta este momento */
        while (!bufferCircularVacio(pLecturas)) {
            bufferCircularSaca(pLecturas, &medida);
            valorMedio += medida;
            numValores++;
        }
        if (numValores) {
            valorMedio = valorMedio / numValores;

            /* Incluye el valor medio en el buffer de medias */
            if (!bufferCircularMete(pMedias, valorMedio)) {
                continuar = false;
            }

            ESP_LOGI(pConfig->tag, "Valor medio: %f", valorMedio);
            ESP_LOGD(pConfig->tag, "a partir de %d datos", numValores);
        }
    }

}
