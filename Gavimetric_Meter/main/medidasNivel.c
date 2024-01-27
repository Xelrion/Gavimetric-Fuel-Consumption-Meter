/***********************************************************************************************************
 * MÓDULO DE MONITORIZACIÓN DEL DEPÓSITO
 * Objeto cíclico: Toma de medidas y comprobación de nivel
 * Toma medidas periódicas de la báscula, comprueba si superan el nivel máximo o mínimo, y las almacena en un buffer
 ***********************************************************************************************************/

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LOG_LOCAL_LEVEL ESP_LOG_NONE
#include "esp_log.h"

#include "bufferCircular.h"
#include "estadoSistema.h"
#include "configSistema.h"
#include "myTaskConfig.h"
#include "medidasNivel.h"

/* Etiqueta para depuración */
static char* TAG = "medidasNivel";

/***********************************************************************************************************
 * Funciones de lectura de la báscula
 ***********************************************************************************************************/

/* Función de petición de medidas a la báscula */
double medida_bascula()
{
    /* Nota: devolver valor de medida de la báscula */
    return 50;
}

/***********************************************************************************************************
 * Funciones de control sobre el timer para el periodo de medidas
 ***********************************************************************************************************/

/* Comprueba si el timer ha vencido o no está activo */
int  timer_expired(int timer)
{
    return (timer == 0);
}

/* Inicia el timer con el tiempo especificado en milisegundos
El periodo de cálculo de consumo se corresponde al periodo de ejecución fijo de la tarea. */
void timer_start(int* timer, int tiempo_max_ms, int periodo_ms)
{
    *timer = (tiempo_max_ms) / periodo_ms;
}

/* Decrementa el timer si aún no ha llegado a 0 */
void timer_next(int* timer)
{
    if (*timer > 0) --timer;
    ESP_LOGD(TAG, "Tiempo restante: %d", *timer);
}

/***********************************************************************************************************
 * Punto de entrada de la tarea de toma de medidas
 ***********************************************************************************************************/

/* Configuración de la tarea de toma de medidas */
void tareaMedidasNivelSet(tareaMedidasNivelInfo_t* pTaskInfo, bufferCircular_t* pMedidas, estadoSistema_t* pEstadoSist, configSistema_t* pConfigSist, paradaEmergencia_t* pEmergencia)
{
    pTaskInfo->pMedidas   = pMedidas;
    pTaskInfo->pEstadoSist = pEstadoSist;
    pTaskInfo->pConfigSist = pConfigSist;
    pTaskInfo->pEmergencia = pEmergencia;
}

