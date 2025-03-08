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
 *     Microsoft         - Initial version
 *     Frédéric Desbiens - 2024 version.
 *     Howard Ginsburg   - Moved application logic to App.c.  This file should not need to be changed
 *                       - anymore.  It is only here to create the ThreadX application thread.
 */

#include <stdio.h>
#include "tx_api.h"
#include "board_init.h"
#include "cmsis_utils.h"
#include "app.h"

#define APP_THREAD_STACK_SIZE 4096
#define APP_THREAD_PRIORITY   4

TX_THREAD app_thread;
ULONG app_thread_stack[APP_THREAD_STACK_SIZE / sizeof(ULONG)];

void tx_application_define(void* first_unused_memory)
{
    systick_interval_set(TX_TIMER_TICKS_PER_SECOND);

    // Create ThreadX thread
    UINT status = tx_thread_create(&app_thread,
        "Application ThreadX Thread",
        app_thread_entry,
        0,
        app_thread_stack,
        APP_THREAD_STACK_SIZE,
        APP_THREAD_PRIORITY,
        APP_THREAD_PRIORITY,
        TX_NO_TIME_SLICE,
        TX_AUTO_START);

    if (status != TX_SUCCESS)
    {
        printf("ERROR: Application ThreadX thread creation failed\r\n");
    }
}

int main(void)
{
    // Initialize the board
    board_init();

    // Enter the ThreadX kernel
    tx_kernel_enter();

    return 0;
}
