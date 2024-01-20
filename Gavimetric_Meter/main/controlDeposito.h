/* Módulo para gestionar el control del depósito */
#ifndef CONTROLDEPOSITO_H
#define CONTROLDEPOSITO_H

#include "estadoSistema.h"
#include "paradaEmergencia.h"

/* Información que necesita la tarea de control del depósito para funcionar */
typedef struct _tareaControlDepositoInfo
{
    estadoSistema_t* pEstadoSist;
    paradaEmergencia_t* pEmergencia;
} tareaControlDepositoInfo_t;

/* Parámetros de entrada necesarios para actualizar la máquina de estados */
typedef struct _tareaControlDepositoParams
{
    bool emergencia;
    estadoSistemaNivel_t nivelDeposito;
    estadoSistemaDeposito_t estadoDeposito;
    estadoSistema_t* estadoSist;
    bool* continuar;

} tareaControlDepositoParams_t;

/* Punto de entrada a la tarea */
void tareaControlDeposito(void* pvParametros);

/* Configuración de la tarea de informe */
void tareaControlDepositoSet(tareaControlDepositoInfo_t* pTaskInfo, estadoSistema_t* pEstadoSist, paradaEmergencia_t* pEmergencia);

#endif