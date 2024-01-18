/* Módulo para gestionar la toma de medidas de la báscula y comprobar si superan el nivel máximo o mínimo */
#ifndef MEDIDASNIVEL_H
#define MEDIDASNIVEL_H

#include "bufferCircular.h"
#include "estadoSistema.h"
#include "configSistema.h"
#include "paradaEmergencia.h"

/* Información que necesita la tarea de toma de medidas para funcionar */
typedef struct _tareaMedidasNivelInfo
{
    bufferCircular_t* pMedidas;
    estadoSistema_t* pEstadoSist;
    configSistema_t* pConfigSist;
    paradaEmergencia_t* pEmergencia;
} tareaMedidasNivelInfo_t;

/* Punto de entrada a la tarea */
void tareaMedidasNivel(void* pvParametros);

/* Configuración de la tarea de toma de medidas */
void tareaMedidasNivelSet(tareaMedidasNivelInfo_t* pTaskInfo, bufferCircular_t* pMedidas, estadoSistema_t* pEstadoSist, configSistema_t* pConfigSist, paradaEmergencia_t* pEmergencia);
#endif
