/* Módulo para gestionar el informe de resultados bajo exclusión mutua */
#ifndef CONTROLDEPOSITO_H
#define CONTROLDEPOSITO_H

// Nota: incluir ficheros necesarios de otros módulos

#include "bufferCircular.h"

/* Información que necesita la tarea informe para funcionar */
typedef struct _tareaInformeInfo
{
    bufferCircular_t* pMedias;
} tareaControlDepositoInfo_t;

/* Punto de entrada a la tarea */
void tareaControlDeposito(void* pvParametros);

/* Configuración de la tarea de informe */
void tareaControlDepositoSet(tareaControlDepositoInfo_t* pTaskInfo, bufferCircular_t* pMedias);

#endif