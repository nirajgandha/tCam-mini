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
#include "sif_utilities.h"

//
// AWS Network CMD Task variables
//
static const char *TAG = "send_img_interval_cmd_task";

//
// Network CMD Task API
//
void send_img_interval_task()
{

	long interval_in_msec_send_image = 30 * 1000;
	ESP_LOGI(TAG, "Start task to trigger cmd on tx/rx port at interval(mSec): %ld", interval_in_msec_send_image);

	char message[100];
	bool toggle1 = false;
	bool toggle2 = false;

	while (1)
	{
		bool should_send = false;
		if (!(check_if_aws_fully_connected()))
		{
			vTaskDelay(pdMS_TO_TICKS(1000));
			continue;
		}
		memset(message, 0, sizeof(message));
		if (!is_stream_on() && !toggle1)
		{
			toggle1 = true;
			ESP_LOGW(TAG, "stream off so sending get_image cmd to sif");
			snprintf(message, sizeof(message), "%c{\"cmd\":\"%s\"}%c", CMD_JSON_STRING_START, CMD_GET_IMAGE_S, CMD_JSON_STRING_STOP);
			should_send = true;
			goto send_message;
		}
		if (!is_stream_on() && toggle1)
		{
			toggle1 = false;
			ESP_LOGW(TAG, "stream off so sending stream_on cmd to sif");
			snprintf(message, sizeof(message), "%c{\"cmd\":\"%s\", \"args\":{\"delay_msec\":0,\"num_frames\":0}}%c", CMD_JSON_STRING_START, CMD_STREAM_ON_S, CMD_JSON_STRING_STOP);
			should_send = true;
			goto send_message;
		}
		if (is_stream_on() && !toggle2)
		{
			toggle2 = true;
			ESP_LOGW(TAG, "stream on so sending get_image cmd to sif");
			snprintf(message, sizeof(message), "%c{\"cmd\":\"%s\"}%c", CMD_JSON_STRING_START, CMD_GET_IMAGE_S, CMD_JSON_STRING_STOP);
			should_send = true;
			goto send_message;
		}
		if (is_stream_on() && toggle2)
		{
			toggle2 = false;
			ESP_LOGW(TAG, "stream on so sending stream_off cmd to sif");
			snprintf(message, sizeof(message), "%c{\"cmd\":\"%s\"}%c", CMD_JSON_STRING_START, CMD_STREAM_OFF_S, CMD_JSON_STRING_STOP);
			should_send = true;
			goto send_message;
		}
	send_message:
		if (should_send)
		{
			sif_send(message, sizeof(message));
		}
		vTaskDelay(pdMS_TO_TICKS(interval_in_msec_send_image));
	}
}
