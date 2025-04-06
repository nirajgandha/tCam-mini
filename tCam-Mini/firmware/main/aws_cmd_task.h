/*
 * Network Command Task
 *
 * Implement the command processing module for use when either the
 * Ethernet or WiFi interfaces are active.  Enable mDNS for device
 * discovery.
 *
 * Copyright 2020-2022 Dan Julio
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
bool aws_cmd_connected();
esp_websocket_client_handle_t aws_cmd_get_ws_handle();

#endif /* AWS_CMD_TASK_H */