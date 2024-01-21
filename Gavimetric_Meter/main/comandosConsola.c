/***********************************************************************************************************
 * MÓDULO DE CONSOLA
 * Objeto cíclico: envío de comandos de la consola
 * Comprueba periódicamente si el usuario ha introducido un comando en la consola.
 * En caso afirmativo, realiza la operación correspondiente sobre el sistema.
 ***********************************************************************************************************/

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

#include "myTaskConfig.h"
#include "estadoSistema.h"
#include "configSistema.h"
#include "paradaEmergencia.h"
#include "comandosConsola.h"

/* Etiqueta para depuración */
const char* TAG = "comandosConsola";

/***********************************************************************************************************
 * Funciones de lectura de comandos
 ***********************************************************************************************************/

/* Lee el último comando recibido por la consola, y el valor asociado si se trata de configurar un parámetro del sistema */
void lectura_comando( comandosConsola_t* comando, double* valor)
{
    /* Nota: devolver comando y valor asociado si lo hubiera */
    /* Si no hay comando, devuelve -1 */
    comando = -1;
    valor = 100;
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
    estadoSistemaDeposito_t estadoDeposito = NORMAL;

    /* Lectura de comandos */
    comandosConsola_t comando = -1;   // comando recibido desde la consola
    double valor = 0;   // valor asociado a los comandos de configuración de parámetros

    /* Bucle de recepción de comandos */
    bool continuar = true;
    bool modo_manual = false;   // estado interno: establece comunicación con la consola de control (manual) o con el sistema remoto (automático)

    while( continuar )
    {
        /* Espera a la siguiente activación */  
        xTaskDelayUntil(&activacionPrevia, periodo);

        pConfig->numActivaciones++;
        ESP_LOGD(pConfig->tag, "Numero de activaciones: %lu", pConfig->numActivaciones);

        /* Comprueba si la parada de emergencia se encuentra activa */
        paradaEmergenciaLeer(pEmergencia, &emergencia);

        /* En caso afirmativo, si el sistema aún no se encuentra en modo manual, conmuta */
        if (emergencia & !modo_manual) { modo_manual = true; }

        /* Lee el último comando recibido desde la consola, y su valor asociado, si los hubiera */
        lectura_comando(&comando, &valor);

        /* En función del comando recibido, ejecuta una tarea u otra */
        switch (comando)
        {
        case CONMUTAR_ESTADO:
            modo_manual = !modo_manual;
            break;

        case DETENER_PARADA_EMERGENCIA:
            paradaEmergenciaDesactivar(pEmergencia);
            break;

        case INICIAR_MEDIDA_CONTINUADA:
            if (modo_manual & !emergencia) 
            { 
                if ( !estadoSistemaEscribirComando(pEstadoSist, MEDIDA_MANUAL_CONTINUADA) ) { continuar = false; }
            }
            break;

        case DETENER_MEDIDA_CONTINUADA:
            if (modo_manual & !emergencia)
            {
                if ( !estadoSistemaEscribirComando(pEstadoSist, DESACTIVADA) ) { continuar = false; }
            }
            break;

        case INICIAR_LLENADO:
            if (modo_manual & !emergencia)
            {
                if ( !estadoSistemaEscribirDeposito(pEstadoSist, LLENADO) ) { continuar = false; }
            }
            break;

        case DETENER_LLENADO:
            /* Lee el estado actual del depósito, y solo actúa si es "LLENADO" */
            if ( !estadoSistemaLeerDeposito(pEstadoSist, &estadoDeposito) ) { continuar = false; }
            if (modo_manual & !emergencia & estadoDeposito == LLENADO)
            { 
                if ( !estadoSistemaEscribirDeposito(pEstadoSist, NORMAL) ) { continuar = false; }
            }
            break;

        case INICIAR_VACIADO:
            if (modo_manual & !emergencia)
            {
                if ( !estadoSistemaEscribirDeposito(pEstadoSist, VACIADO) ) { continuar = false; }
            }
            break;

        case DETENER_VACIADO:
            /* Lee el estado actual del depósito, y solo actúa si es "VACIADO" */
            if ( !estadoSistemaLeerDeposito(pEstadoSist, &estadoDeposito) ) { continuar = false; }
            if (modo_manual & !emergencia & estadoDeposito == VACIADO)
            { 
                if ( !estadoSistemaEscribirDeposito(pEstadoSist, NORMAL) ) { continuar = false; }
            }
            break;

        case CONFIG_PERIODO_MEDIDA:
            if (modo_manual)
            {
                if ( !configSistemaEscribirPeriodo(pConfigSist, (int) valor) ) { continuar = false; }
            }

        case CONFIG_TIEMPO_ESTAB:
            if (modo_manual)
            {
                if ( !configSistemaEscribirEspera(pConfigSist, valor) ) { continuar = false; }
            }

        case CONFIG_CONSUMO_MAXIMO:
            if (modo_manual)
            {
                if ( !configSistemaEscribirConsumoMax(pConfigSist, valor) ) { continuar = false; }
            }

        case CONFIG_NIVEL_MINIMO:
            if (modo_manual)
            {
                if ( !configSistemaEscribirConsumoMin(pConfigSist, valor) ) { continuar = false; }
            }

        case CONFIG_NIVEL_MAXIMO:
            if (modo_manual)
            {
                if ( !configSistemaEscribirConsumoMax(pConfigSist, valor) ) { continuar = false; }
            }

        default:
            break;
        }
    }
}