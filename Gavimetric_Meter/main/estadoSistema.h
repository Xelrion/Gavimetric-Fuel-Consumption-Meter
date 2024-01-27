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

/* Aviso de límite de nivel del depósito */
typedef enum _estadoSistemaNivel
{
    NIVEL_NORMAL,
    NIVEL_MAXIMO,
    NIVEL_MINIMO

} estadoSistemaNivel_t;

/* Estado del depósito */
typedef enum _estadoSistemaDeposito
{
    DEPOSITO_NORMAL,
    DEPOSITO_LLENADO,
    DEPOSITO_VACIADO

} estadoSistemaDeposito_t;

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

    /* Aviso de límite de nivel del depósito */
    estadoSistemaNivel_t nivelDeposito;

    /* Estado del depósito */
    estadoSistemaDeposito_t estadoDeposito;

    /* Estado de señal de petición de medidas remotas */
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

/* Lee el aviso de límite de nivel del depósito */
bool estadoSistemaLeerNivel( estadoSistema_t* pEstadoSist, estadoSistemaNivel_t* pNivel );

/* Lee el estado actual del depósito */
bool estadoSistemaLeerDeposito( estadoSistema_t* pEstadoSist, estadoSistemaDeposito_t* pDeposito );

/* Lee el estado de la petición de medidas remotas */
bool estadoSistemaLeerPeticion( estadoSistema_t* pEstadoSist );

/* Modifica el estado del comando */
bool estadoSistemaEscribirComando( estadoSistema_t* pEstadoSist, estadoSistemaComando_t pComando );

/* Modifica el estado de la espera de estabilización */
bool estadoSistemaEscribirEspera( estadoSistema_t* pEstadoSist, estadoSistemaEspera_t pEspera );

/* Modifica el aviso de límite de nivel del depósito */
bool estadoSistemaEscribirNivel( estadoSistema_t* pEstadoSist, estadoSistemaNivel_t pNivel );

/* Modifica el aviso de límite de nivel del depósito */
bool estadoSistemaEscribirDeposito( estadoSistema_t* pEstadoSist, estadoSistemaDeposito_t pDeposito );

#endif