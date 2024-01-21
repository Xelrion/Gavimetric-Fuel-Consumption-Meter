/***********************************************************************************************************
 * MÓDULO DE MONITORIZACIÓN DEL DEPÓSITO
 * Objeto cíclico: Cálculo de medidas de consumo
 * Lee el buffer de medidas y, si se cumplen las condiciones necesarias, calcula el consumo medio por cada pareja
 * Envía las medidas de consumo al buffer de la consola o al buffer del sistema remoto, según el modo de funcionamiento
 ***********************************************************************************************************/

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

#include "myTaskConfig.h"
#include "bufferCircular.h"
#include "estadoSistema.h"
#include "configSistema.h"
#include "calculaConsumo.h"

/* Etiqueta para depuración */
const char* TAG = "calculaConsumo";

/***********************************************************************************************************
 * Funciones de cálculo de medidas
 ***********************************************************************************************************/

/* Comprueba si hay suficientes medidas en el buffer para calcular el consumo */
bool medidasDisponibles( bufferCircular_t* pMedidas )
{
    int numMedidas;
    int minMedidas = 2;

    bufferCircularNumElementos(pMedidas, &numMedidas);
    return (numMedidas >= 2);
}

/* Calcula el consumo del depósito por segundo */
double consumoSegundo( double medida1, double medida2, int periodo_ms )
{
    return (medida1 + medida2)/(periodo_ms*1000);
}

/* Calcula el consumo del depósito por minuto */
double consumoMinuto( double medida1, double medida2, int periodo_ms )
{
    return (medida1 + medida2)/(periodo_ms*1000*60);
}

/* Calcula el consumo del depósito por hora */
double consumoHora( double medida1, double medida2, int periodo_ms )
{
    return (medida1 + medida2)/(periodo_ms*1000*3600);
}

/* Calcula una medida de consumo y la introduce en el buffer especificado */
void calculoConsumo( bufferCircular_t* pSacaMedidas, bufferCircular_t* pEnviaConsumo, int periodo_medidas, bool* continuar)
{
    double medida1, medida2, consumo;

    /* Extrae las dos medidas más antiguas almacenadas en el buffer */
    bufferCircularSaca(pSacaMedidas, &medida1);
    bufferCircularSaca(pSacaMedidas, &medida2);

    /* Calcula el consumo medio entre ellas */
    consumo = consumoHora(medida1, medida2, periodo_medidas);

    /* Introduce el consumo calculado en el buffer de envío */
    if (!bufferCircularMete(pEnviaConsumo, consumo)) { continuar = false; }
}

/***********************************************************************************************************
 * Punto de entrada de la tarea de cálculo de consumo
 ***********************************************************************************************************/

/* Configuración de la tarea de cálculo de medias */
void tareaConsumoSet(tareaConsumoInfo_t* pTaskInfo, bufferCircular_t* pMedidas, bufferCircular_t* pConsumoRemoto, bufferCircular_t* pConsumoConsola, estadoSistema_t* pEstadoSist, configSistema_t* pConfigSist)
{
    pTaskInfo->pMedidas = pMedidas;
    pTaskInfo->pConsumoRemoto = pConsumoRemoto;
    pTaskInfo->pConsumoConsola = pConsumoConsola;
    pTaskInfo->pEstadoSist = pEstadoSist;
    pTaskInfo->pConfigSist = pConfigSist;
}

/* Punto de entrada de la tarea de cálculo de consumo */
void tareaConsumo(void* pParametros)
{
    /* Estructuras para intercambio de información */
    /* Entre la tarea y la aplicación principal */
    taskConfig_t*             pConfig = ((taskInfo_t *)pParametros)->pConfig;
    void*                       pData = ((taskInfo_t *)pParametros)->pData;
    bufferCircular_t*        pMedidas = ((tareaConsumoInfo_t*)pData)->pMedidas;
    bufferCircular_t*  pConsumoRemoto = ((tareaConsumoInfo_t*)pData)->pConsumoRemoto;
    bufferCircular_t* pConsumoConsola = ((tareaConsumoInfo_t*)pData)->pConsumoConsola;
    estadoSistema_t*      pEstadoSist = ((tareaConsumoInfo_t*)pData)->pEstadoSist;
    configSistema_t*      pConfigSist = ((tareaConsumoInfo_t*)pData)->pConfigSist;

    ESP_LOGD(pConfig->tag, "Periodo de planificación: %lu ms", pConfig->periodo);
    ESP_LOGD(pConfig->tag, "Número inicial de activaciones: %lu", pConfig->numActivaciones);
    // ESP_LOGI(pConfig->tag, "LED en pin %u. Nivel inicial: %s", pLED->pin, (pLED->nivel==0)? "LOW":"HIGH");
    // ESP_LOGI(pConfig->tag, "Mutex en %x", (unsigned int)(pLED->sem));

    /* Prepara la tarea */
    const TickType_t periodo = (pConfig->periodo) / portTICK_PERIOD_MS;
    TickType_t activacionPrevia = xTaskGetTickCount();

    /* Parámetros de funcionamiento configurables */
    int periodo_medidas = 500;  // periodo de toma de medidas en ms
    int periodo_medidas_old = periodo_medidas; // se utiliza para comprobar si ha cambiado el periodo de medidas en esta ejecución

    /* Estado del sistema */
    estadoSistemaComando_t comando = DESACTIVADA;

    /* Bucle de cálculo de consumo */
    bool continuar = true;
    bool periodo_medidas_modif = false; // si en esta ejecución se ha modificado el periodo de medidas, no se calcula el consumo para evitar datos erróneos

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
        /* En caso afirmativo, no se calculará el consumo en esta ejecución para evitar utilizar datos erróneos del buffer */
        /* Además, limpiará las medidas de consumo que aún pueda haber en el buffer de la consola o del sistema remoto */
        if (periodo_medidas_old != periodo_medidas)
        {
            periodo_medidas_modif = true;
            if( !bufferCircularLimpia(pConsumoConsola) ) { continuar = false; }
            if( !bufferCircularLimpia(pConsumoRemoto) ) { continuar = false; }
        }
        else { periodo_medidas_modif = false; }

        /* CÁLCULO DE MEDIDAS DE CONSUMO */
        /* Comprueba el comando actual de petición de medidas */
        if (!estadoSistemaLeerComando(pEstadoSist, &comando)) { continuar = false; }
        /* Lee y calcula el consumo entre dos medidas, siempre que haya suficientes en el buffer */
        /* Primer bucle: envía medidas al buffer de la consola (modo manual) */
        while (medidasDisponibles(pMedidas) & (comando == MEDIDA_MANUAL_PUNTUAL || comando == MEDIDA_MANUAL_CONTINUADA || comando == MEDIDA_OFF))
        {
            calculoConsumo( pMedidas, pConsumoConsola, periodo_medidas, &continuar);
        }
        /* Segundo bucle: envía medidas al buffer del sistema remoto (modo remoto) */
        while (medidasDisponibles(pMedidas) && comando == MEDIDA_REMOTO)
        {
            calculoConsumo( pMedidas, pConsumoRemoto, periodo_medidas, &continuar);
        }
    }
}
