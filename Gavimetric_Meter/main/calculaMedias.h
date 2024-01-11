/* Módulo para gestionar el cálculo de medias bajo exclusión mutua */
#ifndef CALCULAMEDIAS_H
#define CALCULAMEDIAS_H

#include "bufferCircular.h"

/* Información que necesita la tarea de cálculo de medias para funcionar */
typedef struct _tareaMediaInfo
{
    bufferCircular_t* pLecturas;
    bufferCircular_t* pMedias;
} tareaMediaInfo_t;

/* Punto de entrada a la tarea */
void tareaMedia(void* pvParametros);

/* Configuración de la tarea de lectura */
void tareaMediaSet(tareaMediaInfo_t* pTaskInfo, bufferCircular_t* pLecturas, bufferCircular_t* pMedias);

#endif
