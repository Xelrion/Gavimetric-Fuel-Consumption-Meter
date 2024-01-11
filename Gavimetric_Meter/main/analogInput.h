/* Módulo para gestionar la lectura de señales analógicas bajo exclusión mutua */
#ifndef ANALOGREAD_H
#define ANALOGREAD_H

#include "hal/adc_types.h"

#include "bufferCircular.h"

#define CANAL_POTENCIOMETRO         ADC_CHANNEL_0   // GPIO36
#define ATENUACION_POTENCIOMETRO    ADC_ATTEN_DB_11

#define CANAL_PRESION               ADC_CHANNEL_1   // GPIO37
#define ATENUACION_PRESION          ADC_ATTEN_DB_11

/* Información que necesita la tarea de lecturas para funcionar */
typedef struct _tareaLecturaInfo
{
    bufferCircular_t* pLecturas;
} tareaLecturaInfo_t;

/* Punto de entrada a la tarea */
void tareaLectura(void* pvParametros);

/* Configuración de la tarea de lectura */
void tareaLecturaSet(tareaLecturaInfo_t* pTaskInfo, bufferCircular_t* pLecturas);
#endif
