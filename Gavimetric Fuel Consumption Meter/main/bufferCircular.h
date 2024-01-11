/* Manejo de un buffer circular simple */

#ifndef BUFFERCIRCULAR_H
#define BUFFERCIRCULAR_H

#include <freertos/FreeRTOS.h>
#include "freertos/semphr.h"

/* Numero máximo de elementos que puede contener el buffer */
#define NUMELEMENTOS    100

/* Codigos de error devueltos por la funciones */
typedef enum _bufferCircularError
{
    BUFFER_OK,
    BUFFER_ERR_MUTEX,
    BUFFER_ERR_LLENO,
    BUFFER_ERR_VACIO

} bufferCircularError_t;

typedef struct _bufferCircular
{
    /* Nombre del buffer */
    const char* tag;

    /* Semáforo para gestionar la exclusión mútua */
    SemaphoreHandle_t mutex;

    /* Indices al proximo elemento a sacar y
    al próximo a meter y número actual de elementos */
    int cabeza, cola, numElementos;

    /* Almacen con los valores almacenados */
    double valor[NUMELEMENTOS];       // valores almacenados

    /* Código de error devuelto por la última operación */
    bufferCircularError_t err;

} bufferCircular_t;

/* Crea un buffer circular */
bool bufferCircularCrea( bufferCircular_t* pBuffer, const char* etiqueta );

/* Libera los recursos usados por un buffer circular */
bool bufferCircularLibera( bufferCircular_t* pBuffer );

/* Introduce un nuevo elemento en la cabeza de un buffer circular */
bool bufferCircularMete( bufferCircular_t* pBuffer, double valor );

/* Retira un elemento por la cola */
bool bufferCircularSaca( bufferCircular_t* pBuffer, double* pValor );

/* Comprueba si está lleno */
bool bufferCircularLleno( bufferCircular_t* pBuffer );

/* Comprueba si está vacio */
bool bufferCircularVacio( bufferCircular_t* pBuffer );

#endif