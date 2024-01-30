/***********************************************************************************************************
 * MÓDULO DE CONSOLA
 * Objeto cíclico: envío de comandos de la consola
 * Comprueba periódicamente si el usuario ha introducido un comando en la consola.
 * En caso afirmativo, realiza la operación correspondiente sobre el sistema.
 ***********************************************************************************************************/

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LOG_LOCAL_LEVEL ESP_LOG_NONE
#include "esp_log.h"
//#include "esp_timer.h"

#include "myTaskConfig.h"
#include "estadoSistema.h"
#include "configSistema.h"
#include "paradaEmergencia.h"
#include "comandosConsola.h"

/* Etiqueta para depuración */
static char* TAG = "comandosConsola";

/***********************************************************************************************************
 * Implementación de hardware: Funciones de lectura de comandos
 ***********************************************************************************************************/

/* Lee el último comando recibido por la consola, y el valor asociado si se trata de configurar un parámetro del sistema */
void lectura_comando_valor( comandosConsola_t* comando, double* valor)
{
    /* Devuelve el último comando recibido y su valor asociado si lo hubiera */
    /* Si no hay comando, devuelve -1 */
    *comando = -1;
    *valor = 100;
}

/***********************************************************************************************************
 * Punto de entrada de la tarea de lectura de comandos
 ***********************************************************************************************************/

/* Configuración de la tarea de lectura de comandos */
void tareaComandosConsolaSet(tareaComandosConsolaInfo_t* pTaskInfo, estadoSistema_t* pEstadoSist, configSistema_t* pConfigSist, paradaEmergencia_t* pEmergencia)
{
    pTaskInfo->pEstadoSist = pEstadoSist;
    pTaskInfo->pConfigSist = pConfigSist;
    pTaskInfo->pEmergencia = pEmergencia;
}

