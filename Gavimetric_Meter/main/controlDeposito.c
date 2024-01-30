/***********************************************************************************************************
 * MÓDULO DE CONTROL DEL DEPÓSITO
 * Objeto cíclico: Control del depósito
 * Máquina de estados que se actualiza periódicamente, controla el estado de las electroválvulas y la bomba
 ***********************************************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LOG_LOCAL_LEVEL ESP_LOG_NONE
#include "esp_log.h"
//#include "esp_timer.h"

#include "myTaskConfig.h"
#include "estadoSistema.h"
#include "paradaEmergencia.h"
#include "fsm.h"
#include "controlDeposito.h"

/* Etiqueta para depuración */
static char* TAG = "controlDeposito";

/***********************************************************************************************************
 * Implementación de hardware: Funciones de control del depósito
 ***********************************************************************************************************/

/* Activación de la bomba eléctrica */
void bomba_on (void)
{
    ESP_LOGD(TAG, "Activa la bomba eléctrica");
}

/* Desactivación de la bomba eléctrica */
void bomba_off (void)
{
    ESP_LOGD(TAG, "Desactiva la bomba eléctrica");
}

/* Abre la electroválvula de llenado */
void valv_llen_open (void)
{
    ESP_LOGD(TAG, "Abre la electroválvula de llenado");
}

/* Cierra la electroválvula de llenado */
void valv_llen_close (void)
{
    ESP_LOGD(TAG, "Cierra la electroválvula de llenado");
}

/* Abre la electroválvula de vaciado */
void valv_vac_open (void)
{
    ESP_LOGD(TAG, "Abre la electroválvula de vaciado");
}

/* Cierra la electroválvula de vaciado */
void valv_vac_close (void)
{
    ESP_LOGD(TAG, "Cierra la electroválvula de vaciado");
}

/***********************************************************************************************************
 * Funciones de condición de transferencia
 ***********************************************************************************************************/

/* Parada de emergencia inactiva Y (Comando de llenado O Detección de nivel mínimo)*/
int deposito_c12 (void *params)
{
    tareaControlDepositoParams_t *pParams = (tareaControlDepositoParams_t *)params;

    bool emergencia = 0;
    bool llenado = 0;
    bool nivel_min = 0;

    emergencia = pParams->emergencia;
    if (pParams->estadoDeposito == DEPOSITO_LLENADO) { llenado = 1; }
    if (pParams->nivelDeposito == NIVEL_MINIMO) { nivel_min = 1; }
    
    return !emergencia && (llenado + nivel_min);
}

/* Parada de emergencia inactiva Y Comando de vaciado */
int deposito_c13 (void *params)
{
    tareaControlDepositoParams_t *pParams = (tareaControlDepositoParams_t *)params;

    bool emergencia = 0;
    bool vaciado = 0;

    emergencia = pParams->emergencia;
    if (pParams->estadoDeposito == DEPOSITO_VACIADO) { vaciado = 1; }
    
    return !emergencia && vaciado;
}

/* Parada de emergencia activa O Comando de detención de llenado O Detección de nivel máximo */
int deposito_c21 (void *params)
{
    tareaControlDepositoParams_t *pParams = (tareaControlDepositoParams_t *)params;

    bool emergencia = 0;
    bool llenado_stop = 0;
    bool nivel_max = 0;

    emergencia = pParams->emergencia;
    if (pParams->estadoDeposito == DEPOSITO_NORMAL) { llenado_stop = 1; }
    if (pParams->nivelDeposito == NIVEL_MAXIMO) { nivel_max = 1; }
    ESP_LOGI("controlDeposito: ", "Comprobada la tercera condición de estado");
    return emergencia + llenado_stop + nivel_max;
}

/* Comando de vaciado */
int deposito_c23 (void *params)
{
    tareaControlDepositoParams_t *pParams = (tareaControlDepositoParams_t *)params;

    bool vaciado = 0;

    if (pParams->estadoDeposito == DEPOSITO_VACIADO) { vaciado = 1; }

    return vaciado;
}

