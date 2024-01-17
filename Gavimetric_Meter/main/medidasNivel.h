/* Módulo para gestionar la toma de medidas de la báscula y comprobar si superan el nivel máximo o mínimo */
#ifndef MEDIDASNIVEL_H
#define MEDIDASNIVEL_H

#include "bufferCircular.h"

/* Información que necesita la tarea de lecturas para funcionar */
typedef struct _tareaMedidasNivelInfo
{
    bufferCircular_t* pMedidas;
} tareaMedidasNivelInfo_t;

/* Punto de entrada a la tarea */
void tareaMedidasNivel(void* pvParametros);

/* Configuración de la tarea de lectura */
void tareaMedidasNivelSet(tareaMedidasNivelInfo_t* pTaskInfo, bufferCircular_t* pMedidas);
#endif
