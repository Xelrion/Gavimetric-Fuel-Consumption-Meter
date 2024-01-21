/* Módulo para gestionar la lectura de comandos de la consola */
#ifndef COMANDOSCONSOLA_H
#define COMANDOSCONSOLA_H

#include "estadoSistema.h"
#include "configSistema.h"
#include "paradaEmergencia.h"

/* Lista de comandos de consola */
typedef enum _comandosConsola
{
    CONMUTAR_ESTADO,
    DETENER_PARADA_EMERGENCIA,
    INICIAR_MEDIDA_CONTINUADA,
    DETENER_MEDIDA_CONTINUADA,
    INICIAR_LLENADO,
    DETENER_LLENADO,
    INICIAR_VACIADO,
    DETENER_VACIADO,
    CONFIG_PERIODO_MEDIDA,
    CONFIG_TIEMPO_ESTAB,
    CONFIG_CONSUMO_MAXIMO,
    CONFIG_NIVEL_MINIMO,
    CONFIG_NIVEL_MAXIMO

} comandosConsola_t;

/* Información que necesita la tarea de toma de medidas para funcionar */
typedef struct _tareaComandosConsolaInfo
{
    estadoSistema_t* pEstadoSist;
    configSistema_t* pConfigSist;
    paradaEmergencia_t* pEmergencia;
} tareaComandosConsolaInfo_t;

/* Punto de entrada a la tarea */
void tareaComandosConsola(void* pvParametros);

/* Configuración de la tarea de toma de medidas */
void tareaComandosConsolaSet(tareaComandosConsolaInfo_t* pTaskInfo, estadoSistema_t* pEstadoSist, configSistema_t* pConfigSist, paradaEmergencia_t* pEmergencia);
#endif
