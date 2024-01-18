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
#include "myTaskConfig.h"
#include "medidasNivel.h"

/* Etiqueta para depuración */
const char* TAG = "medidasNivel";

/***********************************************************************************************************
 * Funciones de la tarea
 ***********************************************************************************************************/

/* Función de petición de medidas a la báscula */
double medida_bascula()
{
    /* Nota: devolver valor de medida de la báscula */
    return 0;
}

/***********************************************************************************************************
 * Funciones de control sobre el timer para el periodo de medidas
 ***********************************************************************************************************/

/* Comprueba si el timer ha vencido o no está activo */
int  timer_expired(int* timer)
{
    return (timer == 0);
}

/* Inicia el timer con el tiempo especificado en milisegundos
El periodo de cálculo de consumo se corresponde al periodo de ejecución fijo de la tarea. */
void timer_start(int* timer, int tiempo_max_ms, int periodo_ms)
{
    timer = (tiempo_max_ms) / periodo_ms;
}

/* Decrementa el timer si aún no ha llegado a 0 */
void timer_next(int* timer)
{
    if (timer > 0) --timer;
    ESP_LOGD(TAG, "Tiempo restante: %d", timer);
}

/***********************************************************************************************************
 * Punto de entrada de la tarea de toma de medidas
 ***********************************************************************************************************/

/* Configuración de la tarea de toma de medidas */
void tareaMedidasNivelSet(tareaMedidasNivelInfo_t* pTaskInfo, bufferCircular_t* pMedidas)
{
    pTaskInfo->pMedidas   = pMedidas;
}

/* Punto de entrada de la tarea de toma de medidas */
void tareaLectura(void* pParametros)
{
    /* Estructuras para intercambio de información */
    /* Entre la tarea y la aplicación principal */
    taskConfig_t*       pConfig = ((taskInfo_t *)pParametros)->pConfig;
    void*                 pData = ((taskInfo_t *)pParametros)->pData;
    bufferCircular_t*  pMedidas = ((tareaMedidasNivelInfo_t*)pData)->pMedidas;

    ESP_LOGI(pConfig->tag, "Periodo de planificación: %lu ms", pConfig->periodo);
    ESP_LOGI(pConfig->tag, "Número inicial de activaciones: %lu", pConfig->numActivaciones);

    /* Prepara la tarea */
    const TickType_t periodo = (pConfig->periodo) / portTICK_PERIOD_MS;
    TickType_t activacionPrevia = xTaskGetTickCount();

    /* Parámetros de funcionamiento configurables */
    int periodo_medidas = pConfig->periodo;  // periodo de toma de medidas en ms, inicialmente coincide con el periodo de la tarea

    /* Bucle de toma de medidas */
    bool continuar = true;
    double medida = 0.0;
    int timer_periodo_medidas = 0;  // cada vez que llega a 0, se toma una medida y se reinicia
    int timer_espera_estabilizacion = 0;    // mientras esté activo, no se tomarán medidas. Al desactivarse, se modifica el estado de espera

    while( continuar )
    {
        /* Espera a la siguiente activación */  
        xTaskDelayUntil(&activacionPrevia, periodo);

        pConfig->numActivaciones++;
        ESP_LOGD(pConfig->tag, "Numero de activaciones: %lu", pConfig->numActivaciones);

        // Nota: Comprueba si ha cambiado el periodo de toma de medidas en los parámetros de conf.
        // Si ha cambiado, actualiza el periodo de medidas, reinicia el timer y limpia el buffer de medidas

        /* Lectura de la medida de la báscula */
        medida = medida_bascula();

        /* Comprueba si el valor medido supera el máximo o el mínimo */
        // Nota: comparar el valor medido con el recurso protegido de configuración, que contiene los valores máximo y mínimo
        // Si supera el valor máximo, se notifica al recurso de avisos de control del depósito
        // Si supera el valor mínimo, lo mismo

        // Nota: Comprueba el estado de la máq. de estados de cálculo de consumo
        // Si está en "desactivado" o "espera", limpia el buffer de medidas y no toma medida
        // Luego, comprueba si el timer de periodo de medidas ha llegado a 0
        // Finalmente, comprueba si el timer de espera de estabilización ha llegado a 0
        // Si se cumplen todas condiciones, reinicia el timer e incluye la medida en el buffer
        /* Incluye la medida tomada en el buffer de lecturas */
        if (!bufferCircularMete(pMedidas, medida))
        {
            continuar = false;
        };

        /* Actualiza el timer de periodo de medidas*/
        timer_next(&timer_periodo_medidas);
    }
}