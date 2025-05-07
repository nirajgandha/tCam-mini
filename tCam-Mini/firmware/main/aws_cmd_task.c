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
#include "esp_transport_ws.h"
#include "json_utilities.h"
#include "ps_utilities.h"
#include "sif_utilities.h"

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
		break;

	case WEBSOCKET_EVENT_DISCONNECTED:
		ESP_LOGW(TAG, "WebSocket DISCONNECTED");
		connected = false;
		break;

	case WEBSOCKET_EVENT_DATA:
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
	char message[100];
	while (1)
	{
		onboarding_details_t *onboarding_details_info;
		ps_get_onboarding_details_info(&onboarding_details_info);
		if (!onboarding_details_info->update_required)
		{
			break;
		}
		memset(message, 0, sizeof(message));
		ESP_LOGW(TAG, "requires update in onboarding details, so sending get_onboarding_details cmd to esp32");
		snprintf(message, sizeof(message), "%c{\"cmd\":\"%s\"}%c", CMD_JSON_STRING_START, CMD_GET_ONBOARD_DETAILS_FROM_ESP32, CMD_JSON_STRING_STOP);
		sif_send(message, sizeof(message));
		vTaskDelay(pdMS_TO_TICKS(500));
	}

	while (1)
	{
		// Wait until the network interface is connected
		if (!(*net_is_connected)())
		{
			vTaskDelay(pdMS_TO_TICKS(500));
			continue;
		}
		if (!ws_client)
		{
			esp_websocket_client_config_t config = get_aws_client_config("4264", "13.126.143.157", 8390);
			init_aws_client(&config);
			start_aws_connection();
			vTaskDelay(pdMS_TO_TICKS(10000));
		}
		else
		{
			if (ws_client && !esp_websocket_client_is_connected(ws_client))
			{
				ESP_LOGE(TAG, "close websocket connection");
				esp_websocket_client_close(ws_client, pdMS_TO_TICKS(100));
				ESP_LOGE(TAG, "destroy websocket connection and free all resources");
				esp_websocket_client_destroy(ws_client);
				ws_client = NULL;
			}
		}
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

// Optional cleaner access
esp_websocket_client_handle_t aws_cmd_get_ws_handle()
{
	return ws_client;
}

/**
 * True when connected to a aws server and connection is intact
 */
bool check_if_aws_fully_connected()
{
	return connected && ws_client && esp_websocket_client_is_connected(ws_client);
}

void init_aws_client(esp_websocket_client_config_t *config)
{
	ESP_LOGE(TAG, "init_aws_client on %s:%d%s", config->host, config->port, config->path);
	ws_client = esp_websocket_client_init(config);
	esp_websocket_register_events(ws_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, NULL);
}

esp_websocket_client_config_t get_aws_client_config(char *serialNumber, char *host, int port)
{
	static char path[100]; // Use a static buffer to avoid dynamic memory allocation issues
	snprintf(path, sizeof(path), "/mlai/streaming/ws/stream_thermal/%s", serialNumber);
	esp_websocket_client_config_t websocket_cfg = {
		.host = host,
		.port = port,
		.path = path,
		.transport = WEBSOCKET_TRANSPORT_OVER_TCP, // Use TCP (ws://)
		.disable_auto_reconnect = false,		   // Enable automatic reconnect
		.ping_interval_sec = 30,				   // Send pings every 30 seconds
		.keep_alive_enable = true,				   // Enable TCP keep-alive
		.keep_alive_idle = 10,					   // Idle time before sending keep-alive
		.keep_alive_interval = 5,				   // Interval between probes
		.keep_alive_count = 100,				   // Retry count
	};
	return websocket_cfg;
}

void start_aws_connection()
{
	ESP_LOGE(TAG, "start_aws_connection");
	esp_err_t aws_connection_start_code = -1;
	while (1)
	{
		aws_connection_start_code = esp_websocket_client_start(ws_client);
		if (aws_connection_start_code != ESP_OK)
		{
			ESP_LOGE(TAG, "Error in connecting to websocket: %s", esp_err_to_name(aws_connection_start_code));
			vTaskDelay(pdMS_TO_TICKS(2000));
		}
		else
		{
			ESP_LOGI(TAG, "Connected to aws socket");
			break;
		}
	};
}