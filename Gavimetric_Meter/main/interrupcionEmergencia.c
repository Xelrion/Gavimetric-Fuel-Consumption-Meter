/***********************************************************************************************************
 * MÓUDLO DE GESTIÓN DE INTERRUPCIONES
 * Tarea esporádica: se activa mediante interrupción al detectar la señal de desbordamiento o la activación
 * del pulsador de emergencia. Activa la parada de emergencia.
 ***********************************************************************************************************/

#include "interrupcionEmergencia.h"
#include "paradaEmergencia.h"
#include "driver/gpio.h"

/***********************************************************************************************************
 * Pines de interrupción
 ***********************************************************************************************************/

#define GPIO_PIN_PULSADOR 14 // Pin asociado a la interrupción del pulsador de emergencia
#define GPIO_PIN_DESBORDAMIENTO 27 // Pin asociado a la interrupción del sensor de desbordamiento

/***********************************************************************************************************
 * Función de interrupción
 ***********************************************************************************************************/

void IRAM_ATTR interrupcionEmergencia(void* pParams)
{
    /* Referencia a la estructura de emergencia */
    paradaEmergencia_t* pEmergencia  = (paradaEmergencia_t*)pParams;

    /* Accede al recurso protegido y activa la parada de emergencia */
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    xSemaphoreTakeFromISR(pEmergencia->mutex, &higherPriorityTaskWoken);
    pEmergencia->paradaEmergencia = true;
    xSemaphoreGiveFromISR(pEmergencia->mutex, &higherPriorityTaskWoken);
}

/***********************************************************************************************************
 * Configuración de pines de interrupción y función asociada
 ***********************************************************************************************************/

void configInterrupcionEmergencia(paradaEmergencia_t* pEmergencia)
{
    /* Configuración del pulsador de emergencia */
    gpio_config_t io_conf_1 = {
        .pin_bit_mask = (1ULL<<GPIO_PIN_PULSADOR),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_POSEDGE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
    };
    gpio_config(&io_conf_1);

    /* Configuración del sensor de desbordamiento */
    gpio_config_t io_conf_2 = {
        .pin_bit_mask = (1ULL<<GPIO_PIN_DESBORDAMIENTO),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_POSEDGE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
    };
    gpio_config(&io_conf_2);

    /* Configuración de la interrupción */
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_PIN_PULSADOR, interrupcionEmergencia, (void*) pEmergencia);
    gpio_isr_handler_add(GPIO_PIN_DESBORDAMIENTO, interrupcionEmergencia, (void*) pEmergencia);
}
