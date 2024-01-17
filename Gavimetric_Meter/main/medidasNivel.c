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
 * Punto de entrada de la tarea de toma de medidas
 ***********************************************************************************************************/

/* Configuración de la tarea de toma de medidas */
void tareaMedidasNivelSet(tareaMedidasNivelInfo_t* pTaskInfo, bufferCircular_t* pMedidas)
{
    pTaskInfo->pMedidas   = pMedidas;
}

/* Punto de entrada de la tarea de lectura de entradas analógicas */
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

    /* Bucle de toma de medidas */
    bool continuar = true;
    double medida = 0.0;

    while( continuar )
    {
        /* Espera a la siguiente activación */  
        xTaskDelayUntil(&activacionPrevia, periodo);

        pConfig->numActivaciones++;
        ESP_LOGD(pConfig->tag, "Numero de activaciones: %lu", pConfig->numActivaciones);

        /* Lectura del valor del potenciometro */
        medida = medida_bascula();

        /* Comprueba si el valor medido supera el máximo o el mínimo */
        // Nota: comparar el valor medido con el recurso protegido de configuración, que contiene los valores máximo y mínimo
        // Si supera el valor máximo, se notifica al recurso de avisos de control del depósito
        // Si supera el valor mínimo, lo mismo

        // Nota: Comprueba el estado de la máq. de estados de cálculo de consumo
        // Si está en "desactivado" o "espera", limpia el buffer de medidas
        // Si no, introduce la última medida tomada
        /* Incluye la medida tomada en el buffer de lecturas */
        if (!bufferCircularMete(pMedidas, medida))
        {
            continuar = false;
        };

        // Nota: Comprueba si ha cambiado el periodo de toma de medidas en los parámetros de conf.
        // Si ha cambiado, actualiza el periodo
    }
}