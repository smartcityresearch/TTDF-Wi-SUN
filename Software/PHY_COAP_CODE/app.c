/***************************************************************************/ /**
                                                                               * @file app.c
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
// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmsis_os2.h"
#include "app.h"

#include "sl_assert.h"
#include "sl_string.h"
#include "sl_memory_manager.h"

#include "sl_wisun_api.h"
#include "sl_wisun_types.h"
#include "sl_wisun_version.h"
#include "sl_wisun_trace_util.h"
#include "sl_wisun_crash_handler.h"
#include "sl_wisun_event_mgr.h"
#include "sl_wisun_config.h"
#include "sl_wisun_coap.h"
#include "sl_wisun_coap_config.h"

#include "app_parameters.h"
#include "app_coap.h"
#include "app_check_neighbors.h"
#include "app_timestamp.h"
#include "app_rtt_traces.h"
#include "modbusmaster.h"
#include "sl_iostream.h"
#include "sl_iostream_handles.h"
#include "sl_iostream_init_eusart_instances.h"
#include "sl_iostream_uart.h"
#include "em_gpio.h"

#ifdef SL_CATALOG_SIMPLE_BUTTON_PRESENT
#include "sl_simple_button_instances.h"
#endif

#include "sl_sleeptimer.h"
#include "sl_simple_led_instances.h"

#ifdef SL_CATALOG_WISUN_APP_CORE_PRESENT
#include "sl_wisun_app_core_util.h"
#endif

#define APP_VERSION_STRING "V1.0"
#define APP_TRACK_HEAP
#define APP_CHECK_PREVIOUS_CRASH

#ifdef APP_CHECK_PREVIOUS_CRASH
#define PREVIOUS_CRASH_FORMAT_STRING ""
#else
#define PREVIOUS_CRASH_FORMAT_STRING ""
#endif

#ifdef APP_TRACK_HEAP
#define TRACK_HEAP_FORMAT_STRING "\"heap_used\":\"%.2f\",\n"
#define TRACK_HEAP_VALUE 1.0 * app_heap_info.used_size / (app_heap_info.total_size / 100),
#else
#define TRACK_HEAP_FORMAT_STRING
#define TRACK_HEAP_VALUE
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// Macros to treat possible errors
#define NO_ERROR(ret, ...)       \
  if (ret == SL_STATUS_OK)       \
  {                              \
    printfBothTime(__VA_ARGS__); \
  }
#define IF_ERROR(ret, ...)   \
  if (ret != SL_STATUS_OK)   \
  {                          \
    printfBothTime("\n");    \
    printfBoth(__VA_ARGS__); \
  }
#define IF_ERROR_RETURN(ret, ...) \
  if (ret != SL_STATUS_OK)        \
  {                               \
    printfBothTime("\n");         \
    printfBoth(__VA_ARGS__);      \
    return ret;                   \
  }
#define IF_ERROR_INCR(ret, error_count, ...) \
  if (ret != SL_STATUS_OK)                   \
  {                                          \
    printfBothTime("\n");                    \
    printfBoth(__VA_ARGS__);                 \
    error_count++;                           \
  }

#define TRACES_WHILE_CONNECTING                                                  \
  app_set_all_traces(SL_WISUN_TRACE_LEVEL_INFO, true);                           \
  app_set_trace(SL_WISUN_TRACE_GROUP_SOCK, SL_WISUN_TRACE_LEVEL_INFO, true);     \
  app_set_trace(SL_WISUN_TRACE_GROUP_BOOT, SL_WISUN_TRACE_LEVEL_DEBUG, true);    \
  app_set_trace(SL_WISUN_TRACE_GROUP_RF, SL_WISUN_TRACE_LEVEL_ERROR, true);      \
  app_set_trace(SL_WISUN_TRACE_GROUP_RPL, SL_WISUN_TRACE_LEVEL_DEBUG, true);     \
  app_set_trace(SL_WISUN_TRACE_GROUP_TIM_SRV, SL_WISUN_TRACE_LEVEL_ERROR, true); \
  app_set_trace(SL_WISUN_TRACE_GROUP_FHSS, SL_WISUN_TRACE_LEVEL_ERROR, true);    \
  app_set_trace(SL_WISUN_TRACE_GROUP_MAC, SL_WISUN_TRACE_LEVEL_WARN, true);      \
  app_set_trace(SL_WISUN_TRACE_GROUP_FSM, SL_WISUN_TRACE_LEVEL_WARN, true);

#define TRACES_WHEN_CONNECTED                                                    \
  app_set_all_traces(SL_WISUN_TRACE_LEVEL_INFO, true);                           \
  app_set_trace(SL_WISUN_TRACE_GROUP_RF, SL_WISUN_TRACE_LEVEL_ERROR, true);      \
  app_set_trace(SL_WISUN_TRACE_GROUP_RPL, SL_WISUN_TRACE_LEVEL_ERROR, true);     \
  app_set_trace(SL_WISUN_TRACE_GROUP_TIM_SRV, SL_WISUN_TRACE_LEVEL_ERROR, true); \
  app_set_trace(SL_WISUN_TRACE_GROUP_FHSS, SL_WISUN_TRACE_LEVEL_ERROR, true);    \
  app_set_trace(SL_WISUN_TRACE_GROUP_MAC, SL_WISUN_TRACE_LEVEL_WARN, true);      \
  app_set_trace(SL_WISUN_TRACE_GROUP_FSM, SL_WISUN_TRACE_LEVEL_WARN, true);

// JSON format strings
#define START_JSON "{\n"
#define END_JSON "}\n"

#define DEVICE_CHIP_ITEMS    \
  device_global_ipv6_string, \
      device_tag,            \
      chip,                  \
      device_type,           \
      device_mac_string

#define DEVICE_CHIP_JSON_FORMAT \
  "\"ipv6\":\"%s\",\n"          \
  "\"device\":\"%s\",\n"        \
  "\"chip\":\"%s\",\n"          \
  "\"type\":\"%s\",\n"          \
  "\"MAC\":\"%s\",\n"

#define PARENT_INFO_ITEMS       \
  parent_tag,                   \
      parent_info.rpl_rank,     \
      parent_info.etx,          \
      parent_info.routing_cost, \
      parent_info.rsl_in,       \
      parent_info.rsl_out

#define PARENT_JSON_FORMAT     \
  "\"parent\":\"%s\",\n"       \
  "\"rpl_rank\":\"%d\",\n"     \
  "\"etx\":\"%d\",\n"          \
  "\"routing_cost\":\"%d\",\n" \
  "\"rsl_in\":\"%d\",\n"       \
  "\"rsl_out\":\"%d\",\n"

#define RUNNING_JSON_FORMAT \
  "\"running\":\"%s\",\n"

#define MSG_COUNT_JSON_FORMAT \
  "\"msg_count\":\"%lld\",\n"

#ifdef _SILICON_LABS_32B_SERIES_2
#ifdef _SILICON_LABS_32B_SERIES_2_CONFIG_8
#define CHIP "FG28"
#endif
#endif

#ifndef CHIP
#define CHIP SL_BOARD_NAME
#endif

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
void _join_state_custom_callback(sl_wisun_evt_t *evt);
void _check_neighbors(void);
char *_connection_json_string(void);
char *_status_json_string(char *start_text);
sl_wisun_mac_address_t _get_parent_mac_address_and_update_parent_info(void);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
uint16_t connection_count = 0;
uint16_t network_connection_count = 0;
uint64_t connect_time_sec;
uint64_t connection_time_sec;
uint64_t disconnection_time_sec;
uint64_t connected_total_sec = 0;
uint64_t disconnected_total_sec = 0;
uint64_t msg_count = 0;
sl_wisun_neighbor_info_t parent_info;
sl_wisun_neighbor_info_t secondary_info;
uint16_t preferred_pan_id = 0xffff;
uint8_t min_join_state = 0;
bool send_asap;

#ifdef APP_TRACK_HEAP
sl_memory_heap_info_t app_heap_info;
bool refresh_heap;
#endif

bool print_keep_alive = true;

char chip[8];
char device_tag[8];
char parent_tag[8];
char secondary_tag[8];
char application[100];
char version[80];
char device_type[25];
uint32_t parent_rsl_in = 0;
uint32_t parent_rsl_out = 0;

bool just_disconnected = false;
char history_string[1024] = "";

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
sl_wisun_join_state_t join_state = SL_WISUN_JOIN_STATE_DISCONNECTED;
static uint64_t app_join_state_sec[6];
uint64_t app_join_state_delay_sec[6];
static uint16_t previous_join_state = 0;
char json_string[SL_WISUN_COAP_RESOURCE_HND_SOCK_BUFF_SIZE];
sl_wisun_mac_address_t device_mac;
sl_wisun_mac_address_t parent_mac;
sl_wisun_mac_address_t secondary_mac;

// IPv6 address structures
in6_addr_t device_global_ipv6;
in6_addr_t border_router_ipv6;

// IPv6 address strings
char device_mac_string[40];
char device_global_ipv6_string[40];
char border_router_ipv6_string[40];

sl_wisun_network_info_t network_info;

uint8_t trace_level = SL_WISUN_TRACE_LEVEL_INFO;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
#ifndef SL_CATALOG_WISUN_EVENT_MGR_PRESENT // Event Manager also defines this handler
/* Wi-SUN event handler */
void sl_wisun_on_event(sl_wisun_evt_t *evt)
{
  (void)evt->header.id;

  /////////////////////////////////////////////////////////////////////////////
  // Put your Wi-SUN event handling here!                                    //
  // This is called from Wi-SUN stack.                                       //
  // Do not call blocking functions from here!                               //
  // Protect your data during event handling!                                //
  /////////////////////////////////////////////////////////////////////////////
}
#endif

