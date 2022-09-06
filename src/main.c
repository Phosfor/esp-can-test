/* TWAI Network Listen Only Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
 * The following example demonstrates a Listen Only node in a TWAI network. The
 * Listen Only node will not take part in any TWAI bus activity (no acknowledgments
 * and no error frames). This example will execute multiple iterations, with each
 * iteration the Listen Only node will do the following:
 * 1) Listen for ping and ping response
 * 2) Listen for start command
 * 3) Listen for data messages
 * 4) Listen for stop and stop response
 */
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"

/* --------------------- Definitions and static variables ------------------ */
// Example Configuration
#define NO_OF_ITERS 3
#define RX_TASK_PRIO 9
#define TX_GPIO_NUM GPIO_NUM_25
#define RX_GPIO_NUM GPIO_NUM_26
#define EXAMPLE_TAG "TWAI Listen Only"

#define ID_MASTER_STOP_CMD 0x0A0
#define ID_MASTER_START_CMD 0x0A1
#define ID_MASTER_PING 0x0A2
#define ID_SLAVE_STOP_RESP 0x0B0
#define ID_SLAVE_DATA 0x0B1
#define ID_SLAVE_PING_RESP 0x0B2

static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
// Set TX queue length to 0 due to listen only mode
static const twai_general_config_t g_config = {.mode = TWAI_MODE_NORMAL,
                                               .tx_io = TX_GPIO_NUM,
                                               .rx_io = RX_GPIO_NUM,
                                               .clkout_io = TWAI_IO_UNUSED,
                                               .bus_off_io = TWAI_IO_UNUSED,
                                               .tx_queue_len = 5,
                                               .rx_queue_len = 5,
                                               .alerts_enabled = TWAI_ALERT_NONE,
                                               .clkout_divider = 0};

/* --------------------------- Tasks and Functions -------------------------- */


void app_main(void)
{
    // Install and start TWAI driver
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(EXAMPLE_TAG, "Driver installed");
    ESP_ERROR_CHECK(twai_start());
    ESP_LOGI(EXAMPLE_TAG, "Driver started");

    
    while (true)
    { // iterations < NO_OF_ITERS) {
        twai_message_t msg;
        msg.identifier = 0x003;
        msg.data_length_code = 8;
        msg.rtr = true;
        esp_err_t err = twai_receive(&msg, 1000);
        ESP_LOGI(EXAMPLE_TAG, "RX %u", err);
        if (err == ESP_OK)
        {
            ESP_LOGI(EXAMPLE_TAG, "Received msg 0x%03x", msg.identifier);
            for(int i = 0; i < msg.data_length_code; i++) {
                printf("%02x, ", msg.data[i]);
            }
            printf("\n");
        }

        msg.identifier = 0x003;
        msg.data_length_code = 8;
        msg.rtr = true;
        ESP_LOGI(EXAMPLE_TAG, "TX %u", twai_transmit(&msg, portMAX_DELAY));

        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Stop and uninstall TWAI driver
    ESP_ERROR_CHECK(twai_stop());
    ESP_LOGI(EXAMPLE_TAG, "Driver stopped");
    ESP_ERROR_CHECK(twai_driver_uninstall());
    ESP_LOGI(EXAMPLE_TAG, "Driver uninstalled");
}