/* Parada de emergencia activa O Comando de detención de vaciado */
int deposito_c31 (void *params)
{
    tareaControlDepositoParams_t *pParams = (tareaControlDepositoParams_t *)params;

    bool emergencia = 0;
    bool vaciado_stop = 0;

    emergencia = pParams->emergencia;
    if (pParams->estadoDeposito == DEPOSITO_NORMAL) { vaciado_stop = 1; }

    return emergencia + vaciado_stop;
}

/* Comando de llenado */
int deposito_c32 (void *params)
{
    tareaControlDepositoParams_t *pParams = (tareaControlDepositoParams_t *)params;

    bool llenado = 0;

    if (pParams->estadoDeposito == DEPOSITO_LLENADO) { llenado = 1; }
    
    return llenado;
}

/***********************************************************************************************************
 * Funciones de transferencia de estados
 ***********************************************************************************************************/

void deposito_a12 (void* params)
{
    tareaControlDepositoParams_t *pParams = (tareaControlDepositoParams_t *)params;
    estadoSistema_t* pEstadoSist = pParams->estadoSist;

    bomba_on();
    valv_llen_open();

    if ( !estadoSistemaEscribirDeposito(pEstadoSist, DEPOSITO_LLENADO) ) { *(pParams->continuar) = false; }
}

void deposito_a13 (void* params)
{
    tareaControlDepositoParams_t *pParams = (tareaControlDepositoParams_t *)params;
    estadoSistema_t* pEstadoSist = pParams->estadoSist;

    valv_vac_open();

    if ( !estadoSistemaEscribirDeposito(pEstadoSist, DEPOSITO_VACIADO) ) { *(pParams->continuar) = false; }
}

void deposito_a23 (void* params)
{
    tareaControlDepositoParams_t *pParams = (tareaControlDepositoParams_t *)params;
    estadoSistema_t* pEstadoSist = pParams->estadoSist;

    bomba_off();
    valv_llen_close();
    valv_vac_open();

    if ( !estadoSistemaEscribirDeposito(pEstadoSist, DEPOSITO_VACIADO) ) { *(pParams->continuar) = false; }
    /* Se inicia la espera de estabilización */
    if ( !estadoSistemaEscribirEspera(pEstadoSist, INICIADA) ) { *(pParams->continuar) = false; }
}

void deposito_a32 (void* params)
{
    tareaControlDepositoParams_t *pParams = (tareaControlDepositoParams_t *)params;
    estadoSistema_t* pEstadoSist = pParams->estadoSist;

    valv_vac_close();
    valv_llen_open();
    bomba_on();

    if ( !estadoSistemaEscribirDeposito(pEstadoSist, DEPOSITO_LLENADO) ) { *(pParams->continuar) = false; }
}

void deposito_a21 (void* params)
{
    tareaControlDepositoParams_t *pParams = (tareaControlDepositoParams_t *)params;
    estadoSistema_t* pEstadoSist = pParams->estadoSist;

    bomba_off();
    valv_llen_close();
    
    if ( !estadoSistemaEscribirDeposito(pEstadoSist, DEPOSITO_NORMAL) ) { *(pParams->continuar) = false; }
    /* Se inicia la espera de estabilización */
    if ( !estadoSistemaEscribirEspera(pEstadoSist, INICIADA) ) { *(pParams->continuar) = false; }
}

void deposito_a31 (void* params)
{
    tareaControlDepositoParams_t *pParams = (tareaControlDepositoParams_t *)params;
    estadoSistema_t* pEstadoSist = pParams->estadoSist;

    valv_vac_close();

    if ( !estadoSistemaEscribirDeposito(pEstadoSist, DEPOSITO_NORMAL) ) { *(pParams->continuar) = false; }
}

/***********************************************************************************************************
 * Inicialización de la máquina de estados
 * Estado 1: Funcionamiento normal
 * Estado 2: Llenado
 * Estado 3: Vaciado
 ***********************************************************************************************************/