/* App task function */
void app_task(void *args)
{
  (void)args;
  uint64_t now = 0;
  uint64_t connection_timestamp;
  uint64_t connected_delay_sec;
  bool print_keep_alive = true;
  bool with_time, to_console, to_rtt;

  app_timestamp_init();
  init_app_parameters();

  printf("\n=== PHY_COAP_CODE Application ===\n");

  with_time = to_console = to_rtt = true;

#ifdef APP_CHECK_PREVIOUS_CRASH
  sl_wisun_check_previous_crash();
  if (strlen(crash_info_string))
  {
    printfBothTime("Info on previous crash: %s\n", crash_info_string);
  }
#endif

  osDelay(1000);
  printf("\n");
  sprintf(chip, "%s", CHIP);
  snprintf(application, 100, "%s %s %s %s", chip, "FG28", "PHY_COAP_CODE", APP_VERSION_STRING);
  printfBothTime("%s/%s %s\n", chip, "FG28", application);
  snprintf(version, 80, "Compiled on %s at %s", __DATE__, __TIME__);
  printfBothTime("%s\n", application);
  printfBothTime("%s\n", version);

  printfBothTime("Network %s\n", WISUN_CONFIG_NETWORK_NAME);
  printfBothTime("network_size %s\n", app_wisun_trace_util_nw_size_to_str(WISUN_CONFIG_NETWORK_SIZE));
#ifdef WISUN_CONFIG_TX_POWER
  printfBothTime("Tx Power %d dBm\n", WISUN_CONFIG_TX_POWER);
#endif

  TRACES_WHILE_CONNECTING;

#ifdef SL_CATALOG_WISUN_COAP_PRESENT
  printfBothTime("With CoAP Support\n");
#endif

  // Set device_tag to last 2 bytes of MAC address
  sl_wisun_get_mac_address(&device_mac);
  sprintf(device_mac_string, "%s", app_wisun_mac_addr_to_str(&device_mac));
  sprintf(device_tag, "%02x%02x", device_mac.address[6], device_mac.address[7]);
  printfBothTime("device MAC %s\n", device_mac_string);
  printfBothTime("device_tag %s\n", device_tag);

  // Set device_type based on application settings
#ifdef WISUN_CONFIG_DEVICE_TYPE
  if (WISUN_CONFIG_DEVICE_TYPE == SL_WISUN_LFN)
  {
    sprintf(device_type, "LFN");
  }
  else if (WISUN_CONFIG_DEVICE_TYPE == SL_WISUN_ROUTER)
  {
#ifdef SL_CATALOG_WISUN_LFN_DEVICE_SUPPORT_PRESENT
    sprintf(device_type, "FFN with LFN support");
#else
    sprintf(device_type, "FFN");
#endif
  }
  printfBothTime("device_type %s (%d)\n", device_type, WISUN_CONFIG_DEVICE_TYPE);
#else
#ifdef SL_CATALOG_WISUN_LFN_DEVICE_SUPPORT_PRESENT
  sprintf(device_type, "FFN with LFN support");
#else
  sprintf(device_type, "FFN");
#endif
#endif

  printfBothTime("device_type %s\n", device_type);

  // Register our join state custom callback function with the event manager
  app_wisun_em_custom_callback_register(SL_WISUN_MSG_JOIN_STATE_IND_ID, _join_state_custom_callback);

  printfBothTime("app_parameters.auto_send_sec %d\n", app_parameters.auto_send_sec);

  sl_wisun_set_preferred_pan(app_parameters.preferred_pan_id);
  printfBothTime("preferred_pan_id 0x%04x\n", app_parameters.preferred_pan_id);

  // Initialize Modbus Master for energy meter
  printf("\n=== Initializing Modbus Master ===\n");
  GPIO_PinModeSet(gpioPortC, 2, gpioModePushPull, 0);
  printf("GPIO PC02 (PWM) configured as RS485 DE/RE control\n");
  sl_iostream_uart_set_read_block(sl_iostream_uart_exp_handle, false);
  ModbusMaster_setStream(sl_iostream_exp_handle);
  ModbusMaster_begin(1);
  ModbusMaster_setDebug(0);
  printf("=== Modbus Master Initialized ===\n\n");

  // Initialize Si7021 temperature/humidity sensor
  init_si7021_sensor();

  // CRITICAL: Allow I2C bus to fully settle between sensor initializations
  // Si7021 and VEML6035 share the same I2C bus - must prevent contention
  sl_sleeptimer_delay_millisecond(500);

  // Initialize VEML6035 lux sensor (external sensor via I2C)
  init_veml6035_sensor();

  // Store the time where we call sl_wisun_app_core_network_connect()
  connect_time_sec = now_sec();

#ifdef SL_CATALOG_WISUN_APP_CORE_PRESENT
  // connect to the wisun network
  sl_wisun_app_core_network_connect();
#endif

  while (1)
  {
    now = now_sec();
    sl_wisun_get_join_state(&join_state);
    if (join_state == SL_WISUN_JOIN_STATE_OPERATIONAL)
      break;
    if (now % 60 == 0)
    {
      printfTime("Waiting for connection to %s. join_state %d\n",
                 WISUN_CONFIG_NETWORK_NAME, join_state);
    }
    osDelay(1000);
  }

  /*******************************************
  /  We only reach this part once connected  /
  *******************************************/
  connection_timestamp = now_sec();
  // Once connected for the first time, reduce RTT traces to the minimum
  TRACES_WHEN_CONNECTED;

  printfBothTime("Connected to Wi-SUN network!\n");

  // Print initial connection message
  printfBothTime("%s", _connection_json_string());

#ifdef APP_CHECK_PREVIOUS_CRASH
  if (strlen(crash_info_string))
  {
    osDelay(2UL);
    printfBothTime("Previous crash: %s\n", crash_info_string);
  }
#endif

#ifdef APP_TRACK_HEAP
  sl_memory_get_heap_info(&app_heap_info);
  refresh_heap = true;
#endif

  osDelay(2UL);

  while (1)
  {
    ///////////////////////////////////////////////////////////////////////////
    // Put your application code here!                                       //
    ///////////////////////////////////////////////////////////////////////////

    now = now_sec();
    // Use the connection time as reference, in order to spread messages in time
    // when several devices are powered on at the same time
    connected_delay_sec = now - connection_timestamp;

    // We can only send messages outside if connected
    if (join_state == SL_WISUN_JOIN_STATE_OPERATIONAL)
    {
      // Print status message once then disable status
      if ((connected_delay_sec % app_parameters.auto_send_sec == 0) || (send_asap))
      {
        if ((print_keep_alive == true) || (send_asap))
        {
          printfBothTime("%s", _status_json_string(""));
          print_keep_alive = false;
          send_asap = false;
        }
      }
      // Enable status for next time
      if (connected_delay_sec % app_parameters.auto_send_sec == 1)
      {
        if (print_keep_alive == false)
        {
          print_keep_alive = true;
        }
      }
    }

#ifdef APP_TRACK_HEAP
    // Refresh heap info once then disable the refresh
    if (connected_delay_sec % 5 == 0)
    {
      if (refresh_heap)
      {
        sl_memory_get_heap_info(&app_heap_info);
        refresh_heap = false;
      }
    }
    // Enable heap info refresh for next time
    if (connected_delay_sec % 5 == 1)
    {
      refresh_heap = true;
    }
#endif

    osDelay(1000);
    sl_wisun_app_core_util_dispatch_thread();
  }
}

