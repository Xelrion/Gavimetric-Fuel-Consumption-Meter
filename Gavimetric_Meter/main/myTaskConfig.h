/* Macros y estructuras útiles para manejar tareas */
#ifndef MYTASKCONFIG_H
#define MYTASKCONFIG_H

#include <freertos/FreeRTOS.h>

// Estructura para la info que se necesita para configurar una tarea
typedef struct _taskConfig
{
    uint32_t periodo;         // Periodo entre activaciones
    uint32_t numActivaciones; // Numero de activaciones hasta ahora
    uint8_t  activa;          // Para indicar si la tarea está en el planificador
    const char* tag;          // etiqueta de la tarea
} taskConfig_t;

// Estructura para intercambio de información con una tarea
typedef struct _taskInfo
{
    taskConfig_t* pConfig;  // Configuración de la tarea
    void* pData;           // Información para el proceso de la tarea. Es distinto para cada tarea
} taskInfo_t;


// Configura la activación de una tarea periódica
void taskConfigSet(taskConfig_t* pTaskConfig, uint32_t periodo, const char* tag);

// Monta la estructura de paso de información de una tarea
void taskInfoSet(taskInfo_t* pTaskInfo, taskConfig_t* pTaskConfig, void* pTaskData);

#endif