fsm_t* controlDeposito_new (void)
{
    static fsm_trans_t controlDeposito_tt[] = {
        {  1, deposito_c12, 2, deposito_a12 },
        {  1, deposito_c13, 3, deposito_a13 },
        {  2, deposito_c21, 1, deposito_a21 },
        {  2, deposito_c23, 3, deposito_a23 },
        {  3, deposito_c31, 1, deposito_a31 },
        {  3, deposito_c32, 2, deposito_a32 },
        { -1, NULL, -1, NULL },
    };
    return fsm_new (controlDeposito_tt);
}

/***********************************************************************************************************
 * Punto de entrada de la tarea de control del depósito
 ***********************************************************************************************************/

/* Configuración de la tarea de control del depósito */
void tareaControlDepositoSet(tareaControlDepositoInfo_t* pTaskInfo, estadoSistema_t* pEstadoSist, paradaEmergencia_t* pEmergencia)
{
    pTaskInfo->pEstadoSist   = pEstadoSist;
    pTaskInfo->pEmergencia   = pEmergencia;
}

void tareaControlDeposito(void* pParametros)
{
    /* Estructuras para intercambio de información */
    /* Entre la tarea y la aplicación principal */
    taskConfig_t* pConfig           = ((taskInfo_t *)pParametros)->pConfig;
    void*           pData           = ((taskInfo_t *)pParametros)->pData;
    estadoSistema_t* pEstadoSist    = ((tareaControlDepositoInfo_t *)pData)->pEstadoSist;
    paradaEmergencia_t* pEmergencia = ((tareaControlDepositoInfo_t *)pData)->pEmergencia;

    ESP_LOGI(pConfig->tag, "Periodo de planificación: %lu ms", pConfig->periodo);
    ESP_LOGI(pConfig->tag, "Número inicial de activaciones: %lu", pConfig->numActivaciones);

    /* Prepara la tarea */
    const TickType_t periodo = (pConfig->periodo) / portTICK_PERIOD_MS;
    TickType_t activacionPrevia = xTaskGetTickCount();

    /* Configura la máquina de estados */
    ESP_LOGI(TAG, "Configura el control del depósito");
    fsm_t* fsm_controlDeposito = controlDeposito_new();

    /* Inicializa el estado 1 del depósito */
    bomba_off();
    valv_llen_close();
    valv_vac_close();

    /* Bucle de presentación de medias */
    bool continuar = true;
    
    /* Prepara los parámetros de entrada para las funciones de condición de transferencia */
    tareaControlDepositoParams_t pControlDepositoParams;
    pControlDepositoParams.emergencia = false;
    pControlDepositoParams.nivelDeposito = NIVEL_NORMAL;
    pControlDepositoParams.estadoDeposito = DEPOSITO_NORMAL;
    pControlDepositoParams.estadoSist = pEstadoSist;
    pControlDepositoParams.continuar = &continuar;

    /* DEBUG: TIEMPO DE EJECUCIÓN */
    //uint64_t startTime;
    //uint64_t endTime;
    //uint64_t executionTime;

    while( continuar )
    {
        /* Espera a la siguiente activación */  
        xTaskDelayUntil(&activacionPrevia, periodo);
        pConfig->numActivaciones++;
        ESP_LOGD(pConfig->tag, "Numero de activaciones: %lu", pConfig->numActivaciones);

        /* DEBUG: TIEMPO DE EJECUCIÓN */
        //startTime = esp_timer_get_time();

        /* Comprueba el estado actual del sistema */
        if ( !paradaEmergenciaLeer(pEmergencia, &pControlDepositoParams.emergencia) ) { continuar = false; }
        if ( !estadoSistemaLeerNivel(pEstadoSist, &pControlDepositoParams.nivelDeposito) ) { continuar = false; }
        if ( !estadoSistemaLeerDeposito(pEstadoSist, &pControlDepositoParams.estadoDeposito) ) { continuar = false; }

        /* Actualiza la máquina de estados según el estado leído */
        fsm_update (fsm_controlDeposito, (void*) &pControlDepositoParams);

        /* DEBUG: TIEMPO DE EJECUCIÓN */
        //endTime = esp_timer_get_time();
        //executionTime = endTime - startTime;
        //ESP_LOGD(pConfig->tag, "Duración de tarea: %lld microsegundos\n", executionTime);
    }
}