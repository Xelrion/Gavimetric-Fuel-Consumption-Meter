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
#include "estadoSistema.h"
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
 * Punto de entrada de la tarea de cálculo de consumo
 ***********************************************************************************************************/

/* Configuración de la tarea de cálculo de medias */
void tareaConsumoSet(tareaConsumoInfo_t* pTaskInfo, bufferCircular_t* pMedidas, bufferCircular_t* pConsumo, estadoSistema_t* pEstadoSist)
{
    pTaskInfo->pMedidas = pMedidas;
    pTaskInfo->pConsumo = pConsumo;
    pTaskInfo->pEstadoSist = pEstadoSist;
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
    estadoSistema_t* pEstadoSist = ((tareaConsumoInfo_t*)pData)->pEstadoSist;

    ESP_LOGD(pConfig->tag, "Periodo de planificación: %lu ms", pConfig->periodo);
    ESP_LOGD(pConfig->tag, "Número inicial de activaciones: %lu", pConfig->numActivaciones);
    // ESP_LOGI(pConfig->tag, "LED en pin %u. Nivel inicial: %s", pLED->pin, (pLED->nivel==0)? "LOW":"HIGH");
    // ESP_LOGI(pConfig->tag, "Mutex en %x", (unsigned int)(pLED->sem));

    /* Prepara la tarea */
    const TickType_t periodo = (pConfig->periodo) / portTICK_PERIOD_MS;
    TickType_t activacionPrevia = xTaskGetTickCount();

    /* Parámetros de funcionamiento configurables */
    int periodo_medidas = 500;  // periodo de toma de medidas en ms

    /* Estado del sistema */
    estadoSistemaComando_t comando = DESACTIVADA;

    /* Bucle de cálculo de consumo */
    bool continuar = true;
    double medida1, medida2, consumo;
    bool medida_puntual = true;    // comprueba si ya se ha tomado la medida solicitada en control manual puntual
    bool periodo_medidas_modif = false; // si en esta ejecución se ha modificado el periodo de medidas, no se calcula el consumo para evitar datos erróneos

    while( continuar )
    {
        /* Espera a la siguiente activación */  
        xTaskDelayUntil(&activacionPrevia, periodo);
        pConfig->numActivaciones++;
        ESP_LOGD(pConfig->tag, "Numero de activaciones: %lu", pConfig->numActivaciones);

        /* Nota: comprobar los parámetros de tiempo de toma de medidas, presente
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
