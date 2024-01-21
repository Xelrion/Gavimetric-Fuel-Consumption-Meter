/* Módulo para gestionar el cálculo de consumo del depósito */
#ifndef CALCULACONSUMO_H
#define CALCULACONSUMO_H

#include "bufferCircular.h"
#include "estadoSistema.h"
#include "configSistema.h"

/* Información que necesita la tarea de cálculo de medias para funcionar */
typedef struct _tareaConsumoInfo
{
    bufferCircular_t* pMedidas;
    bufferCircular_t* pConsumoRemoto;
    bufferCircular_t* pConsumoConsola;
    estadoSistema_t* pEstadoSist;
    configSistema_t* pConfigSist;
} tareaConsumoInfo_t;

/* Punto de entrada a la tarea */
void tareaConsumo(void* pvParametros);

/* Configuración de la tarea de lectura */
void tareaConsumoSet(tareaConsumoInfo_t* pTaskInfo, bufferCircular_t* pMedidas, bufferCircular_t* pConsumoRemoto, bufferCircular_t* pConsumoConsola, estadoSistema_t* pEstadoSist, configSistema_t* pConfigSist);

#endif
