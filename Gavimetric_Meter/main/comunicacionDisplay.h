/* Módulo para gestionar la lectura comunicación con el display */
#ifndef COMUNICACIONDISPLAY_H
#define COMUNICACIONDISPLAY_H

#include "bufferCircular.h"
#include "estadoSistema.h"
#include "paradaEmergencia.h"

/* Información que necesita la tarea de toma de medidas para funcionar */
typedef struct _tareaComunicacionDisplayInfo
{
    bufferCircular_t* pConsumoConsola;
    estadoSistema_t* pEstadoSist;
    paradaEmergencia_t* pEmergencia;
} tareaComunicacionDisplayInfo_t;

/* Punto de entrada a la tarea */
void tareaComunicacionDisplay(void* pvParametros);

/* Configuración de la tarea de toma de medidas */
void tareaComunicacionDisplaySet(tareaComunicacionDisplayInfo_t* pTaskInfo, bufferCircular_t* pConsumoConsola, estadoSistema_t* pEstadoSist, paradaEmergencia_t* pEmergencia);

#endif