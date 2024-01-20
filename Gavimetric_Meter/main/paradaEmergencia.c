/***********************************************************************************************************
 * MÓDULO DE PARADA DE EMERGENCIA
 * Recurso compartido: estado de emergencia
 * Almacena el estado de emergencia: activo o inactivo
 * Solo puede ser activado por desbordamiento o por comando de consola
 * Solo puede ser desactivado mediante comando de consola
 * Afecta a la ejecución de la mayoría de tareas
 ***********************************************************************************************************/

#include "paradaEmergencia.h"

// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"
#include "freertos/semphr.h"

/* Crea la estructura de parada de emergencia*/
bool paradaEmergenciaCrea( paradaEmergencia_t* pEmergencia, const char* etiqueta)
{
    /* Reservar un semaforo de exclusión mutua */
    if ((pEmergencia->mutex = xSemaphoreCreateMutex()) != NULL)
    {
        /* Inicializa la estructura de parada de emergencia */
        pEmergencia->paradaEmergencia = false;
        pEmergencia->tag = etiqueta;
        pEmergencia->err = EMERGENCIA_OK;
    }
    else
    {
        pEmergencia->err = EMERGENCIA_ERR_MUTEX;
        ESP_LOGE(pEmergencia->tag, "Error al intentar crear el mutex");
    }
    return (pEmergencia->err == EMERGENCIA_OK);
}

/* Libera los recursos usados por una estructura de parada de emergencia */
bool paradaEmergenciaLibera( paradaEmergencia_t* pEmergencia )
{
    /* Libera el semaforo de exclusión mutua */
    vSemaphoreDelete(pEmergencia->mutex);
    pEmergencia->err = EMERGENCIA_OK;
    ESP_LOGD(pEmergencia->tag, "Mutex liberado");

    return (pEmergencia->err == EMERGENCIA_OK);
}

/* Activa la parada de emergencia */
bool paradaEmergenciaActivar( paradaEmergencia_t* pEmergencia )
{
    if (xSemaphoreTake(pEmergencia->mutex, (TickType_t) 10) == pdTRUE)
    {
        pEmergencia->paradaEmergencia = true;
        ESP_LOGD(pEmergencia->tag, "Estado parada emergencia: %d", (pEmergencia->paradaEmergencia));
        pEmergencia->err = EMERGENCIA_OK;
        xSemaphoreGive(pEmergencia->mutex);
    }
    else
    {
        ESP_LOGE(pEmergencia->tag, "Fallo al intentar activar la parada de emergencia");

        pEmergencia->err = EMERGENCIA_ERR_MUTEX;
    }

    return (pEmergencia->err == EMERGENCIA_OK);
}

/* Desactiva la parada de emergencia */
bool paradaEmergenciaDesactivar( paradaEmergencia_t* pEmergencia )
{
    if (xSemaphoreTake(pEmergencia->mutex, (TickType_t) 10) == pdTRUE)
    {
        pEmergencia->paradaEmergencia = false;
        ESP_LOGD(pEmergencia->tag, "Estado parada emergencia: %d", (pEmergencia->paradaEmergencia));
        pEmergencia->err = EMERGENCIA_OK;
        xSemaphoreGive(pEmergencia->mutex);
    }
    else
    {
        ESP_LOGE(pEmergencia->tag, "Fallo al intentar desactivar la parada de emergencia");

        pEmergencia->err = EMERGENCIA_ERR_MUTEX;
    }

    return (pEmergencia->err == EMERGENCIA_OK);
}

/* Lee el estado de la parada de emergencia */
bool paradaEmergenciaLeer( paradaEmergencia_t* pEmergencia, bool* pEstadoEmergencia )
{
    if (xSemaphoreTake(pEmergencia->mutex, (TickType_t) 10) == pdTRUE)
    {
        *pEstadoEmergencia = pEmergencia->paradaEmergencia;
        ESP_LOGD(pEmergencia->tag, "Estado parada emergencia: %d", (pEmergencia->paradaEmergencia));
        pEmergencia->err = EMERGENCIA_OK;
        xSemaphoreGive(pEmergencia->mutex);
    }
    else
    {
        ESP_LOGE(pEmergencia->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pEmergencia->tag, "para leer el estado de la parada de emergencia: %d", (pEmergencia->paradaEmergencia));

        pEmergencia->err = EMERGENCIA_ERR_MUTEX;
    }

    return (pEmergencia->err == EMERGENCIA_OK);
}