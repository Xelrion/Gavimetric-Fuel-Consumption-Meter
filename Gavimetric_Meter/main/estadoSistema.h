/* Manejo de una estructura de estado del sistema */

#ifndef ESTSIST_H
#define ESTSIST_H

#include <freertos/FreeRTOS.h>
#include "freertos/semphr.h"


/* Codigos de error devueltos por la funciones */
typedef enum _estadoSistemaError
{
    EST_SIST_OK,
    EST_SIST_ERR_MUTEX,
    EST_SIST_ERR_LLENO,
    EST_SIST_ERR_VACIO

} estadoSistemaError_t;

/* Estado del último comando recibido desde la consola */
typedef enum _estadoSistemaComando
{
    MEDIDA_MANUAL_PUNTUAL,
    MEDIDA_MANUAL_CONTINUADA,
    MEDIDA_REMOTO,
    MEDIDA_OFF,

} estadoSistemaComando_t;

/* Estado de la espera de estabilización */
typedef enum _estadoSistemaEspera
{
    DESACTIVADA,
    INICIADA,
    EN_CURSO,

} estadoSistemaEspera_t;

typedef struct _estadoSistema
{
    /* Nombre de la estructura */
    const char* tag;

    /* Semáforo para gestionar la exclusión mútua */
    SemaphoreHandle_t mutex;

    /* Estado del último comando recibido desde la consola */
    estadoSistemaComando_t comando;

    /* Estado de la espera de estabilización */
    estadoSistemaEspera_t esperaEstabilizacion;

    /* Estado de señal de petición de medidas remotas*/
    bool peticionMedidas;

    /* Código de error devuelto por la última operación */
    estadoSistemaError_t err;

} estadoSistema_t;

/* Crea una estructura de estado del sistema */
bool estadoSistemaCrea( estadoSistema_t* pEstadoSist, const char* etiqueta );

/* Libera la estructura de estado actual del sistema */
bool estadoSistemaLibera( estadoSistema_t* pEstadoSist );

/* Lee el estado del comando */
bool estadoSistemaLeerComando( estadoSistema_t* pEstadoSist, estadoSistemaComando_t* pComando );

/* Lee el estado de la espera de estabilización */
bool estadoSistemaLeerEspera( estadoSistema_t* pEstadoSist, estadoSistemaEspera_t* pEspera );

/* Lee el estado de la petición de medidas remotas */
bool estadoSistemaLeerPeticion( estadoSistema_t* pEstadoSist, bool* pPeticion );

/* Modifica el estado del comando */
bool estadoSistemaEscribirComando( estadoSistema_t* pEstadoSist, estadoSistemaComando_t* pComando );

/* Modifica el estado de la espera de estabilización */
bool estadoSistemaEscribirEspera( estadoSistema_t* pEstadoSist, estadoSistemaEspera_t* pEspera );

/* Comprueba si la toma de medidas se puede o no hacer actualmente */
bool estadoSistemaMedidasActivas( estadoSistema_t* pEstadoSist, bool* pMedidaActiva );

#endif