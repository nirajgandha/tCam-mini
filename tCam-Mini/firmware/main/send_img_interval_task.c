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

#include "send_img_interval_task.h"
#include "aws_cmd_task.h"
#include "rsp_task.h"
#include "cmd_utilities.h"
#include "json_utilities.h"
#include "esp_log.h"

//
// AWS Network CMD Task variables
//
static const char *TAG = "send_img_interval_cmd_task";

//
// Network CMD Task API
//
void send_img_interval_task()
{

	
	ESP_LOGI(TAG, "Start task to send image at interval(mSec): ");
	long interval_in_msec = 60 * 1000;

	const char *json_payload = "{\"cmd\":\"get_image\"}";
    char message[100];

    snprintf(message, sizeof(message), "%c{\"cmd\":\"%s\"}%c", CMD_JSON_STRING_START, CMD_GET_IMAGE_S, CMD_JSON_STRING_STOP);

	while (1)
	{
		if (!(aws_cmd_connected()))
		{
			vTaskDelay(pdMS_TO_TICKS(1000));
			continue;
		}
		if (is_stream_on())
		{
			ESP_LOGI(TAG, "Stream on so setting set_process_image: true");
			set_process_image(true);
		} else {
			ESP_LOGI(TAG, "Stream off so setting sending cmd get_image");
			push_rx_data(message, sizeof(message), TAG);
			process_rx_data();
			
		}
		
		vTaskDelay(pdMS_TO_TICKS(interval_in_msec));
	}
}
