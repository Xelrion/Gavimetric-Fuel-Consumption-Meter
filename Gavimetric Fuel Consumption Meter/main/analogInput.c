/* Implmeentación del */
/* Módulo para gestionar la lectura de señales analógicas bajo exclusión mutua */
/* Está basado en el ejemplo oneshot_read_main de ESP_IDF */

/* 
Los pasos para poder hacer una lectura analógica son:
1. Inicialización del ADC
2. Configuración
3. Calibración
4. Lecturas
Los pasos 1. a 3. se realizan sólo al principio.
El paso 4. se reliza cada vez que se necesita una medida.
Si se usan varios canales de un convertidor, hay que revisar los tiempos de guarda
Entre lecturas de canales no obtener lecturas falseadas por cargas remanentes en
el selector de canal (multiplexor) de entrada al ADC
*/

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_adc/adc_oneshot.h"

#define LOG_LOCAL_LEVEL ESP_LOG_NONE
#include "esp_log.h"

#include "bufferCircular.h"
#include "myTaskConfig.h"
#include "analogInput.h"

// static const char* TAG = "analogInput"; // Etiqueta del módulo para el sistema de LOG

/* se gestionan dos canales analógicos aunque sólo se usa uno */
/* para mostrar la configuracón de dos canales en un ADC */

int adc_raw[2]; // Lectura 'en crudo' de los dos canales gestionados 
int voltage[2]; // Conversión a milivoltios de la lectura en crudo

/* Estructuras para manejar el ADC y su configuración */
adc_oneshot_unit_handle_t   adc1_handle;
adc_oneshot_unit_init_cfg_t adc1_config = {
    .unit_id = ADC_UNIT_1,
};

/* Estructuras para manejar la configuración concreta de cada canal.
   como en este caso ambos canales tienen la misma configuración,
   se usa una sola estructura */
adc_oneshot_chan_cfg_t config_potenciometro = {
    .bitwidth = ADC_BITWIDTH_DEFAULT,
    .atten = ATENUACION_POTENCIOMETRO,
};

adc_oneshot_chan_cfg_t config_presion = {
    .bitwidth = ADC_BITWIDTH_DEFAULT,
    .atten = ATENUACION_PRESION,
};

/* Estructuras para manejar la calibración de cada canal */
adc_cali_handle_t calibracion_potenciometro = NULL;
adc_cali_handle_t calibracion_presion = NULL;

/* Funciones propias para calibrar cada canal */
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void adc_calibration_deinit(adc_cali_handle_t handle);

/* Perapra la información necesaria para pasarle a la tarea de lecturas */
void tareaLecturaSet(tareaLecturaInfo_t* pTaskInfo, bufferCircular_t* pLecturas)
{
    pTaskInfo->pLecturas = pLecturas;
}

/* Punto de entrada de la tarea de lectura de entradas analógicas */
void tareaLectura(void* pParametros)
{
    /* Estructuras para intercambio de información */
    /* Entre la tarea y la aplicación principal */
    taskConfig_t*       pConfig = ((taskInfo_t *)pParametros)->pConfig;
    void*                 pData = ((taskInfo_t *)pParametros)->pData;
    bufferCircular_t*  pMedidas = ((tareaLecturaInfo_t*)pData)->pLecturas;

    ESP_LOGI(pConfig->tag, "Periodo de planificación: %lu ms", pConfig->periodo);
    ESP_LOGI(pConfig->tag, "Número inicial de activaciones: %lu", pConfig->numActivaciones);

    /* Prepara el convertidor */
    
    /* Inicializa el convertiodr */
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc1_config, &adc1_handle));

    /* Configura cada canal */
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, CANAL_POTENCIOMETRO, &config_potenciometro));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, CANAL_PRESION, &config_presion));

    /* Calibra cada canal */
    bool do_calibracion_potenciometro = adc_calibration_init(ADC_UNIT_1, CANAL_POTENCIOMETRO, ATENUACION_POTENCIOMETRO, &calibracion_potenciometro);
    bool do_calibracion_presion       = adc_calibration_init(ADC_UNIT_1, CANAL_PRESION, ATENUACION_PRESION, &calibracion_presion);


    /* Prepara la tarea */
    const TickType_t periodo = (pConfig->periodo) / portTICK_PERIOD_MS;
    TickType_t activacionPrevia = xTaskGetTickCount();

    /* Bucle de toma de lecturas */

    bool continuar = true;
    while( continuar )
    {
        /* Espera a la siguiente activación */  
        xTaskDelayUntil(&activacionPrevia, periodo);

        pConfig->numActivaciones++;
        ESP_LOGD(pConfig->tag, "Numero de activaciones: %lu", pConfig->numActivaciones);

        /* Lectura del valor del potenciometro */
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CANAL_POTENCIOMETRO, &adc_raw[0]));
        ESP_LOGD(pConfig->tag, "ADC%d CANAL[%d] Raw Data: %d", ADC_UNIT_1 + 1, CANAL_POTENCIOMETRO, adc_raw[0]);
        if (do_calibracion_potenciometro) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(calibracion_potenciometro, adc_raw[0], &voltage[0]));
            ESP_LOGI(pConfig->tag, "ADC%d CANAL[%d] Voltaje: %d mV", ADC_UNIT_1 + 1, CANAL_POTENCIOMETRO, voltage[0]);
        }

        /* Lectura del valor del sensor de presion */
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CANAL_PRESION, &adc_raw[1]));
        ESP_LOGD(pConfig->tag, "ADC%d CANAL[%d] Raw Data: %d", ADC_UNIT_1 + 1, CANAL_PRESION, adc_raw[1]);
        if (do_calibracion_presion) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(calibracion_presion, adc_raw[1], &voltage[1]));
            ESP_LOGD(pConfig->tag, "ADC%d CANAL[%d] Voltaje: %d mV", ADC_UNIT_1 + 1, CANAL_PRESION, voltage[1]);
        }

        /* Incluye el valor leido del potenciómetro en el buffer de lecturas */
        if (!bufferCircularMete(pMedidas, (double)voltage[0]))
        {
            continuar = false;
        };
        
    }

    /* Si la tarea termina, hay que liberar los recursos usados */
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
    if (do_calibracion_potenciometro) {
        adc_calibration_deinit(calibracion_potenciometro);
    }
    if (do_calibracion_presion) {
        adc_calibration_deinit(calibracion_presion);
    }

}

/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    const char* TAG = "adc_calibration_init";
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

static void adc_calibration_deinit(adc_cali_handle_t handle)
{
    const char* TAG = "adc_calibration_deinit";

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}
