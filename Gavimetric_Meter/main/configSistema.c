/***********************************************************************************************************
 * MÓDULO DE MONITORIZACIÓN DEL DEPÓSITO
 * Recurso compartido: parámetros configurables del sistema
 * Almacena los valores de los parámetros configurables por el usuario:
 * Periodo de toma de medidas
 * Espera de estabilización
 * Consumo máximo de combustible
 * Nivel mínimo de combustible en el depósito
 * Nivel máximo de combustible en el depósito
 ***********************************************************************************************************/

#include "configSistema.h"

// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"
#include "freertos/semphr.h"

/* Crea la estructura de estado del sistema */
bool configSistemaCrea( configSistema_t* pConfigSist, const char* etiqueta)
{
    /* Reservar un semaforo de exclusión mutua */
    if ((pConfigSist->mutex = xSemaphoreCreateMutex()) != NULL)
    {
        /* Inicializar estados del sistema con los valores por defecto */
        pConfigSist->periodoMedida = 500;   // ms
        pConfigSist->esperaEstabilizacion = 5;  // s
        pConfigSist->consumoMaximo = 10;
        pConfigSist->nivelMaximo = 100;
        pConfigSist->nivelMinimo = 5;
        pConfigSist->tag = etiqueta;
        pConfigSist->err = CONFIG_SIST_OK;
    }
    else
    {
        pConfigSist->err = CONFIG_SIST_ERR_MUTEX;
        ESP_LOGE(pConfigSist->tag, "Error al intentar crear el mutex");
    }
    return (pConfigSist->err == CONFIG_SIST_OK);
}

/* Libera los recursos usados por una estructura de estado del sistema */
bool configSistemaLibera( configSistema_t* pConfigSist )
{
    /* Libera el semaforo de exclusión mutua */
    vSemaphoreDelete(pConfigSist->mutex);
    pConfigSist->err = CONFIG_SIST_OK;
    ESP_LOGD(pConfigSist->tag, "Mutex liberado");

    return (pConfigSist->err == CONFIG_SIST_OK);
}

/* Lee el periodo de medidas */
bool configSistemaLeerPeriodo( configSistema_t* pEstadoSist, double* pPeriodoMedida )
{
    if (xSemaphoreTake(pEstadoSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        *pPeriodoMedida = pEstadoSist->periodoMedida;
        ESP_LOGD(pEstadoSist->tag, "Periodo de medidas: %d", (pEstadoSist->periodoMedida));
        pEstadoSist->err = CONFIG_SIST_OK;
        xSemaphoreGive(pEstadoSist->mutex);
    }
    else
    {
        ESP_LOGE(pEstadoSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pEstadoSist->tag, "para leer el periodo de medidas: %d", (pEstadoSist->periodoMedida));

        pEstadoSist->err = CONFIG_SIST_ERR_MUTEX;
    }

    return (pEstadoSist->err == CONFIG_SIST_OK);
}

/* Lee la espera de estabilización */
bool configSistemaLeerEspera( configSistema_t* pEstadoSist, int* pEspera )
{
    if (xSemaphoreTake(pEstadoSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        *pEspera = pEstadoSist->esperaEstabilizacion;
        ESP_LOGD(pEstadoSist->tag, "Tiempo de espera: %d", (pEstadoSist->esperaEstabilizacion));
        pEstadoSist->err = CONFIG_SIST_OK;
        xSemaphoreGive(pEstadoSist->mutex);
    }
    else
    {
        ESP_LOGE(pEstadoSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pEstadoSist->tag, "para leer el tiempo de espera de estabilización: %d", (pEstadoSist->esperaEstabilizacion));

        pEstadoSist->err = CONFIG_SIST_ERR_MUTEX;
    }

    return (pEstadoSist->err == CONFIG_SIST_OK);
}

/* Lee el consumo máximo de combustible */
bool configSistemaLeerConsumoMax( configSistema_t* pEstadoSist, int* pConsumoMax )
{
    if (xSemaphoreTake(pEstadoSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        *pConsumoMax = pEstadoSist->consumoMaximo;
        ESP_LOGD(pEstadoSist->tag, "Consumo máximo: %d", (pEstadoSist->consumoMaximo));
        pEstadoSist->err = CONFIG_SIST_OK;
        xSemaphoreGive(pEstadoSist->mutex);
    }
    else
    {
        ESP_LOGE(pEstadoSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pEstadoSist->tag, "para leer el consumo máximo de combustible: %d", (pEstadoSist->consumoMaximo));

        pEstadoSist->err = CONFIG_SIST_ERR_MUTEX;
    }

    return (pEstadoSist->err == CONFIG_SIST_OK);
}

/* Lee el nivel máximo de combustible */
bool configSistemaLeerNivelMax( configSistema_t* pEstadoSist, int* pNivelMax )
{
    if (xSemaphoreTake(pEstadoSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        *pNivelMax = pEstadoSist->nivelMaximo;
        ESP_LOGD(pEstadoSist->tag, "Nivel máximo: %d", (pEstadoSist->nivelMaximo));
        pEstadoSist->err = CONFIG_SIST_OK;
        xSemaphoreGive(pEstadoSist->mutex);
    }
    else
    {
        ESP_LOGE(pEstadoSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pEstadoSist->tag, "para leer el nivel máximo de combustible: %d", (pEstadoSist->nivelMaximo));

        pEstadoSist->err = CONFIG_SIST_ERR_MUTEX;
    }

    return (pEstadoSist->err == CONFIG_SIST_OK);
}

/* Lee el nivel mínimo de combustible */
bool configSistemaLeerNivelMin( configSistema_t* pEstadoSist, int* pNivelMin )
{
    if (xSemaphoreTake(pEstadoSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        *pNivelMin = pEstadoSist->nivelMinimo;
        ESP_LOGD(pEstadoSist->tag, "Nivel mínimo: %d", (pEstadoSist->nivelMinimo));
        pEstadoSist->err = CONFIG_SIST_OK;
        xSemaphoreGive(pEstadoSist->mutex);
    }
    else
    {
        ESP_LOGE(pEstadoSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pEstadoSist->tag, "para leer el nivel mínimo de combustible: %d", (pEstadoSist->nivelMinimo));

        pEstadoSist->err = CONFIG_SIST_ERR_MUTEX;
    }

    return (pEstadoSist->err == CONFIG_SIST_OK);
}