/* Punto de entrada de la tarea de toma de medidas */
void tareaMedidasNivel(void* pParametros)
{
    /* Estructuras para intercambio de información */
    /* Entre la tarea y la aplicación principal */
    taskConfig_t*           pConfig  = ((taskInfo_t *)pParametros)->pConfig;
    void*                     pData  = ((taskInfo_t *)pParametros)->pData;
    bufferCircular_t*      pMedidas  = ((tareaMedidasNivelInfo_t*)pData)->pMedidas;
    estadoSistema_t*    pEstadoSist  = ((tareaMedidasNivelInfo_t*)pData)->pEstadoSist;
    configSistema_t*    pConfigSist  = ((tareaMedidasNivelInfo_t*)pData)->pConfigSist;
    paradaEmergencia_t* pEmergencia  = ((tareaMedidasNivelInfo_t*)pData)->pEmergencia;

    ESP_LOGI(pConfig->tag, "Periodo de planificación: %lu ms", pConfig->periodo);
    ESP_LOGI(pConfig->tag, "Número inicial de activaciones: %lu", pConfig->numActivaciones);

    /* Prepara la tarea */
    const TickType_t periodo = (pConfig->periodo) / portTICK_PERIOD_MS;
    TickType_t activacionPrevia = xTaskGetTickCount();

    /* Parámetros de funcionamiento configurables */
    int periodo_medidas = pConfig->periodo;  // periodo de toma de medidas en ms, inicialmente coincide con el periodo de la tarea
    int periodo_medidas_old = periodo_medidas;  // se utiliza para comprobar si ha cambiado el periodo de medidas en esta ejecución
    double periodo_espera_estabilizacion = 5;  // periodo de espera de estabilización en s

    /* Estado del sistema */
    estadoSistemaEspera_t espera_estabilizacion = DESACTIVADA;
    estadoSistemaDeposito_t estado_deposito = DEPOSITO_NORMAL;
    bool paradaEmergencia = false;

    /* Límites de nivel de depósito */
    bool nivelMaximo = false;
    bool nivelMinimo = false;

    /* Bucle de toma de medidas */
    bool continuar = true;
    double medida = 0.0;
    int timer_periodo_medidas = 0;  // cada vez que llega a 0, se toma una medida y se reinicia. Configurable por el usuario
    int timer_espera_estabilizacion = 0;    // mientras esté activo, no se tomarán medidas. Al desactivarse, se modifica el estado de espera

    while( continuar )
    {
        /* Espera a la siguiente activación */  
        xTaskDelayUntil(&activacionPrevia, periodo);

        pConfig->numActivaciones++;
        ESP_LOGD(pConfig->tag, "Numero de activaciones: %lu", pConfig->numActivaciones);


        /* ACTUALIZACIÓN DEL PERIODO CONFIGURABLE DE TOMA DE MEDIDAS */
        /* Comprueba si el periodo de toma de medidas ha cambiado en esta ejecución */
        periodo_medidas_old = periodo_medidas;
        if (!configSistemaLeerPeriodo(pConfigSist, &periodo_medidas)) { continuar = false; }
        /* En caso afirmativo, inicia el nuevo timer de toma de medidas y limpia el buffer de medidas */
        if (periodo_medidas_old != periodo_medidas)
        {
            timer_start(&timer_periodo_medidas, periodo_medidas, pConfig->periodo);
            if (!bufferCircularLimpia(pMedidas)) { continuar = false; }
        }
        /* Actualiza el periodo de espera de estabilización */
        if (!configSistemaLeerEspera(pConfigSist, &periodo_espera_estabilizacion)) { continuar = false; }


        /* COMPROBACIÓN DE NIVEL MÁXIMO Y MÍNIMO DEL DEPÓSITO*/
        /* Comprueba si la parada de emergencia se encuentra activa */
        if (!paradaEmergenciaLeer(pEmergencia, &paradaEmergencia)) { continuar = false; }
        /* Lectura de la medida de la báscula y comprobación de nivel */
        if (!paradaEmergencia)
        {
            medida = medida_bascula();
            ESP_LOGI(pConfig->tag, "Lectura de pesaje: %f", medida);
            /* Comprueba si el valor medido supera el máximo o el mínimo */
            if (!configSistemaComprobarNivel(pConfigSist, medida, &nivelMaximo, &nivelMinimo)) { continuar = false; }
            /* Actualiza el estado del sistema en función del resultado de la comprobación*/
            if (nivelMaximo)
            {
                if (!estadoSistemaEscribirNivel(pEstadoSist, NIVEL_MAXIMO)) { continuar = false; }
            }
            if (nivelMinimo)
            {
                if (!estadoSistemaEscribirNivel(pEstadoSist, NIVEL_MINIMO)) { continuar = false; }
            }
            if (!nivelMaximo + !nivelMinimo)
            {
                if (!estadoSistemaEscribirNivel(pEstadoSist, NIVEL_NORMAL)) { continuar = false; }
            }
        }


        /* INICIALIZACIÓN DEL TIMER DE ESPERA DE ESTABILIZACIÓN */
        /* Si la espera de estabilización está activa y acaba de expirar, se desactiva */
        estadoSistemaLeerEspera(pEstadoSist, &espera_estabilizacion);
        if ((espera_estabilizacion == EN_CURSO) & timer_expired(timer_espera_estabilizacion))
        {
            espera_estabilizacion = DESACTIVADA;
            estadoSistemaEscribirEspera(pEstadoSist, espera_estabilizacion);
        }
        /* Si el sistema de control del depósito acaba de iniciar la espera de estabilización, se inicia su timer*/
        if (espera_estabilizacion == INICIADA)
        {
            timer_start(&timer_espera_estabilizacion, (int) periodo_espera_estabilizacion*1000, pConfig->periodo);
            espera_estabilizacion = DESACTIVADA;
            estadoSistemaEscribirEspera(pEstadoSist, EN_CURSO);
        }


        /* ENVÍO DE MEDIDAS AL BUFFER */
        /* Comprueba el estado del depósito */
        if ( !estadoSistemaLeerDeposito(pEstadoSist, &estado_deposito )) { continuar = false; }
        /* Si se cumplen todas las condiciones y el periodo de toma de medidas ha finalizado, se envía la medida */
        if (!paradaEmergencia && timer_expired(timer_periodo_medidas) && (estado_deposito == DEPOSITO_NORMAL))
        {
            if (!bufferCircularMete(pMedidas, medida)) { continuar = false; }
        }
        /* Si no se cumplen, se limpia el buffer para eliminar medidas desactualizadas */
        if (paradaEmergencia + (estado_deposito != DEPOSITO_NORMAL))
        {
            if (!bufferCircularLimpia(pMedidas)) { continuar = false; }
        }


        /* ACTUALIZACIÓN DEL TIMER DE ESPERA DE ESTABILIZACIÓN Y DE PERIODO DE TOMA DE MEDIDAS*/
        /* Actualiza el timer de periodo de medidas*/
        timer_next(&timer_periodo_medidas);
        /* Actualiza el timer de espera de estabilización */
        timer_next(&timer_espera_estabilizacion);
    }
}