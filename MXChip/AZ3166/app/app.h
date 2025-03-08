/* 
 * Copyright (c) Microsoft
 * Copyright (c) 2024 Eclipse Foundation
 * 
 *  This program and the accompanying materials are made available 
 *  under the terms of the MIT license which is available at
 *  https://opensource.org/license/mit.
 * 
 *  SPDX-License-Identifier: MIT
 * 
 *  Contributors: 
 *     Howard Ginsburg   - Creation of separate application files from main class.
 */

#ifndef _APP_H
#define _APP_H
   
#include "nx_api.h"
#include "nxd_dns.h"
#include "tx_api.h"

VOID app_thread_entry(ULONG parameter);

#endif