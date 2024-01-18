/* Implementación de buffer circular simple */

#include "bufferCircular.h"

// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"
#include "freertos/semphr.h"

/* Crea un buffer circular */
bool bufferCircularCrea( bufferCircular_t* pBuffer, const char* etiqueta)
{
    /* Reservar un semaforo de exclusión mutua */
    if ((pBuffer->mutex = xSemaphoreCreateMutex()) != NULL)
    {
        /* Inicializar cabeza, cola y numElementos */
        pBuffer->cabeza = 0;
        pBuffer->cola = 0;
        pBuffer->numElementos = 0;
        pBuffer->tag = etiqueta;
        pBuffer->err = BUFFER_OK;
    }
    else
    {
        pBuffer->err = BUFFER_ERR_MUTEX;
        ESP_LOGE(pBuffer->tag, "Error al intentar crear el mutex");
    }
    return (pBuffer->err == BUFFER_OK);
}

/* Libera los recursos usados por un buffer circular */
bool bufferCircularLibera( bufferCircular_t* pBuffer )
{
    /* Libera el semaforo de exclusión mutua */
    vSemaphoreDelete(pBuffer->mutex);
    pBuffer->err = BUFFER_OK;
    ESP_LOGD(pBuffer->tag, "Mutex liberado");

    return (pBuffer->err == BUFFER_OK);
}

/* Introduce un nuevo elemento en la cabeza de un buffer circular */
bool bufferCircularMete( bufferCircular_t* pBuffer, double valor )
{
    if (xSemaphoreTake(pBuffer->mutex, (TickType_t) 10) == pdTRUE)
    {
        if (pBuffer->numElementos < NUMELEMENTOS)
        {
            (pBuffer->valor)[pBuffer->cabeza++] = valor;
            ESP_LOGD(pBuffer->tag, "Valor: %f en posición %d", (pBuffer->valor)[(pBuffer->cabeza)-1], (pBuffer->cabeza)-1);

            pBuffer->cabeza = pBuffer->cabeza % NUMELEMENTOS;
            pBuffer->numElementos++;
            ESP_LOGD(pBuffer->tag, "%d elementos almacenados\n\n", (pBuffer->numElementos));

            pBuffer->err = BUFFER_OK;
        }
        else
        {
            ESP_LOGE(pBuffer->tag, "Fallo por buffer lleno");
            ESP_LOGE(pBuffer->tag, "al intentar introducir valor: %f en posición %d", valor, (pBuffer->cabeza));

            pBuffer->err = BUFFER_ERR_LLENO;
        }
        xSemaphoreGive(pBuffer->mutex);
    }
    else
    {
        ESP_LOGE(pBuffer->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pBuffer->tag, "para introducir valor: %f en posición %d", valor, (pBuffer->cabeza));

        pBuffer->err = BUFFER_ERR_MUTEX;
    }

    return (pBuffer->err == BUFFER_OK);
}

/* Retira un elemento por la cola */
bool bufferCircularSaca( bufferCircular_t* pBuffer, double* pValor )
{
    if (xSemaphoreTake(pBuffer->mutex, (TickType_t) 10) == pdTRUE)
    {
        if (pBuffer->numElementos)
        {
            *pValor = (pBuffer->valor)[pBuffer->cola++];
            ESP_LOGD(pBuffer->tag, "Saca valor: %f de posición %d", *pValor, (pBuffer->cola)-1);

            pBuffer->cola = pBuffer->cola % NUMELEMENTOS;
            pBuffer->numElementos--;
            ESP_LOGD(pBuffer->tag, "%d elementos almacenados", (pBuffer->numElementos));

            pBuffer->err = BUFFER_OK;
        }
        else
        {
            ESP_LOGE(pBuffer->tag, "Fallo por buffer vacio");
            ESP_LOGE(pBuffer->tag, "al intentar retirar valor en posición %d", (pBuffer->cola));

            pBuffer->err = BUFFER_ERR_VACIO;
        }
        xSemaphoreGive(pBuffer->mutex);
    }
    else
    {
        ESP_LOGE(pBuffer->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pBuffer->tag, "para retirar valor en posición %d", (pBuffer->cabeza));

        pBuffer->err = BUFFER_ERR_MUTEX;
    }

    return (pBuffer->err == BUFFER_OK);
}

/* Comprueba si está lleno */
bool bufferCircularLleno( bufferCircular_t* pBuffer )
{
    bool bufferLleno = false;

    if (xSemaphoreTake(pBuffer->mutex, (TickType_t) 10) == pdTRUE)
    {
        bufferLleno = (pBuffer->numElementos == NUMELEMENTOS);
        xSemaphoreGive(pBuffer->mutex);
        pBuffer->err = BUFFER_OK;
        xSemaphoreGive(pBuffer->mutex);
    }
    else
    {
        ESP_LOGE(pBuffer->tag, "Fallo al intentar tomar mutex");
        pBuffer->err = BUFFER_ERR_MUTEX;
    }

    return bufferLleno;
}

/* Comprueba si está vacio */
bool bufferCircularVacio( bufferCircular_t* pBuffer )
{
    bool bufferVacio = false;

    if (xSemaphoreTake(pBuffer->mutex, (TickType_t) 10) == pdTRUE)
    {
        bufferVacio = (pBuffer->numElementos == 0);
        xSemaphoreGive(pBuffer->mutex);
        pBuffer->err = BUFFER_OK;
        xSemaphoreGive(pBuffer->mutex);
    }
    else
    {
        ESP_LOGE(pBuffer->tag, "Fallo al intentar tomar mutex");
        pBuffer->err = BUFFER_ERR_MUTEX;
    }

    return bufferVacio;
}

/* Comprueba cuántos elementos contiene actualmente el buffer */
bool bufferCircularNumElementos( bufferCircular_t* pBuffer, int* pValor )
{
    int numElementos = 0;

    if (xSemaphoreTake(pBuffer->mutex, (TickType_t) 10) == pdTRUE)
    {
        numElementos = (pBuffer->numElementos);
        xSemaphoreGive(pBuffer->mutex);
        pBuffer->err = BUFFER_OK;
        xSemaphoreGive(pBuffer->mutex);
    }
    else
    {
        ESP_LOGE(pBuffer->tag, "Fallo al intentar tomar mutex");
        pBuffer->err = BUFFER_ERR_MUTEX;
    }

    return (pBuffer->err == BUFFER_OK);
}

/* Vacía la cola del buffer */
bool bufferCircularLimpia( bufferCircular_t* pBuffer )
{
    if (xSemaphoreTake(pBuffer->mutex, (TickType_t) 10) == pdTRUE)
    {
        while (pBuffer->numElementos)
        {
            pBuffer->cola++;
            pBuffer->cola = pBuffer->cola % NUMELEMENTOS;
            pBuffer->numElementos--;
        }
        ESP_LOGE(pBuffer->tag, "Buffer vaciado");
        pBuffer->err = BUFFER_OK;

        xSemaphoreGive(pBuffer->mutex);
    }
    else
    {
        ESP_LOGE(pBuffer->tag, "Fallo al intentar tomar mutex");
        ESP_LOGE(pBuffer->tag, "para retirar valor en posición %d", (pBuffer->cabeza));

        pBuffer->err = BUFFER_ERR_MUTEX;
    }

    return (pBuffer->err == BUFFER_OK);
}