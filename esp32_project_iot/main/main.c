/**
 * Application entry point.
 */

#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi_app.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
//static const char TAG[] = "main";

static const int RX_BUF_SIZE = 1024;

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)

#define UART_PORT UART_NUM_2

//void wifi_application_connected_events(void)
//{
//	ESP_LOGI(TAG, "WiFi Application Connected!!");
//	sntp_time_sync_task_start();
//	aws_iot_start();
//}
void init_uart(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_2, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void app_main(void)
{
	init_uart();
    // Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Start Wifi
	wifi_app_start();

//	// Configure Wifi reset button
//	wifi_reset_button_config();
//
//	// Start DHT22 Sensor task
//	DHT22_task_start();
//
//	// Set connected event callback
//	wifi_app_set_callback(&wifi_application_connected_events);
}

