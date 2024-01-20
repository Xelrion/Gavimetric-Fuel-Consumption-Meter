/* Módulo para gestionar el control del depósito */
#ifndef CONTROLDEPOSITO_H
#define CONTROLDEPOSITO_H

#include "estadoSistema.h"

/* Información que necesita la tarea de control del depósito para funcionar */
typedef struct _tareaControlDepositoInfo
{
    estadoSistema_t* pEstadoSist;
} tareaControlDepositoInfo_t;

/* Punto de entrada a la tarea */
void tareaControlDeposito(void* pvParametros);

/* Configuración de la tarea de informe */
void tareaControlDepositoSet(tareaControlDepositoInfo_t* pTaskInfo, estadoSistema_t* pEstadoSist);

#endif