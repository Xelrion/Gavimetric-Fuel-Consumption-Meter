/* Manejo de una estructura de configuración del sistema */

#ifndef CONFIGSIST_H
#define CONFIGSIST_H

#include <freertos/FreeRTOS.h>
#include "freertos/semphr.h"

/* Codigos de error devueltos por la funciones */
typedef enum _configSistemaError
{
    CONFIG_SIST_OK,
    CONFIG_SIST_ERR_MUTEX,

} configSistemaError_t;

typedef struct _configSistema
{
    /* Nombre de la estructura */
    const char* tag;

    /* Semáforo para gestionar la exclusión mútua */
    SemaphoreHandle_t mutex;

    /* Periodo de medida en ms */
    int periodoMedida;

    /* Tiempo de estabilización del depósito en s */
    int esperaEstabilizacion;

    /* Medida de consumo máximo del depósito */
    double consumoMaximo;

    /* Medida de nivel máximo del depósito */
    double nivelMaximo;

    /* Medida de nivel mínimo del depósito */
    double nivelMinimo;

    /* Código de error devuelto por la última operación */
    configSistemaError_t err;

} configSistema_t;

/* Crea una estructura de configuración del sistema */
bool configSistemaCrea( configSistema_t* pConfigSist, const char* etiqueta );

/* Libera la estructura de configuración del sistema */
bool configSistemaLibera( configSistema_t* pConfigSist );

/* Lee el periodo de medidas */
bool configSistemaLeerPeriodo( configSistema_t* pConfigSist, double* pPeriodoMedida );

/* Lee la espera de estabilización */
bool configSistemaLeerEspera( configSistema_t* pConfigSist, int* pEspera );

/* Lee el consumo máximo de combustible */
bool configSistemaLeerConsumoMax( configSistema_t* pConfigSist, int* pConsumoMax );

/* Lee el nivel máximo de combustible */
bool configSistemaLeerNivelMax( configSistema_t* pConfigSist, int* pNivelMax );

/* Lee el nivel mínimo de combustible */
bool configSistemaLeerNivelMax( configSistema_t* pConfigSist, int* pNivelMin );

/* Indica si una medida supera alguno de los niveles límite de combustible */
bool configSistemaComprobarNivel( configSistema_t* pConfigSist, double pMedida, bool* pNivelMin, bool* pNivelMax );

#endif