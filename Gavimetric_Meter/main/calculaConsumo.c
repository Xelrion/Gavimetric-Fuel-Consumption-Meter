/***********************************************************************************************************
 * MÓDULO DE MONITORIZACIÓN DEL DEPÓSITO
 * Objeto cíclico: Cálculo de medidas de consumo
 * Lee el buffer de medidas y calcula el consumo medio por cada pareja
 * La decisión de calcular o no el consumo se realiza mediante una máquina de estados
 ***********************************************************************************************************/

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

#include "myTaskConfig.h"
#include "bufferCircular.h"
#include "fsm.h"
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

/***********************************************************************************************************
 * Funciones de control sobre el timer para la espera de estabilización
 ***********************************************************************************************************/

/* Comprueba si el timer ha vencido o no está activo */
int  timer_expired(int* timer)
{
    return (timer == 0);
}

/* Inicia el timer con el tiempo especificado en segundos
El periodo de cálculo de consumo se corresponde al periodo de ejecución de la tarea. */
void timer_start(int* timer, int tiempo_max_s, int periodo_ms)
{
    timer = (tiempo_max_s * 1000) / periodo_ms;
}

/* Decrementa el timer si aún no ha llegado a 0 */
void timer_next(int* timer)
{
    if (timer > 0) --timer;
    ESP_LOGD(TAG, "Tiempo restante: %d", timer);
}

/***********************************************************************************************************
 * Funciones de condición de transferencia
 ***********************************************************************************************************/
// Nota: actualizar las condiciones de comprobación con las funciones reales

/* ESTADO ORIGEN 1: MEDICIÓN MANUAL DESACTIVADA */

bool monitorizacion_c16 (void *params)
{
    bool espera_estabilizacion = 0;
    bool estado_llenado = 0;
    bool estado_vaciado = 0;
    return espera_estabilizacion + estado_llenado + estado_vaciado;
}
bool monitorizacion_c11 (void *params)
{
    return 0; // Nota: cambiar a return emergencia();
}
bool monitorizacion_c12 (void *params)
{
    return 0; // Nota: cambiar a return comando_medida_puntual();
}
bool monitorizacion_c13 (void *params)
{
    return 0; // Nota: cambiar a return comando_medida_continuada();
}
bool monitorizacion_c14 (void *params)
{
    bool comando_modo_remoto = 0;
    bool peticion_medida = 0;
    return comando_modo_remoto + !peticion_medida;
}
bool monitorizacion_c15 (void *params)
{
    bool comando_modo_remoto = 0;
    bool peticion_medida = 0;
    return comando_modo_remoto + peticion_medida;
}

/* ESTADO ORIGEN 2: MEDICIÓN PUNTUAL A CONSOLA */

bool monitorizacion_c26 (void *params)
{
    bool espera_estabilizacion = 0;
    bool estado_llenado = 0;
    bool estado_vaciado = 0;
    return espera_estabilizacion + estado_llenado + estado_vaciado;
}
bool monitorizacion_c21 (bool *medida_puntual)
{
    bool emergencia = 0;
    return emergencia + medida_puntual;
}
bool monitorizacion_c23 (void *params)
{
    return 0; // Nota: cambiar a return comando_medida_continuada();
}
bool monitorizacion_c24 (void *params)
{
    bool comando_modo_remoto = 0;
    bool peticion_medida = 0;
    return comando_modo_remoto + !peticion_medida;
}
bool monitorizacion_c25 (void *params)
{
    bool comando_modo_remoto = 0;
    bool peticion_medida = 0;
    return comando_modo_remoto + peticion_medida;
}

/* ESTADO ORIGEN 3: MEDICIÓN CONTINUADA A CONSOLA */

bool monitorizacion_c36 (void *params)
{
    bool espera_estabilizacion = 0;
    bool estado_llenado = 0;
    bool estado_vaciado = 0;
    return espera_estabilizacion + estado_llenado + estado_vaciado;
}
bool monitorizacion_c31 (void *params)
{
    return 0; // Nota: cambiar a return emergencia();
}
bool monitorizacion_c32 (void *params)
{
    return 0; // Nota: cambiar a return comando_medida_puntual();
}
bool monitorizacion_c34 (void *params)
{
    bool comando_modo_remoto = 0;
    bool peticion_medida = 0;
    return comando_modo_remoto + !peticion_medida;
}
bool monitorizacion_c35 (void *params)
{
    bool comando_modo_remoto = 0;
    bool peticion_medida = 0;
    return comando_modo_remoto + peticion_medida;
}