// -----------------------------------------------------------------------------
//                          Public Function Definitions (CoAP support)
// -----------------------------------------------------------------------------
void app_reset_statistics(void)
{
  connection_time_sec = now_sec();
  disconnection_time_sec = connection_time_sec;
  connection_count = 0;
  network_connection_count = 0;
  connected_total_sec = 0;
  disconnected_total_sec = 0;
  memset(app_join_state_delay_sec, 0, sizeof(app_join_state_delay_sec));
}

void refresh_parent_tag(void)
{
  _get_parent_mac_address_and_update_parent_info();
  sprintf(parent_tag, "%02x%02x", parent_mac.address[6], parent_mac.address[7]);
  sprintf(secondary_tag, "%02x%02x", secondary_mac.address[6], secondary_mac.address[7]);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
sl_wisun_mac_address_t _get_parent_mac_address_and_update_parent_info(void)
{
  sl_status_t ret;
  uint8_t neighbor_count;
  uint8_t i;
  sl_wisun_neighbor_info_t neighbor_info;

  // Clear parent and secondary MAC addresses
  for (i = 0; i < SL_WISUN_MAC_ADDRESS_SIZE; i++)
  {
    parent_mac.address[i] = 0;
    secondary_mac.address[i] = 0;
  }

  ret = sl_wisun_get_neighbor_count(&neighbor_count);
  if (ret)
    return parent_mac;

  sl_wisun_mac_address_t neighbor_mac_addresses[neighbor_count];
  ret = sl_wisun_get_neighbors(&neighbor_count, neighbor_mac_addresses);
  if (ret)
    return parent_mac;

  for (i = 0; i < neighbor_count; i++)
  {
    sl_wisun_get_neighbor_info(&neighbor_mac_addresses[i], &neighbor_info);
    if (neighbor_info.type == SL_WISUN_NEIGHBOR_TYPE_PRIMARY_PARENT)
    {
      sl_wisun_get_neighbor_info(&neighbor_mac_addresses[i], &parent_info);
      parent_mac = neighbor_mac_addresses[i];
    }
    if (neighbor_info.type == SL_WISUN_NEIGHBOR_TYPE_SECONDARY_PARENT)
    {
      sl_wisun_get_neighbor_info(&neighbor_mac_addresses[i], &secondary_info);
      secondary_mac = neighbor_mac_addresses[i];
    }
  }
  return parent_mac;
}

void _join_state_custom_callback(sl_wisun_evt_t *evt)
{
  int i;
  uint64_t delay;
  bool with_time, to_console, to_rtt;
  with_time = to_console = to_rtt = true;

  join_state = (sl_wisun_join_state_t)evt->evt.join_state.join_state;
  if (join_state > SL_WISUN_JOIN_STATE_OPERATIONAL)
  {
    // Do not process intermediate join states, whose values are > 5
    return;
  }

  if (join_state != previous_join_state)
  {
    // join_state changed...
    printfBothTime("[Join state %u->%u]\n", previous_join_state, join_state);
    if (join_state < min_join_state)
    {
      min_join_state = join_state;
    }

    if ((join_state > SL_WISUN_JOIN_STATE_DISCONNECTED) &&
        (join_state <= SL_WISUN_JOIN_STATE_OPERATIONAL))
    {
      app_join_state_sec[join_state] = now_sec();
      // Store transition delay
      sl_simple_led_turn_on(sl_led_network_status.context);
      sl_sleeptimer_delay_millisecond(100);
      sl_simple_led_turn_off(sl_led_network_status.context);
      delay = app_join_state_delay_sec[join_state] =
          app_join_state_sec[join_state] - app_join_state_sec[join_state - 1];
      printfBothTime("app_join_state_delay_sec[%d] = %llu sec\n", join_state, delay);
    }

    if (join_state == SL_WISUN_JOIN_STATE_OPERATIONAL)
    {
      sl_simple_led_turn_on(sl_led_network_status.context);
      connection_time_sec = app_join_state_sec[join_state];
      connection_count++;
      if (min_join_state <= SL_WISUN_JOIN_STATE_ACQUIRE_PAN_CONFIG)
      {
        network_connection_count++;
      }
      // reset min_join_state to follow the next network reconnection
      min_join_state = SL_WISUN_JOIN_STATE_OPERATIONAL;

      // Count disconnected time only once connected for the first time
      if (connection_count == 1)
      {
        printfBothTime("First connection after %llu sec\n", connection_time_sec);
        disconnected_total_sec = 0;
      }
      else
      {
        printfBothTime("Reconnected after %llu sec\n",
                       connection_time_sec - disconnection_time_sec);
        disconnected_total_sec += connection_time_sec - disconnection_time_sec;
      }

      parent_mac = _get_parent_mac_address_and_update_parent_info();
      sprintf(parent_tag, "%02x%02x", parent_mac.address[6], parent_mac.address[7]);

      TRACES_WHEN_CONNECTED;
    }

    // Prepare counting disconnected time
    if (previous_join_state == SL_WISUN_JOIN_STATE_OPERATIONAL)
    {
      for (i = 0; i <= join_state; i++)
      {
        // Clear join_state info for lower and equal join states
        app_join_state_sec[i] = now_sec();
      }
      for (i = 0; i <= join_state; i++)
      {
        app_join_state_delay_sec[i + 1] = app_join_state_sec[i + 1] - app_join_state_sec[i];
      }
      disconnection_time_sec = app_join_state_sec[join_state];
      printfBothTime("Disconnected after %llu sec\n",
                     disconnection_time_sec - connection_time_sec);
      connected_total_sec += disconnection_time_sec - connection_time_sec;
      just_disconnected = true;
      TRACES_WHILE_CONNECTING;
    }
    previous_join_state = join_state;
  }
}

void _check_neighbors(void)
{
  sl_status_t ret;
  uint8_t neighbor_count;
  uint8_t i;

  ret = sl_wisun_get_neighbor_count(&neighbor_count);
  if (ret)
  {
    printf("[Failed: sl_wisun_get_neighbor_count() returned 0x%04x]\n", (uint16_t)ret);
    return;
  }

  if (neighbor_count == 0)
  {
    printf(" no neighbor\n");
  }
  else
  {
    for (i = 0; i < neighbor_count; i++)
    {
      printf("%s\n", app_neighbor_info_str(i));
    }
  }
}

char *_connection_json_string(void)
{
#define CONNECTION_JSON_FORMAT_STR                       \
  START_JSON                                             \
  DEVICE_CHIP_JSON_FORMAT                                \
  PARENT_JSON_FORMAT                                     \
  RUNNING_JSON_FORMAT                                    \
  MSG_COUNT_JSON_FORMAT                                  \
  "\"PAN_ID\":\"0x%04x (%d)\",\n"                        \
  "\"preferred_pan_id\":\"0x%04x (%d)\",\n"              \
  "\"hop_count\":\"%d\",\n"                              \
  "\"join_states_sec\": \"%llu %llu %llu %llu %llu\",\n" \
  "\"application\": \"%s\"\n" END_JSON

  char sec_string[20];

  sl_wisun_get_network_info(&network_info);
  sl_wisun_get_ip_address(SL_WISUN_IP_ADDRESS_TYPE_GLOBAL, &device_global_ipv6);
  sprintf(device_global_ipv6_string, app_wisun_trace_util_get_ip_str(&device_global_ipv6));
  sprintf(sec_string, "%s", now_str());
  refresh_parent_tag();
  msg_count++;

  snprintf(json_string, SL_WISUN_COAP_RESOURCE_HND_SOCK_BUFF_SIZE,
           CONNECTION_JSON_FORMAT_STR,
           DEVICE_CHIP_ITEMS,
           PARENT_INFO_ITEMS,
           sec_string,
           msg_count,
           network_info.pan_id, network_info.pan_id,
           app_parameters.preferred_pan_id, app_parameters.preferred_pan_id,
           network_info.hop_count,
           app_join_state_delay_sec[1],
           app_join_state_delay_sec[2],
           app_join_state_delay_sec[3],
           app_join_state_delay_sec[4],
           app_join_state_delay_sec[5],
           application);
  return json_string;
}

char *_status_json_string(char *start_text)
{
#define CONNECTED_JSON_FORMAT_STR                                \
  "%s" START_JSON                                                \
      DEVICE_CHIP_JSON_FORMAT                                    \
          PARENT_JSON_FORMAT                                     \
              RUNNING_JSON_FORMAT                                \
                  MSG_COUNT_JSON_FORMAT TRACK_HEAP_FORMAT_STRING \
  "\"connected\":\"%s\",\n"                                      \
  "\"disconnected\":\"%s\",\n"                                   \
  "\"connections\":\"%d\",\n"                                    \
  "\"network_connections\":\"%d\"\n" END_JSON

  char sec_string[20];
  char connected_string[20];
  char disconnected_string[20];

  sl_wisun_get_network_info(&network_info);
  sprintf(sec_string, "%s", now_str());
  sprintf(connected_string, "%s", dhms(connected_total_sec));
  sprintf(disconnected_string, "%s", dhms(disconnected_total_sec));
  refresh_parent_tag();
  msg_count++;

  snprintf(json_string, SL_WISUN_COAP_RESOURCE_HND_SOCK_BUFF_SIZE,
           CONNECTED_JSON_FORMAT_STR,
           start_text,
           DEVICE_CHIP_ITEMS,
           PARENT_INFO_ITEMS,
           sec_string,
           msg_count,
           TRACK_HEAP_VALUE
               connected_string,
           disconnected_string,
           connection_count,
           network_connection_count);
  return json_string;
}
