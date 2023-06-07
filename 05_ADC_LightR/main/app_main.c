/*************************************************************************
 * 作者：		路过人间
 * 邮箱：		cnicfhnui@126.com
 * QQ：			125745255
 * 淘宝店铺：	https://hellobug.taobao.com/
 * 博客地址：	https://hellobug.blog.csdn.net/
 * 在线教程：	https://blog.csdn.net/cnicfhnui/article/details/108465952
 * ESP_IDF版本：V4.4.3
 * 例程说明简介：	
 * ADC示例
 *		 通过ADC1通道6读取光照强度传感器的值
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include "../build/config/sdkconfig.h"

static const char *TAG = "ADC_LightR_Demo";

typedef unsigned int	uint32_t;

// ADC所接的通道  GPIO34 if ADC1  = ADC1_CHANNEL_6
#define ADC1_TEST_CHANNEL		ADC1_CHANNEL_6 
// ADC斜率曲线
static esp_adc_cal_characteristics_t *adc_chars;
// 参考电压
#define DEFAULT_VREF				3300			//Use adc2_vref_to_gpio() to obtain a better estimate

// ADC初始化
void adc_Init()
{
	adc1_config_width(ADC_WIDTH_BIT_12);// 12位分辨率
	adc1_config_channel_atten(ADC1_TEST_CHANNEL, ADC_ATTEN_DB_11);// 电压输入衰减
	adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));	// 为斜率曲线分配内存
	esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
}

// 用户函数入口，相当于main函数
void app_main()
{
	uint32_t read_raw;
	ESP_LOGI(TAG, "APP Is Start!~\r\n");
	ESP_LOGI(TAG, "IDF Version is %d.%d.%d",ESP_IDF_VERSION_MAJOR,ESP_IDF_VERSION_MINOR,ESP_IDF_VERSION_PATCH);
	ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
	ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
	adc_Init();
	while(1){
		read_raw = adc1_get_raw(ADC1_TEST_CHANNEL);// 采集ADC原始值
		uint32_t voltage = esp_adc_cal_raw_to_voltage(read_raw, adc_chars);//通过一条斜率曲线把读取adc1_get_raw()的原始数值转变成了mV
		printf("ADC原始值: %d   转换电压值: %dmV\n", read_raw, voltage);
		vTaskDelay(500 / portTICK_RATE_MS);
	}
}