/* ESTADO ORIGEN 4: MEDICIÓN EN REMOTO DESACTIVADA */

bool monitorizacion_c46 (void *params)
{
    bool espera_estabilizacion = 0;
    bool estado_llenado = 0;
    bool estado_vaciado = 0;
    return espera_estabilizacion + estado_llenado + estado_vaciado;
}
bool monitorizacion_c41 (void *params)
{
    bool emergencia = 0;
    bool comando_modo_manual = 0;
    return emergencia + comando_modo_manual;
}
bool monitorizacion_c42 (void *params)
{
    return 0; // Nota: cambiar a return comando_medida_puntual();
}
bool monitorizacion_c43 (void *params)
{
    return 0; // Nota: cambiar a return comando_medida_continuada();
}
bool monitorizacion_c45 (void *params)
{
    bool comando_modo_remoto = 0;
    bool peticion_medida = 0;
    return comando_modo_remoto + peticion_medida;
}

/* ESTADO ORIGEN 5: MEDICIÓN EN REMOTO ACTIVADA */

bool monitorizacion_c56 (void *params)
{
    bool espera_estabilizacion = 0;
    bool estado_llenado = 0;
    bool estado_vaciado = 0;
    return espera_estabilizacion + estado_llenado + estado_vaciado;
}
bool monitorizacion_c51 (void *params)
{
    bool emergencia = 0;
    bool comando_modo_manual = 0;
    return emergencia + comando_modo_manual;
}
bool monitorizacion_c52 (void *params)
{
    return 0; // Nota: cambiar a return comando_medida_puntual();
}
bool monitorizacion_c53 (void *params)
{
    return 0; // Nota: cambiar a return comando_medida_continuada();
}
bool monitorizacion_c54 (void *params)
{
    bool comando_modo_remoto = 0;
    bool peticion_medida = 0;
    return comando_modo_remoto + !peticion_medida;
}

/* ESTADO ORIGEN 6: ESPERA DE ESTABILIZACIÓN Y VACIADO/LLENADO */

bool monitorizacion_c62 (int *timer)
{
    bool emergencia = 0;
    bool comando_medida_puntual = 0;
    return !emergencia * comando_medida_puntual * timer_expired(timer);
}
bool monitorizacion_c63 (int *timer)
{
    bool emergencia = 0;
    bool comando_medida_continuada = 0;
    return !emergencia * comando_medida_continuada * timer_expired(timer);
}
bool monitorizacion_c64 (int *timer)
{
    bool emergencia = 0;
    bool comando_modo_remoto = 0;
    bool peticion_medida = 0;
    return !emergencia * comando_modo_remoto * !peticion_medida * timer_expired(timer);
}
bool monitorizacion_c65 (int *timer)
{
    bool emergencia = 0;
    bool comando_modo_remoto = 0;
    bool peticion_medida = 0;
    return !emergencia * comando_modo_remoto * peticion_medida * timer_expired(timer);
}
bool monitorizacion_c61 (int *timer)
{
    return timer_expired(timer);
}

/***********************************************************************************************************
 * Inicialización de la máquina de estados
 ***********************************************************************************************************/

fsm_t* monitorizacionDeposito_new (bool *medida_puntual, int *timer)
{
    static fsm_trans_t monitorizacionDeposito_tt[] = {
        {  1, monitorizacion_c16, 6, NULL },
        {  1, monitorizacion_c11, 1, NULL },
        {  1, monitorizacion_c12, 2, NULL },
        {  1, monitorizacion_c13, 3, NULL },
        {  1, monitorizacion_c14, 4, NULL },
        {  1, monitorizacion_c15, 5, NULL },
        {  2, monitorizacion_c26, 6, NULL },
        {  2, monitorizacion_c21, 1, NULL },
        {  2, monitorizacion_c23, 3, NULL },
        {  2, monitorizacion_c24, 4, NULL },
        {  2, monitorizacion_c25, 5, NULL },
        {  3, monitorizacion_c36, 6, NULL },
        {  3, monitorizacion_c31, 1, NULL },
        {  3, monitorizacion_c32, 2, NULL },
        {  3, monitorizacion_c34, 4, NULL },
        {  3, monitorizacion_c35, 5, NULL },
        {  4, monitorizacion_c46, 6, NULL },
        {  4, monitorizacion_c41, 1, NULL },
        {  4, monitorizacion_c42, 2, NULL },
        {  4, monitorizacion_c43, 3, NULL },
        {  4, monitorizacion_c45, 5, NULL },
        {  5, monitorizacion_c56, 6, NULL },
        {  5, monitorizacion_c51, 1, NULL },
        {  5, monitorizacion_c52, 2, NULL },
        {  5, monitorizacion_c53, 3, NULL },
        {  5, monitorizacion_c54, 4, NULL },
        {  6, monitorizacion_c62, 2, NULL },
        {  6, monitorizacion_c63, 3, NULL },
        {  6, monitorizacion_c64, 4, NULL },
        {  6, monitorizacion_c65, 5, NULL },
        {  6, monitorizacion_c61, 1, NULL },
        { -1, NULL, -1, NULL },
    };
    return fsm_new (monitorizacionDeposito_tt);
}

