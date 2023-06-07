/*************************************************************************
 * 作者：		路过人间
 * 邮箱：		cnicfhnui@126.com
 * QQ：			125745255
 * 微信：		18559979152
 * 淘宝店铺：	https://hellobug.taobao.com/
 * 博客地址：	https://hellobug.blog.csdn.net/
 * 在线教程：	https://hellobug.blog.csdn.net/category_10350929.html
 * ESP_IDF版本：V4.4.3
 * 例程说明简介：	
				使用RTOS任务来运行LED闪烁
**************************************************************************/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "../build/config/sdkconfig.h"
#include "esp_log.h"

#define LED_GPIO		4					// LED引脚GPIO
#define TAG				"LED_Task_Demo"		// LOG输出标头

// LED闪烁任务
void LED_Task(void *pvParameter)
{
	gpio_pad_select_gpio(LED_GPIO);
	// 设置GPIO为推挽输出模式
	gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
	while(1) {
		// GPIO输出低
		ESP_LOGI(TAG,"Turning off the LED\n");
		gpio_set_level(LED_GPIO, 0);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		// GPIO输出高
		ESP_LOGI(TAG,"Turning on the LED\n");
		gpio_set_level(LED_GPIO, 1);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	
}
// 主函数
void app_main(void)
{
	ESP_LOGI(TAG,"APP Is Start!~\r\n");
	ESP_LOGI(TAG,"IDF Version is %d.%d.%d",ESP_IDF_VERSION_MAJOR,ESP_IDF_VERSION_MINOR,ESP_IDF_VERSION_PATCH);
	ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
	ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

	// 创建一个RTOS任务
	xTaskCreate(
		&LED_Task,		// 任务函数指针
		"LED_Task",		// 任务名称
		2048,			// 任务的堆栈大小
		NULL,			// 任务参数的指针
		5,				// 任务运行时的优先级
		NULL			// 传递任务的句柄
		);
}
