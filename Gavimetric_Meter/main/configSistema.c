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

/* Crea la estructura de configuración del sistema */
bool configSistemaCrea( configSistema_t* pConfigSist, const char* etiqueta)
{
    /* Reservar un semaforo de exclusión mutua */
    if ((pConfigSist->mutex = xSemaphoreCreateMutex()) != NULL)
    {
        /* Inicializar parámetros del sistema con los valores por defecto */
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

/* Libera los recursos usados por una estructura de configuración del sistema */
bool configSistemaLibera( configSistema_t* pConfigSist )
{
    /* Libera el semaforo de exclusión mutua */
    vSemaphoreDelete(pConfigSist->mutex);
    pConfigSist->err = CONFIG_SIST_OK;
    ESP_LOGD(pConfigSist->tag, "Mutex liberado");

    return (pConfigSist->err == CONFIG_SIST_OK);
}

/* Lee el periodo de medidas */
bool configSistemaLeerPeriodo( configSistema_t* pConfigSist, double* pPeriodoMedida )
{
    if (xSemaphoreTake(pConfigSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        *pPeriodoMedida = pConfigSist->periodoMedida;
        ESP_LOGD(pConfigSist->tag, "Periodo de medidas: %d", (pConfigSist->periodoMedida));
        pConfigSist->err = CONFIG_SIST_OK;
        xSemaphoreGive(pConfigSist->mutex);
    }
    else
    {
        ESP_LOGE(pConfigSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pConfigSist->tag, "para leer el periodo de medidas: %d", (pConfigSist->periodoMedida));

        pConfigSist->err = CONFIG_SIST_ERR_MUTEX;
    }

    return (pConfigSist->err == CONFIG_SIST_OK);
}

/* Lee la espera de estabilización */
bool configSistemaLeerEspera( configSistema_t* pConfigSist, int* pEspera )
{
    if (xSemaphoreTake(pConfigSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        *pEspera = pConfigSist->esperaEstabilizacion;
        ESP_LOGD(pConfigSist->tag, "Tiempo de espera: %d", (pConfigSist->esperaEstabilizacion));
        pConfigSist->err = CONFIG_SIST_OK;
        xSemaphoreGive(pConfigSist->mutex);
    }
    else
    {
        ESP_LOGE(pConfigSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pConfigSist->tag, "para leer el tiempo de espera de estabilización: %d", (pConfigSist->esperaEstabilizacion));

        pConfigSist->err = CONFIG_SIST_ERR_MUTEX;
    }

    return (pConfigSist->err == CONFIG_SIST_OK);
}

/* Lee el consumo máximo de combustible */
bool configSistemaLeerConsumoMax( configSistema_t* pConfigSist, int* pConsumoMax )
{
    if (xSemaphoreTake(pConfigSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        *pConsumoMax = pConfigSist->consumoMaximo;
        ESP_LOGD(pConfigSist->tag, "Consumo máximo: %d", (pConfigSist->consumoMaximo));
        pConfigSist->err = CONFIG_SIST_OK;
        xSemaphoreGive(pConfigSist->mutex);
    }
    else
    {
        ESP_LOGE(pConfigSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pConfigSist->tag, "para leer el consumo máximo de combustible: %d", (pConfigSist->consumoMaximo));

        pConfigSist->err = CONFIG_SIST_ERR_MUTEX;
    }

    return (pConfigSist->err == CONFIG_SIST_OK);
}

/* Lee el nivel máximo de combustible */
bool configSistemaLeerNivelMax( configSistema_t* pConfigSist, int* pNivelMax )
{
    if (xSemaphoreTake(pConfigSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        *pNivelMax = pConfigSist->nivelMaximo;
        ESP_LOGD(pConfigSist->tag, "Nivel máximo: %d", (pConfigSist->nivelMaximo));
        pConfigSist->err = CONFIG_SIST_OK;
        xSemaphoreGive(pConfigSist->mutex);
    }
    else
    {
        ESP_LOGE(pConfigSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pConfigSist->tag, "para leer el nivel máximo de combustible: %d", (pConfigSist->nivelMaximo));

        pConfigSist->err = CONFIG_SIST_ERR_MUTEX;
    }

    return (pConfigSist->err == CONFIG_SIST_OK);
}

/* Lee el nivel mínimo de combustible */
bool configSistemaLeerNivelMin( configSistema_t* pConfigSist, int* pNivelMin )
{
    if (xSemaphoreTake(pConfigSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        *pNivelMin = pConfigSist->nivelMinimo;
        ESP_LOGD(pConfigSist->tag, "Nivel mínimo: %d", (pConfigSist->nivelMinimo));
        pConfigSist->err = CONFIG_SIST_OK;
        xSemaphoreGive(pConfigSist->mutex);
    }
    else
    {
        ESP_LOGE(pConfigSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pConfigSist->tag, "para leer el nivel mínimo de combustible: %d", (pConfigSist->nivelMinimo));

        pConfigSist->err = CONFIG_SIST_ERR_MUTEX;
    }

    return (pConfigSist->err == CONFIG_SIST_OK);
}