/***********************************************************************************************************
 * Punto de entrada de la tarea de cálculo de consumo
 ***********************************************************************************************************/

/* Configuración de la tarea de cálculo de medias */
void tareaConsumoSet(tareaConsumoInfo_t* pTaskInfo, bufferCircular_t* pMedidas, bufferCircular_t* pConsumo)
{
    pTaskInfo->pMedidas = pMedidas;
    pTaskInfo->pConsumo = pConsumo;
}

/* Punto de entrada de la tarea de cálculo de valores medios */
void tareaConsumo(void* pParametros)
{
    /* Estructuras para intercambio de información */
    /* Entre la tarea y la aplicación principal */
    taskConfig_t* pConfig = ((taskInfo_t *)pParametros)->pConfig;
    void*           pData = ((taskInfo_t *)pParametros)->pData;
    bufferCircular_t*  pMedidas = ((tareaConsumoInfo_t*)pData)->pMedidas;
    bufferCircular_t*  pConsumo = ((tareaConsumoInfo_t*)pData)->pConsumo;

    ESP_LOGD(pConfig->tag, "Periodo de planificación: %lu ms", pConfig->periodo);
    ESP_LOGD(pConfig->tag, "Número inicial de activaciones: %lu", pConfig->numActivaciones);
    // ESP_LOGI(pConfig->tag, "LED en pin %u. Nivel inicial: %s", pLED->pin, (pLED->nivel==0)? "LOW":"HIGH");
    // ESP_LOGI(pConfig->tag, "Mutex en %x", (unsigned int)(pLED->sem));

    /* Prepara la tarea */
    const TickType_t periodo = (pConfig->periodo) / portTICK_PERIOD_MS;
    TickType_t activacionPrevia = xTaskGetTickCount();

    /* Parámetros de funcionamiento configurables */
    int periodo_medidas = 500;  // periodo de toma de medidas en ms
    int espera_estabilizacion = 5;  // tiempo de espera de estabilización

    /* Bucle de cálculo de consumo */
    bool continuar = true;
    double medida1, medida2, consumo;
    bool medida_puntual = true;    // comprueba si ya se ha tomado la medida solicitada en control manual puntual
    int timer_estabilizacion = 0;   // timer para espera de tiempo de establecimiento
    bool periodo_medidas_modif = false; // si en esta ejecución se ha modificado el periodo de medidas, no se calcula el consumo para evitar datos erróneos

    while( continuar )
    {
        /* Espera a la siguiente activación */  
        xTaskDelayUntil(&activacionPrevia, periodo);
        pConfig->numActivaciones++;
        ESP_LOGD(pConfig->tag, "Numero de activaciones: %lu", pConfig->numActivaciones);

        /* Nota: comprobar los parámetros de tiempo de toma de medidas y de espera de estabilización, presentes
        en el recurso compartido de configuración */
        // Si se modifica el periodo de medidas, establecer periodo_medidas_modif a true

        /* Nota: actualizar el estado de la máquina de estados */

        /* Lee y calcula el consumo entre dos medidas, siempre que haya suficientes en el buffer */
        while (medidasDisponibles(pMedidas) & !periodo_medidas_modif) {

            /* Extrae las dos medidas más antiguas almacenadas en el buffer*/
            bufferCircularSaca(pMedidas, &medida1);
            bufferCircularSaca(pMedidas, &medida2);

            /* Calcula el consumo medio entre ellas */
            consumo = consumoHora(medida1, medida2, periodo_medidas);

            /* Introduce el consumo calculado en el buffer de consumo */
            if (!bufferCircularMete(pConsumo, consumo)) {
                continuar = false;
            }
            ESP_LOGI(pConfig->tag, "Consumo/h: %f", consumo);

            /* Si medida_puntual == false, se establece a true y se termina el bucle (el sistema solo ha solicitado una medida) */
            if (medida_puntual == false)
            {
                medida_puntual = true;
                break;
            }
        }

        /* Nota: si el estado de la máquina es "espera", actualiza el timer */
    }
}
