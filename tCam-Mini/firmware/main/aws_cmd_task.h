/*
 * AWS Command Task
 *
 *
 * Created by Niraj Gandha
 *
 * This file is part of tCam.
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
#ifndef AWS_CMD_TASK_H
#define AWS_CMD_TASK_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_websocket_client.h"

//
// AWS Network CMD Task API
//
void aws_cmd_task();
esp_websocket_client_config_t get_aws_client_config(char *serialNumber, char *host, int port);
esp_websocket_client_handle_t aws_cmd_get_ws_handle();
void init_aws_client(esp_websocket_client_config_t *config);
void start_aws_connection();
bool check_if_aws_fully_connected();

#endif /* AWS_CMD_TASK_H */