/*************************************************************************
 * 作者：		路过人间
 * 邮箱：		cnicfhnui@126.com
 * QQ：			125745255
 * 淘宝店铺：	https://hellobug.taobao.com/
 * 博客地址：	https://hellobug.blog.csdn.net/
 * 在线教程：	https://hellobug.blog.csdn.net/category_10350929.html
 * ESP_IDF版本：V4.4.3
 * 例程说明简介：	
				使用LEDC控制器实现LED PWM实验
				代码分为两部分：单通道版本（开发板）和 多通道版本
				单通道版本适用于此开发板
				默认多通道版本是注释掉的，可以用小核心板来测试
**************************************************************************/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "../build/config/sdkconfig.h"

//////////////////////////////////////////////////////////////
/////////////////////单通道版本（开发板）////////////////////////
//////////////////////////////////////////////////////////////
#define LEDC_HS_CH0_GPIO		4
#define LEDC_HS_CH0_CHANNEL		LEDC_CHANNEL_0

#define LEDC_TEST_DUTY			8000
static const char *TAG = "LEDC_PWM_Demo";

void app_main() 
{
	ESP_LOGI(TAG,"APP Is Start!~\r\n");
	ESP_LOGI(TAG,"IDF Version is %d.%d.%d",ESP_IDF_VERSION_MAJOR,ESP_IDF_VERSION_MINOR,ESP_IDF_VERSION_PATCH);
	ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
	ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
	// 初始化LEDC定时器结构体
	ledc_timer_config_t ledc_timer = { 
		.duty_resolution = LEDC_TIMER_13_BIT,	// PWM 占空比分辨率
		.freq_hz = 5000,						// PWM 信号频率
		.speed_mode = LEDC_HIGH_SPEED_MODE,		// 高速模式
		.timer_num = LEDC_TIMER_0				// 使用定时器0
	};

	// 设置LEDC定时器配置
	ledc_timer_config(&ledc_timer);

	// LEDC通道配置
	ledc_channel_config_t ledc_channel = {
			.channel =	LEDC_HS_CH0_CHANNEL,
			.duty = 0, 
			.gpio_num = LEDC_HS_CH0_GPIO,
			.speed_mode = LEDC_HIGH_SPEED_MODE,
			.timer_sel = LEDC_TIMER_0 
	};
	// 将前面的的配置设置到 LED 控制器
	ledc_channel_config(&ledc_channel);
	// 初始化淡入淡出服务
	ledc_fade_func_install(0);

	// 循环将LED变亮变暗
	while (1) {
		printf("PWM to max %d\n", LEDC_TEST_DUTY);
		ledc_set_fade_with_time(ledc_channel.speed_mode,ledc_channel.channel, LEDC_TEST_DUTY,3000);
		ledc_fade_start(ledc_channel.speed_mode,ledc_channel.channel, LEDC_FADE_NO_WAIT);

		vTaskDelay(3000 / portTICK_PERIOD_MS);

		printf("PWM to min 0\n");
		ledc_set_fade_with_time(ledc_channel.speed_mode,ledc_channel.channel,0, 3000);
		ledc_fade_start(ledc_channel.speed_mode,ledc_channel.channel, LEDC_FADE_NO_WAIT);
		vTaskDelay(3000 / portTICK_PERIOD_MS);
	}
}

//////////////////////////////////////////////////////////////
//////////////////////////多通道版本////////////////////////////
//////////////////////////////////////////////////////////////

/*




#define  LED_TOTAL_NUM			2			// LED数量

#define LEDC_HS_CH0_GPIO		12
#define LEDC_HS_CH0_CHANNEL		LEDC_CHANNEL_0
#define LEDC_HS_CH1_GPIO		19
#define LEDC_HS_CH1_CHANNEL		LEDC_CHANNEL_1


#define LEDC_TEST_DUTY			8000

void app_main() 
{
	int ch;
	// 初始化LEDC定时器结构体
	ledc_timer_config_t ledc_timer = { 
		.duty_resolution = LEDC_TIMER_13_BIT,	// PWM 占空比分辨率
		.freq_hz = 5000,						// PWM 信号频率
		.speed_mode = LEDC_HIGH_SPEED_MODE,		// 高速模式
		.timer_num = LEDC_TIMER_0				// 使用定时器0
	};

	// 设置LEDC定时器配置
	ledc_timer_config(&ledc_timer);

	// 
	ledc_channel_config_t ledc_channel[LED_TOTAL_NUM] = { 
		{ 
			.channel =	LEDC_HS_CH0_CHANNEL, 
			.duty = 0, 
			.gpio_num = LEDC_HS_CH0_GPIO, 
			.speed_mode = LEDC_HIGH_SPEED_MODE, 
			.timer_sel = LEDC_TIMER_0 
		},
		{ 
			.channel = LEDC_HS_CH1_CHANNEL, 
			.duty = 0, 
			.gpio_num = LEDC_HS_CH1_GPIO,
			.speed_mode = LEDC_HIGH_SPEED_MODE, 
			.timer_sel = LEDC_TIMER_0 
		},
	};
	// 将前面的的配置设置到 LED 控制器
	for (ch = 0; ch < LED_TOTAL_NUM; ch++) {
		ledc_channel_config(&ledc_channel[ch]);
	}
	// 初始化淡入淡出服务
	ledc_fade_func_install(0);

	// 循环将LED变亮变暗
	while (1) {
		printf("PWM to max %d\n", LEDC_TEST_DUTY);
		for (ch = 0; ch < LED_TOTAL_NUM; ch++) {
			ledc_set_fade_with_time(ledc_channel[ch].speed_mode,ledc_channel[ch].channel, LEDC_TEST_DUTY,3000);
			ledc_fade_start(ledc_channel[ch].speed_mode,ledc_channel[ch].channel, LEDC_FADE_NO_WAIT);
		}
		vTaskDelay(3000 / portTICK_PERIOD_MS);

		printf("PWM to min 0\n");
		for (ch = 0; ch < LED_TOTAL_NUM; ch++) {
			ledc_set_fade_with_time(ledc_channel[ch].speed_mode,ledc_channel[ch].channel,0, 3000);
			ledc_fade_start(ledc_channel[ch].speed_mode,ledc_channel[ch].channel, LEDC_FADE_NO_WAIT);
		}
		vTaskDelay(3000 / portTICK_PERIOD_MS);
	}
}

*/