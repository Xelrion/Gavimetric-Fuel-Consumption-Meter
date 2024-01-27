/***********************************************************************************************************
 * MÓDULO DE MONITORIZACIÓN DEL DEPÓSITO
 * Recurso compartido: estado de comandos y notificaciones del sistema
 * Almacena el último comando enviado por el sistema de control (relacionado con la toma de medidas),
 * el estado de la espera de estabilización y la señal digital de petición de medidas del sistema remoto.
 * Los componentes de monitorización revisan este recurso para ejecutar sus tareas.
 ***********************************************************************************************************/

#include "estadoSistema.h"

// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"
#include "freertos/semphr.h"

/* Crea la estructura de estado del sistema */
bool estadoSistemaCrea( estadoSistema_t* pEstadoSist, const char* etiqueta)
{
    /* Reservar un semaforo de exclusión mutua */
    if ((pEstadoSist->mutex = xSemaphoreCreateMutex()) != NULL)
    {
        /* Inicializar estados del sistema */
        pEstadoSist->comando = MEDIDA_OFF;
        pEstadoSist->esperaEstabilizacion = DESACTIVADA;
        pEstadoSist->nivelDeposito = NIVEL_NORMAL;
        pEstadoSist->estadoDeposito = DEPOSITO_NORMAL;
        pEstadoSist->peticionMedidas = 0;
        pEstadoSist->tag = etiqueta;
        pEstadoSist->err = EST_SIST_OK;
    }
    else
    {
        pEstadoSist->err = EST_SIST_ERR_MUTEX;
        ESP_LOGE(pEstadoSist->tag, "Error al intentar crear el mutex");
    }
    return (pEstadoSist->err == EST_SIST_OK);
}

/* Libera los recursos usados por una estructura de estado del sistema */
bool estadoSistemaLibera( estadoSistema_t* pEstadoSist )
{
    /* Libera el semaforo de exclusión mutua */
    vSemaphoreDelete(pEstadoSist->mutex);
    pEstadoSist->err = EST_SIST_OK;
    ESP_LOGD(pEstadoSist->tag, "Mutex liberado");

    return (pEstadoSist->err == EST_SIST_OK);
}