/* Punto de entrada de la tarea de lectura de comandos */
void tareaComandosConsola(void* pParametros)
{
    /* Estructuras para intercambio de información */
    /* Entre la tarea y la aplicación principal */
    taskConfig_t*           pConfig  = ((taskInfo_t *)pParametros)->pConfig;
    void*                     pData  = ((taskInfo_t *)pParametros)->pData;
    estadoSistema_t*    pEstadoSist  = ((tareaComandosConsolaInfo_t*)pData)->pEstadoSist;
    configSistema_t*    pConfigSist  = ((tareaComandosConsolaInfo_t*)pData)->pConfigSist;
    paradaEmergencia_t* pEmergencia  = ((tareaComandosConsolaInfo_t*)pData)->pEmergencia;

    ESP_LOGI(pConfig->tag, "Periodo de planificación: %lu ms", pConfig->periodo);
    ESP_LOGI(pConfig->tag, "Número inicial de activaciones: %lu", pConfig->numActivaciones);

    /* Prepara la tarea */
    const TickType_t periodo = (pConfig->periodo) / portTICK_PERIOD_MS;
    TickType_t activacionPrevia = xTaskGetTickCount();

    /* Estado de emergencia */
    bool emergencia = false;

    /* Estado del depósito */
    estadoSistemaDeposito_t estadoDeposito = DEPOSITO_NORMAL;

    /* Lectura de comandos */
    comandosConsola_t comando = -1;   // comando recibido desde la consola
    double valor = 0;   // valor asociado a los comandos de configuración de parámetros

    /* Bucle de recepción de comandos */
    bool continuar = true;
    bool modo_manual = false;   // estado interno: establece comunicación con la consola de control (manual) o con el sistema remoto (automático)

    /* DEBUG: TIEMPO DE EJECUCIÓN */
    //uint64_t startTime;
    //uint64_t endTime;
    //uint64_t executionTime;

    while( continuar )
    {
        /* Espera a la siguiente activación */  
        xTaskDelayUntil(&activacionPrevia, periodo);

        pConfig->numActivaciones++;
        //ESP_LOGD(pConfig->tag, "Numero de activaciones: %lu", pConfig->numActivaciones);

        /* DEBUG: TIEMPO DE EJECUCIÓN */
        //startTime = esp_timer_get_time();

        /* Comprueba si la parada de emergencia se encuentra activa */
        paradaEmergenciaLeer(pEmergencia, &emergencia);

        /* En caso afirmativo, si el sistema aún no se encuentra en modo manual, conmuta */
        if (emergencia && !modo_manual) { modo_manual = true; }

        /* Lee el último comando recibido desde la consola, y su valor asociado, si los hubiera */
        lectura_comando_valor(&comando, &valor);

        /* En función del comando recibido, ejecuta una tarea u otra */
        switch (comando)
        {
        case CONMUTAR_ESTADO:
            modo_manual = !modo_manual;
            break;

        case DETENER_PARADA_EMERGENCIA:
            if (modo_manual && emergencia) { paradaEmergenciaDesactivar(pEmergencia); }
            break;

        case PEDIR_MEDIDA_PUNTUAL:
            if (modo_manual && !emergencia) 
            { 
                if ( !estadoSistemaEscribirComando(pEstadoSist, MEDIDA_MANUAL_PUNTUAL) ) { continuar = false; }
            }
            break;

        case INICIAR_MEDIDA_CONTINUADA:
            if (modo_manual && !emergencia) 
            { 
                if ( !estadoSistemaEscribirComando(pEstadoSist, MEDIDA_MANUAL_CONTINUADA) ) { continuar = false; }
            }
            break;

        case DETENER_MEDIDA_CONTINUADA:
            if (modo_manual && !emergencia)
            {
                if ( !estadoSistemaEscribirComando(pEstadoSist, DESACTIVADA) ) { continuar = false; }
            }
            break;

        case INICIAR_LLENADO:
            if (modo_manual && !emergencia)
            {
                if ( !estadoSistemaEscribirDeposito(pEstadoSist, DEPOSITO_LLENADO) ) { continuar = false; }
            }
            break;

        case DETENER_LLENADO:
            /* Lee el estado actual del depósito, y solo actúa si es "DEPOSITO_LLENADO" */
            if ( !estadoSistemaLeerDeposito(pEstadoSist, &estadoDeposito) ) { continuar = false; }
            if (modo_manual && !emergencia && estadoDeposito == DEPOSITO_LLENADO)
            { 
                if ( !estadoSistemaEscribirDeposito(pEstadoSist, DEPOSITO_NORMAL) ) { continuar = false; }
            }
            break;

        case INICIAR_VACIADO:
            if (modo_manual && !emergencia)
            {
                if ( !estadoSistemaEscribirDeposito(pEstadoSist, DEPOSITO_VACIADO) ) { continuar = false; }
            }
            break;

        case DETENER_VACIADO:
            /* Lee el estado actual del depósito, y solo actúa si es "DEPOSITO_VACIADO" */
            if ( !estadoSistemaLeerDeposito(pEstadoSist, &estadoDeposito) ) { continuar = false; }
            if (modo_manual && !emergencia && estadoDeposito == DEPOSITO_VACIADO)
            { 
                if ( !estadoSistemaEscribirDeposito(pEstadoSist, DEPOSITO_NORMAL) ) { continuar = false; }
            }
            break;

        case CONFIG_PERIODO_MEDIDA:
            if (modo_manual)
            {
                if ( !configSistemaEscribirPeriodo(pConfigSist, (int) valor) ) { continuar = false; }
            }
            break;

        case CONFIG_TIEMPO_ESTAB:
            if (modo_manual)
            {
                if ( !configSistemaEscribirEspera(pConfigSist, valor) ) { continuar = false; }
            }
            break;

        case CONFIG_CONSUMO_MAXIMO:
            if (modo_manual)
            {
                if ( !configSistemaEscribirConsumoMax(pConfigSist, valor) ) { continuar = false; }
            }
            break;

        case CONFIG_NIVEL_MINIMO:
            if (modo_manual)
            {
                if ( !configSistemaEscribirNivelMin(pConfigSist, valor) ) { continuar = false; }
            }
            break;

        case CONFIG_NIVEL_MAXIMO:
            if (modo_manual)
            {
                if ( !configSistemaEscribirNivelMax(pConfigSist, valor) ) { continuar = false; }
            }
            break;

        default:
            break;
        }

        /* DEBUG: TIEMPO DE EJECUCIÓN */
        //endTime = esp_timer_get_time();
        //executionTime = endTime - startTime;
        //ESP_LOGD(pConfig->tag, "Duración de tarea: %lld microsegundos\n", executionTime);
    }
}