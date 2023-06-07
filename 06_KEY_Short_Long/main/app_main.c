/*************************************************************************
 * 作者：		路过人间
 * 邮箱：		cnicfhnui@126.com
 * QQ：			125745255
 * 淘宝店铺：	https://hellobug.taobao.com/
 * 博客地址：	https://hellobug.blog.csdn.net/
 * 在线教程：	https://hellobug.blog.csdn.net/category_10350929.html
 * ESP_IDF版本：V4.4.3
 * 例程说明简介：	
				按键长按短按触发示例
**************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "time.h"
#include "../build/config/sdkconfig.h"


static const char *TAG = "KEY_Short_Long_Demo";


typedef unsigned int	uint32_t;

// 定义按键短按长按事件枚举
typedef enum {
	KEY_SHORT_PRESS = 1, 
	KEY_LONG_PRESS,
} alink_key_t;

// 指定按键IO为IO0
#define KEY_GPIO			GPIO_NUM_0


// 创建队列的句柄
static xQueueHandle gpio_evt_queue = NULL;

// GPIO中断处理函数
void IRAM_ATTR gpio_isr_handler(void *arg) {
	uint32_t gpio_num = (uint32_t) arg;
	xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// 按键GPIO初始化
void KeyInit(uint32_t key_gpio_pin) {

	//配置GPIO结构体
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_ANYEDGE;		// 下降沿和上升沿触发中断
	io_conf.pin_bit_mask = 1 << key_gpio_pin;	// 设置GPIO号
	io_conf.mode = GPIO_MODE_INPUT;				// 模式输入
	io_conf.pull_up_en = GPIO_PULLUP_ENABLE;	// 端口上拉使能
	gpio_config(&io_conf);

	// 设置GPIO中断类型
	gpio_set_intr_type(key_gpio_pin, GPIO_INTR_ANYEDGE);// 下降沿和上升沿触发中断
	// 初始化GPIO事件队列
	gpio_evt_queue = xQueueCreate(2, sizeof(uint32_t));
	// 安装GPIO中断服务
	gpio_install_isr_service(0);
	// 添加GPIO中断事件回调函数
	gpio_isr_handler_add(key_gpio_pin, gpio_isr_handler, (void *) key_gpio_pin);
}

// 按键扫描函数，任务中调用
esp_err_t alink_key_scan(TickType_t ticks_to_wait) 
{
	uint32_t io_num;
	BaseType_t press_key = pdFALSE;
	BaseType_t release_key = pdFALSE;
	int backup_time = 0;
	while (1) {
		// 接收从消息队列发来的消息
		xQueueReceive(gpio_evt_queue, &io_num, ticks_to_wait);
		if (gpio_get_level(io_num) == 0) {//当前低电平，记录下用户按下按键的时间点
			press_key = pdTRUE;
			backup_time = esp_timer_get_time();
			//如果当前GPIO口的电平已经记录为按下，则开始减去上次按下按键的时间点
		} else if (press_key) {
			//记录抬升时间点
			release_key = pdTRUE;
			backup_time = esp_timer_get_time() - backup_time;
		}
		//近当按下标志位和按键弹起标志位都为1时候，才执行回调
		if (press_key & release_key) {
			press_key = pdFALSE;
			release_key = pdFALSE;
			//如果大于1s则回调长按，否则就短按回调
			if (backup_time > 1000000) {
				return KEY_LONG_PRESS;
			} else {
				return KEY_SHORT_PRESS;
			}
		}
	}
}

// 按键中断任务
void key_trigger_Task(void *arg) {
	esp_err_t ret = 0;
	KeyInit(KEY_GPIO);			// 初始化按键IO和中断服务等
	while (1) {
		ret = alink_key_scan(portMAX_DELAY);
		if (ret == -1)
			vTaskDelete(NULL);
		switch (ret) {
		case KEY_SHORT_PRESS:
			ESP_LOGI(TAG, "------------>短按触发");
			break;
		case KEY_LONG_PRESS:
			ESP_LOGI(TAG, "------------>长按触发");
			break;
		default:
			break;
		}
	}
	vTaskDelete(NULL);
}
// 主函数
void app_main() {
	ESP_LOGI(TAG, "APP Is Start!~\r\n");
	ESP_LOGI(TAG, "IDF Version is %d.%d.%d",ESP_IDF_VERSION_MAJOR,ESP_IDF_VERSION_MINOR,ESP_IDF_VERSION_PATCH);
	ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
	ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

	xTaskCreate(key_trigger_Task, "key_trigger_Task", 1024 * 2, NULL, 10, NULL);
}
