/*************************************************************************
 * 作者：		路过人间
 * 邮箱：		cnicfhnui@126.com
 * QQ：			125745255
 * 淘宝店铺：	https://hellobug.taobao.com/
 * 博客地址：	https://hellobug.blog.csdn.net/
 * 在线教程：	https://hellobug.blog.csdn.net/category_10350929.html
 * ESP_IDF版本：V4.4.3
 * 例程说明简介：	
				IO中断示例
**************************************************************************/
 
#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "../build/config/sdkconfig.h"

static const char *TAG = "GPIO_Interrupt_Demo";

//宏定义一个GPIO口用于GPIO口的输入输出
#define Interrupt_GPIO  		GPIO_NUM_0

static xQueueHandle gpio_evt_queue = NULL; //定义一个队列返回变量


// 中断回调函数
void IRAM_ATTR gpio_isr_handler(void* arg) {
	//把中断消息插入到队列的后面，将gpio的io参数传递到队列中
	uint32_t gpio_num = (uint32_t) arg;
	xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

//低电平触发的回调方法
void gpio_low_interrupt_callBack(void* arg) {
	printf(" \r\n into gpio_low_interrupt_callBack ...\r\n  ");
	uint32_t io_num;
	while (1) {
		//不断读取gpio队列，读取完后将删除队列
		if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
			printf("GPIO[%d] Interrupt GPIO Level: %d\n", io_num,gpio_get_level(io_num));
		}
	}
}

void app_main() 
{
	ESP_LOGI(TAG, "APP Is Start!~\r\n");
	ESP_LOGI(TAG, "IDF Version is %d.%d.%d",ESP_IDF_VERSION_MAJOR,ESP_IDF_VERSION_MINOR,ESP_IDF_VERSION_PATCH);
	ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
	ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

	//GPIO口结构体定义
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_NEGEDGE;//下降沿触发
	io_conf.mode = GPIO_MODE_INPUT;//选择为输出模式
	io_conf.pin_bit_mask = Interrupt_GPIO;//配置GPIO_OUT寄存器
	io_conf.pull_down_en = 0;//设置下拉使能
	io_conf.pull_up_en = 1;//设置上拉使能
	gpio_config(&io_conf);//配置使能
    // 设置GPIO中断类型
	gpio_set_intr_type(Interrupt_GPIO, GPIO_INTR_NEGEDGE);// 下降沿触发中断
	gpio_install_isr_service(0);//注册中断服务
	//设置GPIO的中断回调函数
	gpio_isr_handler_add(Interrupt_GPIO, gpio_isr_handler,(void*) Interrupt_GPIO);
	//创建一个消息队列，从中获取队列句柄
	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

	xTaskCreate(gpio_low_interrupt_callBack //任务函数
			, "gpio_task_example" //任务名字
			, 2048  //任务堆栈大小
			, NULL  //传递给任务函数的参数
			, 10   //任务优先级
			, NULL); //任務句柄
}
