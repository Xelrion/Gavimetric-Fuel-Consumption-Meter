/* Manejo de una estructura de parada de emergencia */

#ifndef EMERGENCIA_H
#define EMERGENCIA_H

#include <freertos/FreeRTOS.h>
#include "freertos/semphr.h"

/* Codigos de error devueltos por la funciones */
typedef enum _paradaEmergenciaError
{
    EMERGENCIA_OK,
    EMERGENCIA_ERR_MUTEX,

} paradaEmergenciaError_t;

typedef struct _paradaEmergencia
{
    /* Nombre de la estructura */
    const char* tag;

    /* Semáforo para gestionar la exclusión mútua */
    SemaphoreHandle_t mutex;

    /* Indica si la parada de emergencia está o no activa */
    bool paradaEmergencia;

    /* Código de error devuelto por la última operación */
    paradaEmergenciaError_t err;

} paradaEmergencia_t;

/* Crea una estructura de parada de emergencia */
bool paradaEmergenciaCrea( paradaEmergencia_t* pEmergencia, const char* etiqueta );

/* Libera la estructura de parada de emergencia */
bool paradaEmergenciaLibera( paradaEmergencia_t* pEmergencia );

/* Activa la parada de emergencia */
bool paradaEmergenciaActivar( paradaEmergencia_t* pEmergencia );

/* Desactiva la parada de emergencia */
bool paradaEmergenciaDesactivar( paradaEmergencia_t* pEmergencia );

/* Lee el estado de la parada de emergencia */
bool paradaEmergenciaLeer( paradaEmergencia_t* pEmergencia, bool* pEstadoEmergencia );

#endif