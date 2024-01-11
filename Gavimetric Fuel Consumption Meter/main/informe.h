/* Módulo para gestionar el informe de resultados bajo exclusión mutua */
#ifndef INFORME_H
#define INFORME_H

#include "bufferCircular.h"

/* Información que necesita la tarea informe para funcionar */
typedef struct _tareaInformeInfo
{
    bufferCircular_t* pMedias;
} tareaInformeInfo_t;

/* Punto de entrada a la tarea */
void tareaInforme(void* pvParametros);

/* Configuración de la tarea de informe */
void tareaInformeSet(tareaInformeInfo_t* pTaskInfo, bufferCircular_t* pMedias);

#endif
