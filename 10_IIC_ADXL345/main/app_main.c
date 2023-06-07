/*************************************************************************
 * 作者：		路过人间
 * 邮箱：		cnicfhnui@126.com
 * QQ：			125745255
 * 淘宝店铺：	https://hellobug.taobao.com/
 * 博客地址：	https://hellobug.blog.csdn.net/
 * 在线教程：	https://hellobug.blog.csdn.net/category_10350929.html
 * ESP_IDF版本：V4.4.3
 * 例程说明简介：	
				硬件IIC测试三轴加速度传感器
**************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "../build/config/sdkconfig.h"

#include "adxl345.h"

#define SCL_PIN		GPIO_NUM_21			// 硬件IIC 时钟引脚
#define SDA_PIN		GPIO_NUM_22			// 硬件IIC 数据引脚

static const char *TAG = "ADXL345_Demo";

static void ADXL345_Task(void *pvParameters)
{
	uint8_t ret = 0;
	short x,y,z,angx,angy,angz;
	ESP_LOGI(TAG,"ADXL345_Task Started\n");
	ret = Init_ADXL345(SDA_PIN,SCL_PIN);			// 初始化IIC和ADXL345
	ESP_LOGI(TAG,"Init_ADXL355: %d\r\n",ret);
	ESP_LOGI(TAG,"Analog Devices ID = 0x%02X  \r\n",GetDevicesID());	// 永远是0xE5
	ADXL345_AUTO_Adjust((char*)&x,(char*)&y,(char*)&z);					// 自动校准

	while (1) {
		//得到X,Y,Z轴的加速度值(原始值)
		ADXL345_Read_Average(&x,&y,&z,10);	//读取X,Y,Z三个方向的加速度值 
		//ESP_LOGI(TAG,"X=%d,   Y=%d,   Z=%d\n",x,y,z);//显示加速度原始值
 		//得到角度值,并显示
		angx = ADXL345_Get_Angle(x,y,z,1);
		angy = ADXL345_Get_Angle(x,y,z,2);
		angz = ADXL345_Get_Angle(x,y,z,0);
		ESP_LOGI(TAG,"X= %hd.%hd   Y=%hd.%hd   Z=%hd.%hd\r\n",angx/10,angx%10,angy/10,angy%10,angz/10,angz%10);
		vTaskDelay(100 / portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}

void app_main() {
	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_LOGI(TAG, "[APP] APP Is Start!~\r\n");
	ESP_LOGI(TAG, "[APP] IDF Version is %d.%d.%d",ESP_IDF_VERSION_MAJOR,ESP_IDF_VERSION_MINOR,ESP_IDF_VERSION_PATCH);
	ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
	ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
	
	xTaskCreate(&ADXL345_Task,	//pvTaskCode
			"ADXL345_Task",//pcName
			4096,//usStackDepth
			NULL,//pvParameters
			4,//uxPriority
			NULL//pxCreatedTask
			);
}

