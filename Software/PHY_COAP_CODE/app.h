/***************************************************************************/ /**
                                                                               * @file app.h
                                                                               * @brief Application code
                                                                               *******************************************************************************
                                                                               * # License
                                                                               * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
                                                                               *******************************************************************************
                                                                               *
                                                                               * SPDX-License-Identifier: Zlib
                                                                               *
                                                                               * The licensor of this software is Silicon Laboratories Inc.
                                                                               *
                                                                               * This software is provided 'as-is', without any express or implied
                                                                               * warranty. In no event will the authors be held liable for any damages
                                                                               * arising from the use of this software.
                                                                               *
                                                                               * Permission is granted to anyone to use this software for any purpose,
                                                                               * including commercial applications, and to alter it and redistribute it
                                                                               * freely, subject to the following restrictions:
                                                                               *
                                                                               * 1. The origin of this software must not be misrepresented; you must not
                                                                               *    claim that you wrote the original software. If you use this software
                                                                               *    in a product, an acknowledgment in the product documentation would be
                                                                               *    appreciated but is not required.
                                                                               * 2. Altered source versions must be plainly marked as such, and must not be
                                                                               *    misrepresented as being the original software.
                                                                               * 3. This notice may not be removed or altered from any source distribution.
                                                                               *
                                                                               ******************************************************************************/

#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C"
{
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_component_catalog.h"
#include "sl_wisun_types.h"
#include <stdint.h>
#include <stdbool.h>
    // -----------------------------------------------------------------------------
    //                              Macros and Typedefs
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    //                                Global Variables
    // -----------------------------------------------------------------------------
    // CoAP-related global variables
    extern char chip[];
    extern char application[];
    extern char version[];
    extern char device_tag[];
    extern char parent_tag[];
    extern char device_type[];
    extern char history_string[];
    extern char crash_info_string[];
    extern bool send_asap;
    extern sl_wisun_mac_address_t parent_mac;

    // Statistics and timing variables
    extern uint16_t connection_count;
    extern uint16_t network_connection_count;
    extern uint64_t connect_time_sec;
    extern uint64_t connection_time_sec;
    extern uint64_t disconnection_time_sec;
    extern uint64_t connected_total_sec;
    extern uint64_t disconnected_total_sec;
    extern uint64_t app_join_state_delay_sec[];
    extern uint64_t msg_count;
    extern uint8_t trace_level;

    // -----------------------------------------------------------------------------
    //                          Public Function Declarations
    // -----------------------------------------------------------------------------
    /**************************************************************************/ /**
                                                                                  * @brief Application task function
                                                                                  * @details This function is the main app task implementation
                                                                                  * @param[in] args arguments
                                                                                  *****************************************************************************/
    void app_task(void *args);
    void app_reset_statistics(void);
    void refresh_parent_tag(void);

#ifdef __cplusplus
}
#endif

#endif // APP_H
