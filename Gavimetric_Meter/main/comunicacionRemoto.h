/* Módulo para gestionar la lectura comunicación con el sistema remoto */
#ifndef SISTEMAREMOTO_H
#define SISTEMAREMOTO_H

#include "bufferCircular.h"
#include "estadoSistema.h"
#include "configSistema.h"
#include "paradaEmergencia.h"

/* Información que necesita la tarea de toma de medidas para funcionar */
typedef struct _tareaComunicacionRemotoInfo
{
    bufferCircular_t* pConsumoRemoto;
    estadoSistema_t* pEstadoSist;
    configSistema_t* pConfigSist;
    paradaEmergencia_t* pEmergencia;
} tareaComunicacionRemotoInfo_t;

/* Punto de entrada a la tarea */
void tareaComunicacionRemoto(void* pvParametros);

/* Configuración de la tarea de toma de medidas */
void tareaComunicacionRemotoSet(tareaComunicacionRemotoInfo_t* pTaskInfo, bufferCircular_t* pConsumoRemoto, estadoSistema_t* pEstadoSist, configSistema_t* pConfigSist, paradaEmergencia_t* pEmergencia);

#endif