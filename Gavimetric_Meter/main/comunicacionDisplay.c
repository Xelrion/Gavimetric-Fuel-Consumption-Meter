/***********************************************************************************************************
 * MÓDULO DE CONSOLA
 * Objeto cíclico: envío de datos al display de control
 * Actualiza periódicamente la información mostrada en el display del panel de control
 ***********************************************************************************************************/

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

#include "myTaskConfig.h"
#include "bufferCircular.h"
#include "estadoSistema.h"
#include "configSistema.h"
#include "paradaEmergencia.h"
#include "comunicacionDisplay.h"

/* Etiqueta para depuración */
static char* TAG = "comunicacionDisplay";

/***********************************************************************************************************
 * Funciones de comunicación con el display
 ***********************************************************************************************************/

void actualizar_consumo( double medidasConsumo )
{
    /* Añade una nueva medida al display */
    ESP_LOGI("Comunicación Display", "Consumo a display: %f", medidasConsumo);
}

void actualizar_modo_funcionamiento( estadoSistemaComando_t comando )
{
    /* Actualiza el modo de funcionamiento mostrado en el display: medidas desactivadas, medida puntual, medida continuada o automático/remoto */
}

void actualizar_estado_deposito( estadoSistemaDeposito_t estadoDeposito )
{
    /* Actualiza el estado actual del depósito: normal, llenado, vaciado */
}

void notificar_nivel_deposito( estadoSistemaNivel_t nivelDeposito )
{
    /* Notifica sobre el nivel actual del depósito: normal, mínimo, máximo */
}

void notificar_emergencia()
{
    /* Notifica sobre el estado de emergencia activo */
}

/***********************************************************************************************************
 * Punto de entrada de la tarea de comunicación con el display
 ***********************************************************************************************************/

/* Configuración de la tarea de comunicación con el display */
void tareaComunicacionDisplaySet(tareaComunicacionDisplayInfo_t* pTaskInfo, bufferCircular_t* pConsumoConsola, estadoSistema_t* pEstadoSist, paradaEmergencia_t* pEmergencia)
{
    pTaskInfo->pConsumoConsola = pConsumoConsola;
    pTaskInfo->pEstadoSist = pEstadoSist;
    pTaskInfo->pEmergencia = pEmergencia;
}

/* Punto de entrada de la tarea de comunicación con el display */
void tareaComunicacionDisplay(void* pParametros)
{
    /* Estructuras para intercambio de información */
    /* Entre la tarea y la aplicación principal */
    taskConfig_t*            pConfig  = ((taskInfo_t *)pParametros)->pConfig;
    void*                      pData  = ((taskInfo_t *)pParametros)->pData;
    bufferCircular_t* pConsumoConsola = ((tareaComunicacionDisplayInfo_t*)pData)->pConsumoConsola;
    estadoSistema_t*     pEstadoSist  = ((tareaComunicacionDisplayInfo_t*)pData)->pEstadoSist;
    paradaEmergencia_t*  pEmergencia  = ((tareaComunicacionDisplayInfo_t*)pData)->pEmergencia;

    ESP_LOGI(pConfig->tag, "Periodo de planificación: %lu ms", pConfig->periodo);
    ESP_LOGI(pConfig->tag, "Número inicial de activaciones: %lu", pConfig->numActivaciones);

    /* Prepara la tarea */
    const TickType_t periodo = (pConfig->periodo) / portTICK_PERIOD_MS;
    TickType_t activacionPrevia = xTaskGetTickCount();

    /* Estado del sistema */
    estadoSistemaComando_t comando;
    estadoSistemaDeposito_t estadoDeposito;
    estadoSistemaNivel_t nivelDeposito;
    bool emergencia;

    /* Bucle de comunicación con el display */
    bool continuar = true;
    double medidaConsumo = 0;

    while( continuar )
    {
        /* Espera a la siguiente activación */  
        xTaskDelayUntil(&activacionPrevia, periodo);

        pConfig->numActivaciones++;
        ESP_LOGD(pConfig->tag, "Numero de activaciones: %lu", pConfig->numActivaciones);

        /* Comprueba si la parada de emergencia se encuentra activa */
        paradaEmergenciaLeer(pEmergencia, &emergencia);
        /* Lee el último comando de petición de medidas del sistema */
        estadoSistemaLeerComando(pEstadoSist, &comando);

        /* NOTIFICACIÓN DE PARADA DE EMERGENCIA */
        if (emergencia) { notificar_emergencia(); }

        /* ACTUALIZACIÓN DE ESTADO DE LA CONSOLA */
        if (!emergencia)
        {
            actualizar_modo_funcionamiento(comando);
            actualizar_estado_deposito(estadoDeposito);
            notificar_nivel_deposito(nivelDeposito);
        }

        /* ENVÍO DE MEDIDAS DE CONSUMO A LA CONSOLA */
        /* Si las medidas están desactivadas o en modo remoto, se limpia el buffer */
        if (emergencia || comando == MEDIDA_OFF || comando == MEDIDA_REMOTO )
        {
            if( !bufferCircularLimpia(pConsumoConsola) ) { continuar = false; }
        }
        /* Si el modo de medida puntual está activo, solo se envía una medida a la consola y se actualiza el estado */
        if (!emergencia && !bufferCircularVacio(pConsumoConsola) && comando == MEDIDA_MANUAL_PUNTUAL)
        {
            if( !bufferCircularSaca(pConsumoConsola, &medidaConsumo) ) { continuar = false; }
            actualizar_consumo(medidaConsumo);
            if( !estadoSistemaEscribirComando(pEstadoSist, MEDIDA_OFF) ) { continuar = false; }
        }
        /* Si el modo de medida continuada está activo, se envían todas las medidas disponibles a la consola */
        while (!emergencia && !bufferCircularVacio(pConsumoConsola) && comando == MEDIDA_MANUAL_CONTINUADA)
        {
            if( !bufferCircularSaca(pConsumoConsola, &medidaConsumo) ) { continuar = false; }
            actualizar_consumo(medidaConsumo);
        }
    }
}