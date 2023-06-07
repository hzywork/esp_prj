/*************************************************************************
 * 作者：		路过人间
 * 邮箱：		cnicfhnui@126.com
 * QQ：			125745255
 * 淘宝店铺：	https://hellobug.taobao.com/
 * 博客地址：	https://hellobug.blog.csdn.net/
 * 在线教程：	https://hellobug.blog.csdn.net/category_10350929.html
 * ESP_IDF版本：V4.4.3
 * 例程说明简介：	串口调试输出测试，打印芯片的一些信息
**************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "../build/config/sdkconfig.h"
#include "esp_log.h"

static const char *TAG = "HelloBug";

void app_main()
{
	uint8_t MAC[6];
	ESP_LOGI(TAG, "[APP] Start!~\r\n");
	ESP_LOGI(TAG, "[APP] IDF Version is %d.%d.%d",ESP_IDF_VERSION_MAJOR,ESP_IDF_VERSION_MINOR,ESP_IDF_VERSION_PATCH);
	ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
	// 打印芯片信息
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
	ESP_LOGI(TAG, "ESP32 Chip Cores Count:  %d",chip_info.cores);
	if(chip_info.model == 1){
		ESP_LOGI(TAG, "ESP32 Chip Model is:  ESP32");
	}else if(chip_info.model == 2){
		ESP_LOGI(TAG, "ESP32 Chip Model is:  ESP32S2");
	}else{
		ESP_LOGI(TAG, "ESP32 Chip Model is:  Unknown Model");
	}
	ESP_LOGI(TAG, "ESP32 Chip Features is:  %d",chip_info.features);
	ESP_LOGI(TAG, "ESP32 Chip Revision is:  %d",chip_info.revision);

	ESP_LOGI(TAG, "ESP32 Chip, WiFi%s%s, ",
			(chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
			(chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

	ESP_LOGI(TAG, "SPI Flash Chip Size: %dMByte %s Flash", spi_flash_get_chip_size() / (1024 * 1024),
			(chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "Embedded" : "External");

	ESP_LOGI(TAG, "Free Heap Size is:  %d Byte",esp_get_free_heap_size());
	ESP_LOGI(TAG, "Free Internal Heap Size is:  %d Byte",esp_get_free_internal_heap_size());
	ESP_LOGI(TAG, "Free minimum Heap Size is:  %d Byte",esp_get_minimum_free_heap_size());

	esp_efuse_mac_get_default(MAC);
	ESP_LOGI(TAG, "MAC Address:");
	ESP_LOG_BUFFER_HEX(TAG, MAC,6);
	
	// 打印10次退出重启芯片
	for (int i = 10; i >= 0; i--) {
		ESP_LOGI(TAG, "Hello Bug! ^_^			Restarting in %d seconds...", i);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	printf("Restarting now.\n");
	esp_restart();					// 重启芯片
}
