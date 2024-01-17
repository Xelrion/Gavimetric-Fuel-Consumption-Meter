/***********************************************************************************************************
 * MÓDULO DE CONTROL DEL DEPÓSITO
 * Objeto cíclico: Control del depósito
 * Máquina de estados que se actualiza periódicamente, controla el estado de las electroválvulas y la bomba
 ***********************************************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

#include "myTaskConfig.h"
#include "fsm.h"
#include "controlDeposito.h"

/* Etiqueta para depuración */
const char* TAG = "controlDeposito";

// Nota: crear otro objeto pasivo para implementar el control directo sobre el depósito, que solo espere a recibir instrucciones directas de este módulo

/***********************************************************************************************************
 * Funciones de control del depósito
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
bool deposito_c12 (void *params)
{
    bool emergencia = 0;/* Nota: comprobación estado de emergencia */
    bool llenado = 0;/* Nota: comprobación comando de llenado */
    bool nivel_min = 0;/* Nota: comprobación nivel mínimo */
    return !emergencia * (llenado + nivel_min);
}

/* Parada de emergencia inactiva Y Comando de vaciado */
bool deposito_c13 (void *params)
{
    bool emergencia = 0;/* Nota: comprobación estado de emergencia */
    bool vaciado = 0;/* Nota: comprobación comando de vaciado */
    return !emergencia * vaciado;
}

/* Parada de emergencia activa O Comando de detención de llenado O Detección de nivel máximo */
bool deposito_c21 (void *params)
{
    bool emergencia = 0;/* Nota: comprobación estado de emergencia */
    bool llenado_stop = 0;/* Nota: comprobación comando de detención de llenado */
    bool nivel_max = 0;/* Nota: comprobación nivel máximo */
    return emergencia + llenado_stop + nivel_max;
}

/* Comando de vaciado */
bool deposito_c23 (void *params)
{
    bool vaciado = 0;/* Nota: comprobación comando de vaciado */
    return vaciado;
}

/* Parada de emergencia activa O Comando de detención de vaciado */
bool deposito_c31 (void *params)
{
    bool emergencia = 0;/* Nota: comprobación estado de emergencia */
    bool vaciado_stop = 0;/* Nota: comprobación comando de detención de vaciado */
    return emergencia + vaciado_stop;
}

/* Comando de llenado */
bool deposito_c32 (void *params)
{
    bool llenado = 0;/* Nota: comprobación comando de llenado */
    return llenado;
}

/***********************************************************************************************************
 * Funciones de transferencia de estados
 ***********************************************************************************************************/

void deposito_a12 (void* params)
{
    bomba_on();
    valv_llen_open();
}

void deposito_a13 (void* params)
{
    valv_vac_open();
}

void deposito_a23 (void* params)
{
    bomba_off();
    valv_llen_close();
    valv_vac_open();
}

void deposito_a32 (void* params)
{
    valv_vac_close();
    valv_llen_open();
    bomba_on();
}

void deposito_a21 (void* params)
{
    bomba_off();
    valv_llen_close();
    /* Nota: inicio contador estabilización */
}

void deposito_a31 (void* params)
{
    valv_vac_close();
}

/***********************************************************************************************************
 * Inicialización de la máquina de estados
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
void tareaControlDepositoSet(tareaControlDepositoInfo_t* pTaskInfo, bufferCircular_t* pMedias)
{
    pTaskInfo->pMedias   = pMedias;
}

void tareaControlDeposito(void* pParametros)
{
    /* Estructuras para intercambio de información */
    /* Entre la tarea y la aplicación principal */
    taskConfig_t* pConfig = ((taskInfo_t *)pParametros)->pConfig;
    void*           pData = ((taskInfo_t *)pParametros)->pData;
    // bufferCircular_t*  pMedias   = ((tareaInformeInfo_t*)pData)->pMedias; Nota: añadir referencia a otros módulos y estructuras necesarios para la tarea

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

    while( continuar )
    {
        /* Espera a la siguiente activación */  
        xTaskDelayUntil(&activacionPrevia, periodo);
        pConfig->numActivaciones++;
        ESP_LOGI(pConfig->tag, "Numero de activaciones: %lu", pConfig->numActivaciones);

        /* Actualiza la máquina de estados en cada ejecución */
        fsm_update (fsm_controlDeposito);
    }
}