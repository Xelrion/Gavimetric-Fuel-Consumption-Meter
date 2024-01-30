/***********************************************************************************************************
 * MÓDULO DE SISTEMA REMOTO
 * Objeto cíclico: comunicación con el sistema remoto
 * Lee la señal digital de petición de medidas y envía periódicamente medidas analógicas
 ***********************************************************************************************************/

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"
#include "esp_timer.h"

#include "myTaskConfig.h"
#include "driver/gpio.h"
#include "bufferCircular.h"
#include "estadoSistema.h"
#include "configSistema.h"
#include "paradaEmergencia.h"
#include "comunicacionRemoto.h"

/* Etiqueta para depuración */
static char* TAG = "comunicacionRemoto";

/***********************************************************************************************************
 * Salida digital del sistema remoto
 ***********************************************************************************************************/

#define GPIO_PIN_SIGNAL 19 // Pin asignado a la señal digital del sistema remoto

/* Estado de activación de la señal (activo a nivel alto) */
#define SIGNAL_LEVEL_ACTIVE 1

/***********************************************************************************************************
 * Valor máximo de la señal analógica de salida
 ***********************************************************************************************************/

#define VOLTAJE_MAXIMO 10.00

/***********************************************************************************************************
 * Funciones de comunicación con el sistema remoto
 ***********************************************************************************************************/

/* Envía una medida de consumo en forma de señal analógica, de entre 0 y 10 voltios */
void enviar_consumo( double voltaje )
{
    /* Nota: Envía una señal analógica con el voltaje especificado al sistema remoto */
}

int estado_señal()
{
    bool button_state = (SIGNAL_LEVEL_ACTIVE == gpio_get_level(GPIO_PIN_SIGNAL));
    ESP_LOGD(TAG, "Estado de la señal de petición: %d", gpio_get_level(GPIO_PIN_SIGNAL));
    if (button_state) ESP_LOGI(TAG, "Señal de petición activa");

    return button_state;
}

/***********************************************************************************************************
 * Funciones de cálculo de señal analógica
 ***********************************************************************************************************/

/* Calcula el voltaje de la señal analógica que se debe enviar a partir del consumo medido y el consumo máximo configurado */
double medida_analogica( double consumo, double consumoMaximo, double voltaje_maximo)
{
    double señalAnalogica = voltaje_maximo*(consumo/consumoMaximo);
    if (señalAnalogica < 0) { señalAnalogica = 0; }
    if (señalAnalogica > voltaje_maximo) { señalAnalogica = voltaje_maximo; }

    return señalAnalogica;
}

/***********************************************************************************************************
 * Punto de entrada de la tarea de comunicación con el sistema remoto
 ***********************************************************************************************************/

/* Configuración de la tarea de comunicación con el sistema remoto */
void tareaComunicacionRemotoSet(tareaComunicacionRemotoInfo_t* pTaskInfo, bufferCircular_t* pConsumoRemoto, estadoSistema_t* pEstadoSist, configSistema_t* pConfigSist, paradaEmergencia_t* pEmergencia)
{
    pTaskInfo->pConsumoRemoto = pConsumoRemoto;
    pTaskInfo->pEstadoSist = pEstadoSist;
    pTaskInfo->pConfigSist = pConfigSist;
    pTaskInfo->pEmergencia = pEmergencia;
}

/* Punto de entrada de la tarea de comunicación con el sistema remoto */
void tareaComunicacionRemoto(void* pParametros)
{
    /* Estructuras para intercambio de información */
    /* Entre la tarea y la aplicación principal */
    taskConfig_t*            pConfig  = ((taskInfo_t *)pParametros)->pConfig;
    void*                      pData  = ((taskInfo_t *)pParametros)->pData;
    bufferCircular_t* pConsumoRemoto  = ((tareaComunicacionRemotoInfo_t*)pData)->pConsumoRemoto;
    estadoSistema_t*     pEstadoSist  = ((tareaComunicacionRemotoInfo_t*)pData)->pEstadoSist;
    configSistema_t*     pConfigSist  = ((tareaComunicacionRemotoInfo_t*)pData)->pConfigSist;
    paradaEmergencia_t*  pEmergencia  = ((tareaComunicacionRemotoInfo_t*)pData)->pEmergencia;

    ESP_LOGI(pConfig->tag, "Periodo de planificación: %lu ms", pConfig->periodo);
    ESP_LOGI(pConfig->tag, "Número inicial de activaciones: %lu", pConfig->numActivaciones);

    /* Prepara la tarea */
    const TickType_t periodo = (pConfig->periodo) / portTICK_PERIOD_MS;
    TickType_t activacionPrevia = xTaskGetTickCount();

    /* Configuración del sistema */
    double consumoMaximo = 10;

    /* Estado del sistema */
    estadoSistemaComando_t comando;
    bool peticionMedidas = false;
    bool emergencia;

    /* Configuración del pin digital */
    gpio_set_pull_mode(GPIO_PIN_SIGNAL, GPIO_PULLDOWN_ONLY);

    /* Bucle de comunicación con el display */
    double medidaConsumo = 0;
    bool continuar = true;

    /* DEBUG: TIEMPO DE EJECUCIÓN */
    uint64_t startTime;
    uint64_t endTime;
    uint64_t executionTime;

    while ( continuar )
    {
        /* Espera a la siguiente activación */  
        xTaskDelayUntil(&activacionPrevia, periodo);

        pConfig->numActivaciones++;
        ESP_LOGD(pConfig->tag, "Numero de activaciones: %lu", pConfig->numActivaciones);

        /* DEBUG: TIEMPO DE EJECUCIÓN */
        startTime = esp_timer_get_time();

        /* Comprueba si la parada de emergencia se encuentra activa */
        paradaEmergenciaLeer(pEmergencia, &emergencia);
        /* Lee el último comando de petición de medidas del sistema */
        estadoSistemaLeerComando(pEstadoSist, &comando);
        /* Comprueba la señal digital de petición de medidas */
        peticionMedidas = estado_señal();
        /* Actualiza el parámetro de configuración de consumo máximo (10 voltios) */
        if ( !configSistemaLeerConsumoMax(pConfigSist, &consumoMaximo) ) { continuar = false; }

        /* ENVÍO DE MEDIDAS DE CONSUMO A LA CONSOLA */
        /* Si las medidas no están en modo automático o la señal de petición está inactiva, se limpia el buffer */
        if (emergencia || comando != MEDIDA_REMOTO ||  !peticionMedidas)
        {
            if( !bufferCircularLimpia(pConsumoRemoto) ) { continuar = false; }
        }
        /* Si el modo automático y la señal de petición están activos, se envían todas las medidas disponibles al sistema remoto */
        while (!emergencia && !bufferCircularVacio(pConsumoRemoto) && comando == MEDIDA_REMOTO && peticionMedidas)
        {
            if( !bufferCircularSaca(pConsumoRemoto, &medidaConsumo) ) { continuar = false; }
            medidaConsumo = medida_analogica(medidaConsumo, consumoMaximo, VOLTAJE_MAXIMO);
            enviar_consumo(medidaConsumo);
        }

        /* DEBUG: TIEMPO DE EJECUCIÓN */
        endTime = esp_timer_get_time();
        executionTime = endTime - startTime;
        printf("Duración de tarea comunicacionRemoto: %lld microsegundos\n", executionTime);
    }
}