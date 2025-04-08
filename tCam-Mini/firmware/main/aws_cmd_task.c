/*
 * Network Command Task
 *
 * Implement the command processing module for use when either the
 * Ethernet or WiFi interfaces are active.  Enable mDNS for device
 * discovery.
 *
 * Copyright 2020-2022 Dan Julio
 *
 * This file is part of tCam.
 * Created by Niraj Gandha
 *
 * tCam is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tCam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tCam.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "aws_cmd_task.h"
#include "ctrl_task.h"
#include "cmd_utilities.h"
#include "net_utilities.h"
#include "system_config.h"
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//
// AWS Network CMD Task variables
//
static const char *TAG = "aws_cmd_task";

// WebSocket client handle
static esp_websocket_client_handle_t ws_client = NULL;

// Connected status
static bool connected = false;


static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
	esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
	switch (event_id)
	{
	case WEBSOCKET_EVENT_CONNECTED:
		ESP_LOGI(TAG, "WebSocket CONNECTED");
		connected = true;
		init_command_processor();
		break;

	case WEBSOCKET_EVENT_DISCONNECTED:
		ESP_LOGW(TAG, "WebSocket DISCONNECTED");
		connected = false;
		break;

	case WEBSOCKET_EVENT_DATA:
		if ((char *)data->op_code == 0x1)
		{
			ESP_LOGE(TAG,"WEBSOCKET_EVENT_DATA_RECEIVED: %s", (char *)data->data_ptr);
			push_rx_data((char *)data->data_ptr, data->data_len, TAG);
			while (process_rx_data()){}
		}
		break;
	case WEBSOCKET_EVENT_ERROR:
		ESP_LOGE(TAG, "WebSocket ERROR");
		break;

	default:
		break;
	}
}

//
// Network CMD Task API
//
void aws_cmd_task()
{

	ESP_LOGI(TAG, "Start task as client socket");

	// Wait until the network interface is connected
	if (!(*net_is_connected)())
	{
		vTaskDelay(pdMS_TO_TICKS(500));
	}

	// Init WebSocket
	esp_websocket_client_config_t websocket_cfg = {
		.host = "192.168.2.187",
		.port = 8390,
		.path = "/mlai/streaming/ws/stream",
		.transport = WEBSOCKET_TRANSPORT_OVER_TCP, // Use TCP (ws://)
		.disable_auto_reconnect = false,		   // Enable automatic reconnect
		.ping_interval_sec = 30,				   // Send pings every 30 seconds
		.keep_alive_enable = true,				   // Enable TCP keep-alive
		.keep_alive_idle = 10,					   // Idle time before sending keep-alive
		.keep_alive_interval = 5,				   // Interval between probes
		.keep_alive_count = 100,				   // Retry count
	};

	ws_client = esp_websocket_client_init(&websocket_cfg);
	esp_websocket_register_events(ws_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, NULL);

    esp_err_t aws_connection_start_code = -1;
    while (1)
    {
       aws_connection_start_code = esp_websocket_client_start(ws_client);
       if (aws_connection_start_code != ESP_OK)
       {
            ESP_LOGE(TAG, "Error in connecting to websocket: %s", esp_err_to_name(aws_connection_start_code));
            vTaskDelay(pdMS_TO_TICKS(1000));
       }
       else
       {
            ESP_LOGI(TAG, "Connected to aws socket");
            break;
       }
        
    };
	// Monitor connection
    while (1) {
        if (!connected) {
            ESP_LOGW(TAG, "Waiting to (re)connect...");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * True when connected to a client
 */
bool aws_cmd_connected()
{
	return connected;
}

// Optional cleaner access
esp_websocket_client_handle_t aws_cmd_get_ws_handle()
{
	return ws_client;
}