/* Lee el estado del comando */
bool estadoSistemaLeerComando( estadoSistema_t* pEstadoSist, estadoSistemaComando_t* pComando )
{
    if (xSemaphoreTake(pEstadoSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        *pComando = pEstadoSist->comando;
        ESP_LOGD(pEstadoSist->tag, "Comando actual del sistema: %d", (pEstadoSist->comando));
        pEstadoSist->err = EST_SIST_OK;
        xSemaphoreGive(pEstadoSist->mutex);
    }
    else
    {
        ESP_LOGE(pEstadoSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pEstadoSist->tag, "para leer el comando actual: %d", (pEstadoSist->comando));

        pEstadoSist->err = EST_SIST_ERR_MUTEX;
    }

    return (pEstadoSist->err == EST_SIST_OK);
}

/* Lee el estado de la espera de estabilización */
bool estadoSistemaLeerEspera( estadoSistema_t* pEstadoSist, estadoSistemaEspera_t* pEspera )
{
    if (xSemaphoreTake(pEstadoSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        *pEspera = pEstadoSist->esperaEstabilizacion;
        ESP_LOGD(pEstadoSist->tag, "Espera de estabilización: %d", (pEstadoSist->esperaEstabilizacion));
        pEstadoSist->err = EST_SIST_OK;
        xSemaphoreGive(pEstadoSist->mutex);
    }
    else
    {
        ESP_LOGE(pEstadoSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pEstadoSist->tag, "para leer el estado de la espera de estabilización: %d", (pEstadoSist->esperaEstabilizacion));

        pEstadoSist->err = EST_SIST_ERR_MUTEX;
    }

    return (pEstadoSist->err == EST_SIST_OK);
}

/* Lee el aviso de límite de nivel del depósito */
bool estadoSistemaLeerNivel( estadoSistema_t* pEstadoSist, estadoSistemaNivel_t* pNivel )
{
    if (xSemaphoreTake(pEstadoSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        *pNivel = pEstadoSist->nivelDeposito;
        ESP_LOGD(pEstadoSist->tag, "Nivel del depósito: %d", (pEstadoSist->nivelDeposito));
        pEstadoSist->err = EST_SIST_OK;
        xSemaphoreGive(pEstadoSist->mutex);
    }
    else
    {
        ESP_LOGE(pEstadoSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pEstadoSist->tag, "para leer el aviso de nivel del depósito: %d", (pEstadoSist->nivelDeposito));

        pEstadoSist->err = EST_SIST_ERR_MUTEX;
    }

    return (pEstadoSist->err == EST_SIST_OK);
}

/* Lee el estado actual del depósito */
bool estadoSistemaLeerDeposito( estadoSistema_t* pEstadoSist, estadoSistemaDeposito_t* pDeposito )
{
    if (xSemaphoreTake(pEstadoSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        *pDeposito = pEstadoSist->estadoDeposito;
        ESP_LOGD(pEstadoSist->tag, "Estado del depósito: %d", (pEstadoSist->estadoDeposito));
        pEstadoSist->err = EST_SIST_OK;
        xSemaphoreGive(pEstadoSist->mutex);
    }
    else
    {
        ESP_LOGE(pEstadoSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pEstadoSist->tag, "para leer el estado del depósito: %d", (pEstadoSist->estadoDeposito));

        pEstadoSist->err = EST_SIST_ERR_MUTEX;
    }

    return (pEstadoSist->err == EST_SIST_OK);
}

/* Lee el estado de la petición de medidas remotas */
bool estadoSistemaLeerPeticion( estadoSistema_t* pEstadoSist )
{
    bool peticionActiva = false;

    if (xSemaphoreTake(pEstadoSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        peticionActiva = pEstadoSist->peticionMedidas;
        ESP_LOGD(pEstadoSist->tag, "Petición de medidas del sist. remoto: %d", (pEstadoSist->peticionMedidas));
        pEstadoSist->err = EST_SIST_OK;
        xSemaphoreGive(pEstadoSist->mutex);
    }
    else
    {
        ESP_LOGE(pEstadoSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pEstadoSist->tag, "para leer el estado de la petición de medidas remotas: %d", (pEstadoSist->peticionMedidas));

        pEstadoSist->err = EST_SIST_ERR_MUTEX;
    }

    return peticionActiva;
}

/* Modifica el estado del comando */
bool estadoSistemaEscribirComando( estadoSistema_t* pEstadoSist, estadoSistemaComando_t pComando )
{
    if (xSemaphoreTake(pEstadoSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        pEstadoSist->comando = pComando;
        ESP_LOGD(pEstadoSist->tag, "Nuevo comando del sistema: %d", (pEstadoSist->comando));
        pEstadoSist->err = EST_SIST_OK;
        xSemaphoreGive(pEstadoSist->mutex);
    }
    else
    {
        ESP_LOGE(pEstadoSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pEstadoSist->tag, "para modificar el comando actual: %d", (pEstadoSist->comando));

        pEstadoSist->err = EST_SIST_ERR_MUTEX;
    }

    return (pEstadoSist->err == EST_SIST_OK);
}

/* Modifica el estado de la espera de estabilización */
bool estadoSistemaEscribirEspera( estadoSistema_t* pEstadoSist, estadoSistemaEspera_t pEspera )
{
    if (xSemaphoreTake(pEstadoSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        pEstadoSist->esperaEstabilizacion = pEspera;
        ESP_LOGD(pEstadoSist->tag, "Espera de estabilización: %d", (pEstadoSist->esperaEstabilizacion));
        pEstadoSist->err = EST_SIST_OK;
        xSemaphoreGive(pEstadoSist->mutex);
    }
    else
    {
        ESP_LOGE(pEstadoSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pEstadoSist->tag, "para modificar el estado de la espera de estabilización: %d", (pEstadoSist->esperaEstabilizacion));

        pEstadoSist->err = EST_SIST_ERR_MUTEX;
    }

    return (pEstadoSist->err == EST_SIST_OK);
}

/* Modifica el aviso de límite de nivel del depósito */
bool estadoSistemaEscribirNivel( estadoSistema_t* pEstadoSist, estadoSistemaNivel_t pNivel )
{
    if (xSemaphoreTake(pEstadoSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        pEstadoSist->nivelDeposito = pNivel;
        ESP_LOGD(pEstadoSist->tag, "Nivel del depósito: %d", (pEstadoSist->nivelDeposito));
        pEstadoSist->err = EST_SIST_OK;
        xSemaphoreGive(pEstadoSist->mutex);
    }
    else
    {
        ESP_LOGE(pEstadoSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pEstadoSist->tag, "para modificar el aviso de nivel del depósito: %d", (pEstadoSist->nivelDeposito));

        pEstadoSist->err = EST_SIST_ERR_MUTEX;
    }

    return (pEstadoSist->err == EST_SIST_OK);
}

/* Modifica el aviso de límite de nivel del depósito */
bool estadoSistemaEscribirDeposito( estadoSistema_t* pEstadoSist, estadoSistemaDeposito_t pDeposito )
{
    if (xSemaphoreTake(pEstadoSist->mutex, (TickType_t) 10) == pdTRUE)
    {
        pEstadoSist->estadoDeposito = pDeposito;
        ESP_LOGD(pEstadoSist->tag, "Estado del depósito: %d", (pEstadoSist->estadoDeposito));
        pEstadoSist->err = EST_SIST_OK;
        xSemaphoreGive(pEstadoSist->mutex);
    }
    else
    {
        ESP_LOGE(pEstadoSist->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pEstadoSist->tag, "para modificar el estado del depósito: %d", (pEstadoSist->estadoDeposito));

        pEstadoSist->err = EST_SIST_ERR_MUTEX;
    }

    return (pEstadoSist->err == EST_SIST_OK);
}