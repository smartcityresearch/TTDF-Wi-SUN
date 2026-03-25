# How to Add CoAP Endpoints to Silicon Labs Wi-SUN Projects

This guide provides step-by-step instructions for adding custom CoAP endpoints to **any Silicon Labs Wi-SUN application** using the CoAP Resource Handler component.

**Applies to:** Wi-SUN SoC projects, Wi-SUN Node Monitor, Wi-SUN CoAP Meter, and custom Wi-SUN applications using the `wisun_coap_rhnd` component.

## Table of Contents

- [Overview](#overview)
- [CoAP Endpoint Basics](#coap-endpoint-basics)
- [Step-by-Step Guide](#step-by-step-guide)
- [Example: Simple Text Endpoint](#example-simple-text-endpoint)
- [Example: JSON Sensor Endpoint](#example-json-sensor-endpoint)
- [Example: Control Endpoint (POST)](#example-control-endpoint-post)
- [Example: Parameterized Endpoint](#example-parameterized-endpoint)
- [Example: Multiple LED Control (Single Endpoint)](#example-multiple-led-control-single-endpoint)
- [Advanced Topics](#advanced-topics)
- [Common Pitfalls](#common-pitfalls)
- [Testing Your Endpoint](#testing-your-endpoint)
- [Frequently Asked Questions (FAQ)](#frequently-asked-questions-faq)
- [Best Practices](#best-practices)
- [Summary Checklist](#summary-checklist)

## Overview

CoAP (Constrained Application Protocol) is a REST-style protocol for IoT devices. In Silicon Labs Wi-SUN projects, CoAP endpoints are implemented using the **Wi-SUN CoAP Resource Handler component** (`wisun_coap_rhnd`).

**This guide works for:**
- Custom Wi-SUN SoC applications
- Wi-SUN Node Monitor projects
- Wi-SUN CoAP Meter applications
- Any project using Silicon Labs Wi-SUN SDK 2024.x / 2025.x with CoAP component

**Examples in this guide use generic code patterns.** Adapt file names and structures to match your specific project template.

### Architecture

```
Client Request → Wi-SUN Network → CoAP Resource Handler → Your Callback Function → Response
```

**Standard Project Structure:**
- Endpoint implementations: `app_coap.c` (or similar CoAP handler file)
- Resource registration: `app_coap_resources_init()` function
- Initialization: Called from `app_init.c` **before** Wi-SUN network connection

**Note:** File names may vary slightly depending on your project template (e.g., `wisun_coap.c`, `coap_handlers.c`), but the pattern is the same.

## CoAP Endpoint Basics

### Key Concepts

1. **URI Path**: The endpoint address (e.g., `/sensor/temperature`)
2. **Resource Type**: Metadata describing the resource (e.g., `"json"`, `"txt"`)
3. **Interface**: Additional metadata (e.g., `"node"`, `"sensor"`)
4. **Callback Function**: Your code that handles the request and generates a response
5. **Discoverable**: Whether the endpoint appears in `.well-known/core` discovery

### Request Methods

- **GET**: Retrieve data (most common)
- **POST**: Send commands or configuration
- **PUT**: Update resource (less common in this project)
- **DELETE**: Remove resource (rarely used)

**Important**: When registering a CoAP resource, **you don't specify which methods are allowed**. All methods (GET, POST, PUT, DELETE) can reach your endpoint. Your callback function receives the request and can check which method was used if needed.

### How Method Handling Works

```c
// Your callback handles ALL methods by default
sl_wisun_coap_packet_t *coap_callback_my_endpoint(
    const sl_wisun_coap_packet_t *const req_packet)
{
  // The method information is in req_packet->msg_code
  // COAP_MSG_CODE_REQUEST_GET     = 1
  // COAP_MSG_CODE_REQUEST_POST    = 2
  // COAP_MSG_CODE_REQUEST_PUT     = 3
  // COAP_MSG_CODE_REQUEST_DELETE  = 4
  
  // You can check the method if you want:
  if (req_packet->msg_code == COAP_MSG_CODE_REQUEST_POST) {
    // Handle POST
    return app_coap_reply("POST handled", req_packet);
  } else if (req_packet->msg_code == COAP_MSG_CODE_REQUEST_GET) {
    // Handle GET
    return app_coap_reply("GET handled", req_packet);
  } else {
    // Reject other methods
    return app_coap_reply("Method not allowed", req_packet);
  }
}
```

**Most endpoints in Silicon Labs examples don't check the method** - they simply respond to any request the same way. This is fine for read-only sensor endpoints where GET is the expected method.

#### Where These Constants Come From

The CoAP message code constants (`COAP_MSG_CODE_REQUEST_GET`, etc.) are defined in the **Mbed CoAP library** header file included with the Wi-SUN SDK:

**File Location:**
```
simplicity_sdk_<version>/app/wisun/component/coap/mbed-coap/sn_coap_header.h
```

**Definition (from `sn_coap_header.h`):**
```c
typedef enum sn_coap_msg_code_ {
  COAP_MSG_CODE_EMPTY             = 0,
  COAP_MSG_CODE_REQUEST_GET       = 1,    /**< RFC 7252 Section 5.8 */
  COAP_MSG_CODE_REQUEST_POST      = 2,    /**< RFC 7252 Section 5.8 */
  COAP_MSG_CODE_REQUEST_PUT       = 3,    /**< RFC 7252 Section 5.8 */
  COAP_MSG_CODE_REQUEST_DELETE    = 4,    /**< RFC 7252 Section 5.8 */
  
  // Response codes (examples):
  COAP_MSG_CODE_RESPONSE_CREATED  = 65,   // 2.01
  COAP_MSG_CODE_RESPONSE_DELETED  = 66,   // 2.02
  COAP_MSG_CODE_RESPONSE_CONTENT  = 69,   // 2.05
  COAP_MSG_CODE_RESPONSE_BAD_REQUEST = 128, // 4.00
  COAP_MSG_CODE_RESPONSE_NOT_FOUND = 132,   // 4.04
  // ... more response codes
} sn_coap_msg_code_e;
```

**Automatically Available:**  
These constants are automatically available when you include Wi-SUN CoAP headers in your project:
```c
#include "sl_wisun_api.h"
#include "sl_wisun_types.h"
```

**Based on RFC 7252:**  
These values follow the official CoAP protocol specification (RFC 7252), ensuring compatibility with all CoAP clients and servers.

### Response Format

- **Plain Text**: Simple string responses
- **JSON**: Structured data (recommended for complex responses)
- **Binary**: Raw data (not commonly used)

## Step-by-Step Guide

### Step 1: Plan Your Endpoint

Before coding, define:
- **Purpose**: What data or function does this endpoint provide?
- **URI**: What path will clients use? (e.g., `/my/new/endpoint`)
- **Method**: GET for read, POST for write/control
- **Response Format**: Text, JSON, or binary?
- **Data Source**: Where does the data come from?

### Step 2: Create the Callback Function

Add your callback function in your CoAP handler file (typically `app_coap.c`, `wisun_coap.c`, or similar). Use this template:

```c
// CoAP callback for /your/endpoint - Description of what it does
sl_wisun_coap_packet_t *coap_callback_your_function(
    const sl_wisun_coap_packet_t *const req_packet)
{
  // 1. Log the request (optional but helpful)
  printf("CoAP /your/endpoint Request received\n");

  // 2. Process the request (read sensor, generate data, etc.)
  // Your logic here...

  // 3. Build the response string
  snprintf(coap_response, COAP_MAX_RESPONSE_LEN, 
           "Your response data here");

  // 4. Log the response (optional)
  printf("CoAP Response: %s\n", coap_response);

  // 5. Return the response
  return app_coap_reply(coap_response, req_packet);
}
```

**Important Notes:**
- Use the global `coap_response` buffer (size: `COAP_MAX_RESPONSE_LEN` = 1000 bytes)
- Always use `snprintf()` to prevent buffer overflows
- Return via `app_coap_reply()` helper function

### Step 3: Register the Resource

Add your endpoint registration in the `app_coap_resources_init()` function (location varies by project, typically in `app_coap.c`):

```c
uint8_t app_coap_resources_init()
{
  sl_wisun_coap_rhnd_resource_t coap_resource = {0};
  uint8_t count = 0;

  // ... existing resource registrations ...

  // Add your new resource
  coap_resource.data.uri_path = "/your/endpoint";
  coap_resource.data.resource_type = "txt";  // or "json"
  coap_resource.data.interface = "node";      // or "sensor"
  coap_resource.auto_response = coap_callback_your_function;
  coap_resource.discoverable = true;
  assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
  count++;

  // ... rest of registrations ...
  
  return count;
}
```

#### Understanding Each Registration Parameter

Let's break down what each line does:

```c
coap_resource.data.uri_path = "/sensors/combo";
```
**Purpose**: Sets the URL path that clients will use to access this endpoint  
**Examples**: 
- `/sensors/combo` - Hierarchical path (recommended)
- `/temp` - Simple path
- `/device/status/battery` - Multi-level path  
**Notes**: 
- Must start with `/`
- Case-sensitive
- Use lowercase for consistency
- Avoid spaces and special characters

```c
coap_resource.data.resource_type = "json";
```
**Purpose**: Metadata describing the content type returned by this endpoint  
**Common Values**:
- `"json"` - JSON formatted data (recommended for structured data)
- `"txt"` or `"text"` - Plain text responses
- `"xml"` - XML formatted data
- `"binary"` - Raw binary data
- Custom string - Any descriptive string  
**Notes**: 
- This is **metadata only** - doesn't affect actual response format
- Appears in CoAP discovery (`.well-known/core`)
- Helps clients understand what to expect

```c
coap_resource.data.interface = "sensor";
```
**Purpose**: Additional metadata categorizing the endpoint's function  
**Common Values**:
- `"sensor"` - Sensor data endpoints (read-only)
- `"actuator"` - Control endpoints (commands)
- `"node"` - Device/network information
- `"core"` - Core functionality
- Custom string - Any category you define  
**Notes**: 
- Also metadata only
- Used for logical grouping
- Appears in discovery responses

```c
coap_resource.auto_response = coap_callback_sensors_combo;
```
**Purpose**: Connects this URI to your callback function that handles requests  
**Value**: Function pointer to your callback  
**Requirements**:
- Function must have signature: `sl_wisun_coap_packet_t *function_name(const sl_wisun_coap_packet_t *const req_packet)`
- Function must return `app_coap_reply(response_string, req_packet)`
- Function will be called automatically when clients access this URI

```c
coap_resource.discoverable = true;
```
**Purpose**: Controls whether this endpoint appears in discovery responses  
**Values**:
- `true` - Endpoint appears in `.well-known/core` (recommended for most endpoints)
- `false` - Hidden endpoint (useful for internal/debug endpoints)  
**Discovery Example**:
```bash
coap-client -m get coap://[device]:5683/.well-known/core
# Shows: </sensors/combo>;rt="json";if="sensor"
```

```c
assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
```
**Purpose**: Registers the resource with the CoAP handler and verifies success  
**What happens**:
- Adds the resource to the CoAP resource table
- Returns `SL_STATUS_OK` (0x0000) on success
- Returns error code if registration fails  
**Common Failure Reasons**:
- Resource table full (exceeded `SL_WISUN_COAP_RESOURCE_HND_MAX_RESOURCES`)
- Duplicate URI path
- Invalid configuration
- Handler not initialized  
**Note**: The `assert()` will crash the application if registration fails, making it easy to catch configuration errors during development

```c
count++;
```
**Purpose**: Increments the counter tracking how many resources were registered  
**Important**: 
- **Must** increment after each successful registration
- Returned value used for logging/debugging
- If you forget this, function will return incorrect count

#### Complete Registration Example with Explanations

```c
// Sensor data endpoint returning JSON
coap_resource.data.uri_path = "/sensors/temperature";  // Client accesses via this URL
coap_resource.data.resource_type = "json";             // Returns JSON formatted data
coap_resource.data.interface = "sensor";               // Categorized as sensor endpoint
coap_resource.auto_response = coap_callback_temp;      // Function that handles requests
coap_resource.discoverable = true;                     // Visible in discovery
assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);  // Register it
count++;                                               // Track registration count

// Control endpoint (plain text response)
coap_resource.data.uri_path = "/relay/on";             // Control command URL
coap_resource.data.resource_type = "txt";              // Returns simple text
coap_resource.data.interface = "actuator";             // Categorized as control endpoint
coap_resource.auto_response = coap_callback_relay_on;  // Handler function
coap_resource.discoverable = true;                     // Visible in discovery
assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);  // Register it
count++;                                               // Increment counter

// Hidden diagnostic endpoint
coap_resource.data.uri_path = "/debug/internal";       // Debug access URL
coap_resource.data.resource_type = "txt";              // Text diagnostic info
coap_resource.data.interface = "core";                 // Core system endpoint
coap_resource.auto_response = coap_callback_debug;     // Debug handler
coap_resource.discoverable = false;                    // Hidden from discovery
assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);  // Register it
count++;                                               // Don't forget to increment!
```

#### Quick Reference Table

| Parameter | Purpose | Type | Example Values | Required? |
|-----------|---------|------|----------------|-----------|
| `uri_path` | URL endpoint path | string | `/sensors/temp`, `/relay/on` | ✅ Yes |
| `resource_type` | Content type metadata | string | `"json"`, `"txt"`, `"xml"` | ✅ Yes |
| `interface` | Endpoint category | string | `"sensor"`, `"actuator"`, `"node"` | ✅ Yes |
| `auto_response` | Callback function | function pointer | `coap_callback_myfunction` | ✅ Yes |
| `discoverable` | Show in discovery | boolean | `true`, `false` | ✅ Yes |
| *(add & assert)* | Register resource | function call | `sl_wisun_coap_rhnd_resource_add()` | ✅ Yes |
| `count++` | Increment counter | increment | `count++` | ✅ Yes |

#### Resource Registration Flow

```
1. Configure URI path       → Sets the endpoint URL
                 ↓
2. Set resource_type        → Describes content format (metadata)
                 ↓
3. Set interface            → Categorizes endpoint (metadata)
                 ↓
4. Assign callback          → Links to your handler function
                 ↓
5. Set discoverable         → Controls visibility in discovery
                 ↓
6. Add to resource table    → Registers with CoAP handler
                 ↓
7. Verify success (assert)  → Crashes if registration failed
                 ↓
8. Increment counter        → Tracks number of resources
```

#### Understanding the sl_wisun_coap_rhnd_resource_t Structure

The `sl_wisun_coap_rhnd_resource_t` structure defined by Silicon Labs contains:

```c
typedef struct {
  sl_wisun_coap_resource_data_t data;        // Resource metadata
  sl_wisun_coap_rhnd_resource_callback_t auto_response;  // Handler function
  bool discoverable;                          // Discovery flag
} sl_wisun_coap_rhnd_resource_t;
```

Where `sl_wisun_coap_resource_data_t` contains:

```c
typedef struct {
  const char *uri_path;           // URI path string
  const char *resource_type;      // Resource type string
  const char *interface;          // Interface description string
} sl_wisun_coap_resource_data_t;
```

**Initialization Pattern:**
```c
sl_wisun_coap_rhnd_resource_t coap_resource = {0};  // Zero-initialize all fields
```

This ensures all fields start with safe default values (NULL pointers, false booleans) before you configure them.

#### Common Resource Type and Interface Combinations

| Use Case | resource_type | interface | Example URI |
|----------|---------------|-----------|-------------|
| Sensor reading (JSON) | `"json"` | `"sensor"` | `/sensors/temp` |
| Sensor reading (text) | `"txt"` | `"sensor"` | `/temp` |
| Control command | `"txt"` | `"actuator"` | `/relay/on` |
| Device info | `"json"` | `"node"` | `/info/device` |
| Network status | `"json"` | `"node"` | `/status/network` |
| Configuration | `"json"` | `"core"` | `/config/params` |
| Diagnostics | `"txt"` | `"core"` | `/debug/logs` |

### Step 4: Update the Resource Count

If you're adding many endpoints, ensure `SL_WISUN_COAP_RESOURCE_HND_MAX_RESOURCES` in `config/sl_wisun_coap_config.h` is sufficient:

```c
#define SL_WISUN_COAP_RESOURCE_HND_MAX_RESOURCES        50U
```

Default is typically 10-50 resources depending on the example project. Increase if you need more endpoints.

#### Note About HTTP Methods

**No method specification needed during registration!** 

The resource registration doesn't include a method field. Your endpoint will accept **all methods** (GET, POST, PUT, DELETE) by default. You can optionally check the method in your callback:

```c
// Example: Endpoint that handles both GET and POST
sl_wisun_coap_packet_t *coap_callback_dual_method(
    const sl_wisun_coap_packet_t *const req_packet)
{
  // Check which method was used
  if (req_packet->msg_code == COAP_MSG_CODE_REQUEST_GET) {
    // GET: Return current value
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN, 
             "Current value: %d", some_value);
  }
  else if (req_packet->msg_code == COAP_MSG_CODE_REQUEST_POST) {
    // POST: Update value from payload
    some_value = atoi((char*)req_packet->payload_ptr);
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN, 
             "Value updated to: %d", some_value);
  }
  else {
    // Other methods not supported
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN, 
             "Method not supported");
  }
  
  return app_coap_reply(coap_response, req_packet);
}

// Registration is the same - no method specified
coap_resource.data.uri_path = "/config/value";
coap_resource.data.resource_type = "txt";
coap_resource.auto_response = coap_callback_dual_method;
// ... rest of registration
```

**In practice**, most endpoints in Silicon Labs example projects ignore the method and respond the same way to any request. This works well for read-only sensor endpoints where GET is implied.

#### Available CoAP Message Codes

If you need to check the request method, here are the available codes:

| Method | Code Constant | Value | Typical Use |
|--------|---------------|-------|-------------|
| GET | `COAP_MSG_CODE_REQUEST_GET` | 1 | Retrieve data (read) |
| POST | `COAP_MSG_CODE_REQUEST_POST` | 2 | Submit data, trigger action |
| PUT | `COAP_MSG_CODE_REQUEST_PUT` | 3 | Update/replace resource |
| DELETE | `COAP_MSG_CODE_REQUEST_DELETE` | 4 | Remove resource |

**Access in callback:**
```c
uint8_t method = req_packet->msg_code;

// Or use switch statement:
switch (req_packet->msg_code) {
  case COAP_MSG_CODE_REQUEST_GET:
    // Handle GET
    break;
  case COAP_MSG_CODE_REQUEST_POST:
    // Handle POST
    break;
  case COAP_MSG_CODE_REQUEST_PUT:
    // Handle PUT
    break;
  case COAP_MSG_CODE_REQUEST_DELETE:
    // Handle DELETE
    break;
  default:
    // Unknown method
    break;
}
```

### Step 5: Build and Test

1. **Build** the project in Simplicity Studio
2. **Flash** to your device
3. **Test** using coap-client tool (see [Testing](#testing-your-endpoint))

## Example: Simple Text Endpoint

Let's create `/device/uptime` that returns how long the device has been running.

### Implementation

```c
// CoAP callback for /device/uptime - Returns device uptime
sl_wisun_coap_packet_t *coap_callback_device_uptime(
    const sl_wisun_coap_packet_t *const req_packet)
{
  printf("CoAP /device/uptime Request received\n");

  // Get current uptime using existing utility function
  uint64_t uptime_sec = now_sec();
  
  // Format as days-hours:minutes:seconds
  snprintf(coap_response, COAP_MAX_RESPONSE_LEN, 
           "%s", dhms(uptime_sec));

  printf("CoAP Response: %s\n", coap_response);
  return app_coap_reply(coap_response, req_packet);
}
```

### Registration

```c
// In app_coap_resources_init():
coap_resource.data.uri_path = "/device/uptime";
coap_resource.data.resource_type = "txt";
coap_resource.data.interface = "node";
coap_resource.auto_response = coap_callback_device_uptime;
coap_resource.discoverable = true;
assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
count++;
```

### Testing

```bash
coap-client -m get coap://[device-ipv6]:5683/device/uptime
# Response: 0-02:15:30  (2 hours, 15 minutes, 30 seconds)
```

## Example: JSON Sensor Endpoint

Let's create `/sensors/combo` that returns combined Si7021 and VEML6035 data.

### Implementation

```c
// CoAP callback for /sensors/combo - Returns combined sensor data in JSON
sl_wisun_coap_packet_t *coap_callback_sensors_combo(
    const sl_wisun_coap_packet_t *const req_packet)
{
  #define JSON_COMBO_FORMAT_STR \
    "{\n"                       \
    "  \"temperature\": %.2f,\n" \
    "  \"humidity\": %.1f,\n"   \
    "  \"lux\": %lu,\n"         \
    "  \"white\": %lu,\n"       \
    "  \"si7021_ok\": %s,\n"    \
    "  \"veml_ok\": %s\n"       \
    "}\n"

  printf("CoAP /sensors/combo Request received\n");

  // Read sensors on-demand
  read_si7021_data();
  read_veml_data();

  // Build JSON response
  snprintf(coap_response, COAP_MAX_RESPONSE_LEN, JSON_COMBO_FORMAT_STR,
           latest_si7021_data.temperature,
           latest_si7021_data.humidity,
           latest_veml_data.als,
           latest_veml_data.white,
           latest_si7021_data.available ? "true" : "false",
           latest_veml_data.available ? "true" : "false");

  // Calculate data age
  uint64_t si7021_age = now_sec() - last_si7021_read_time;
  uint64_t veml_age = now_sec() - last_veml_read_time;
  
  printf("CoAP Response (si7021: %llu sec, veml: %llu sec):\n%s", 
         si7021_age, veml_age, coap_response);
  
  return app_coap_reply(coap_response, req_packet);
}
```

### Registration

```c
coap_resource.data.uri_path = "/sensors/combo";
coap_resource.data.resource_type = "json";
coap_resource.data.interface = "sensor";
coap_resource.auto_response = coap_callback_sensors_combo;
coap_resource.discoverable = true;
assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
count++;
```

### Testing

```bash
coap-client -m get coap://[device-ipv6]:5683/sensors/combo
```

**Response:**
```json
{
  "temperature": 24.52,
  "humidity": 45.2,
  "lux": 320,
  "white": 450,
  "si7021_ok": true,
  "veml_ok": true
}
```

## Example: Control Endpoint (POST)

Let's create `/relay/toggle` that toggles the relay state. While we call this a "POST endpoint" and intend it to be used with POST, it technically accepts all methods (as explained in the basics section).

### Implementation

```c
// Global variable to track relay state (add near other globals)
extern bool relay_status_on;  // Already defined in app_coap.c

// CoAP callback for /relay/toggle - Toggle relay state
// Note: This endpoint accepts any method, but POST is recommended for control actions
sl_wisun_coap_packet_t *coap_callback_relay_toggle(
    const sl_wisun_coap_packet_t *const req_packet)
{
  printf("CoAP /relay/toggle Request received\n");

  // Optional: Check if POST was used (uncomment if you want to enforce)
  // if (req_packet->msg_code != COAP_MSG_CODE_REQUEST_POST) {
  //   snprintf(coap_response, COAP_MAX_RESPONSE_LEN, 
  //            "ERROR: Use POST method for this endpoint");
  //   return app_coap_reply(coap_response, req_packet);
  // }

  // Toggle the relay state
  relay_status_on = !relay_status_on;

  // Control the actual hardware LED
  if (relay_status_on) {
    sl_led_turn_on(&sl_led_relay);
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN, 
             "Relay toggled ON");
  } else {
    sl_led_turn_off(&sl_led_relay);
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN, 
             "Relay toggled OFF");
  }

  printf("CoAP Response: %s\n", coap_response);
  return app_coap_reply(coap_response, req_packet);
}
```

### Registration

```c
coap_resource.data.uri_path = "/relay/toggle";
coap_resource.data.resource_type = "txt";
coap_resource.data.interface = "node";
coap_resource.auto_response = coap_callback_relay_toggle;
coap_resource.discoverable = true;
assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
count++;
```

### Testing

```bash
# Toggle relay state
coap-client -m post coap://[device-ipv6]:5683/relay/toggle
# Response: Relay toggled ON

coap-client -m post coap://[device-ipv6]:5683/relay/toggle
# Response: Relay toggled OFF

# Note: Even though we use POST, GET would also work (unless you add method checking)
coap-client -m get coap://[device-ipv6]:5683/relay/toggle
# Response: Relay toggled ON (still works!)
```

### About Methods and Control Endpoints

**Key Takeaway**: Even "control endpoints" like `/relay/toggle` don't enforce POST by default. The resource registration has no method field. If you want to enforce a specific method, you must add checking code in your callback (as shown in the commented lines above).

**Design Choice**:
- ✅ **Simple approach** (used in this project): Accept all methods, rely on client using correct method
- ✅ **Strict approach**: Check `req_packet->msg_code` and return error for unsupported methods

Most IoT clients will use the appropriate method (POST for commands, GET for reads), so method enforcement is often unnecessary overhead.

## Example: Parameterized Endpoint

Let's create `/led/brightness` that accepts a brightness value in the payload.

### Implementation

```c
// CoAP callback for /led/brightness - Set LED brightness (0-100)
sl_wisun_coap_packet_t *coap_callback_led_brightness(
    const sl_wisun_coap_packet_t *const req_packet)
{
  printf("CoAP /led/brightness Request received\n");

  // Check if payload exists
  if (req_packet->payload_len == 0) {
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "ERROR: Missing brightness value (0-100)");
    printf("%s\n", coap_response);
    return app_coap_reply(coap_response, req_packet);
  }

  // Parse brightness value from payload
  char payload_str[16] = {0};
  size_t copy_len = (req_packet->payload_len < 15) ? 
                     req_packet->payload_len : 15;
  memcpy(payload_str, req_packet->payload_ptr, copy_len);
  payload_str[copy_len] = '\0';

  int brightness = atoi(payload_str);

  // Validate range
  if (brightness < 0 || brightness > 100) {
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "ERROR: Brightness must be 0-100, got %d", brightness);
    printf("%s\n", coap_response);
    return app_coap_reply(coap_response, req_packet);
  }

  // Apply brightness (example using PWM - actual implementation depends on hardware)
  // This is pseudo-code - adjust for your LED driver
  // sl_led_set_brightness(&sl_led_relay, (uint8_t)brightness);

  snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
           "Brightness set to %d%%", brightness);

  printf("CoAP Response: %s\n", coap_response);
  return app_coap_reply(coap_response, req_packet);
}
```

### Registration

```c
coap_resource.data.uri_path = "/led/brightness";
coap_resource.data.resource_type = "txt";
coap_resource.data.interface = "node";
coap_resource.auto_response = coap_callback_led_brightness;
coap_resource.discoverable = true;
assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
count++;
```

### Testing

```bash
# Set brightness to 75%
echo -n "75" | coap-client -m post -f- coap://[device-ipv6]:5683/led/brightness
# Response: Brightness set to 75%

# Invalid value
echo -n "150" | coap-client -m post -f- coap://[device-ipv6]:5683/led/brightness
# Response: ERROR: Brightness must be 0-100, got 150
```

## Example: Multiple LED Control (Single Endpoint)

Instead of creating 10 separate endpoints (`/led0/on`, `/led0/off`, `/led1/on`, etc.), create a **single endpoint** that accepts the LED number and state as parameters. This is much more scalable and maintainable.

### Use Case

Control 10 LEDs (led0 through led9) with one endpoint by sending commands like:
- `"led0 on"` - Turn on LED 0
- `"led2 off"` - Turn off LED 2
- `"led9 on"` - Turn on LED 9

### Implementation

```c
// CoAP callback for /led/control - Control multiple LEDs with single endpoint
// Payload format: "led<number> <state>"
// Examples: "led0 on", "led2 off", "led9 on"
sl_wisun_coap_packet_t *coap_callback_led_control(
    const sl_wisun_coap_packet_t *const req_packet)
{
  printf("CoAP /led/control Request received\n");

  // Check if payload exists
  if (req_packet->payload_len == 0) {
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "ERROR: Missing payload. Format: 'led<0-9> <on|off>'");
    printf("%s\n", coap_response);
    return app_coap_reply(coap_response, req_packet);
  }

  // Parse payload (format: "led0 on" or "led2 off")
  char payload_str[32] = {0};
  size_t copy_len = (req_packet->payload_len < 31) ? 
                     req_packet->payload_len : 31;
  memcpy(payload_str, req_packet->payload_ptr, copy_len);
  payload_str[copy_len] = '\0';

  // Variables to store parsed values
  int led_num = -1;
  char state_str[10] = {0};

  // Parse: "led<number> <state>"
  int parsed = sscanf(payload_str, "led%d %s", &led_num, state_str);

  if (parsed != 2) {
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "ERROR: Invalid format. Use: 'led<0-9> <on|off>'");
    printf("%s\n", coap_response);
    return app_coap_reply(coap_response, req_packet);
  }

  // Validate LED number (0-9)
  if (led_num < 0 || led_num > 9) {
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "ERROR: Invalid LED number %d. Must be 0-9", led_num);
    printf("%s\n", coap_response);
    return app_coap_reply(coap_response, req_packet);
  }

  // Validate and execute state command
  bool turn_on = false;
  if (strcmp(state_str, "on") == 0) {
    turn_on = true;
  } else if (strcmp(state_str, "off") == 0) {
    turn_on = false;
  } else {
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "ERROR: Invalid state '%s'. Use 'on' or 'off'", state_str);
    printf("%s\n", coap_response);
    return app_coap_reply(coap_response, req_packet);
  }

  // Control the LED based on number
  // Option 1: Using array of LED instances (if you have them defined)
  /*
  extern sl_led_t led_array[10];  // Assume you have LED array
  if (turn_on) {
    sl_led_turn_on(&led_array[led_num]);
  } else {
    sl_led_turn_off(&led_array[led_num]);
  }
  */

  // Option 2: Using GPIO pins directly (example)
  /*
  // Define LED GPIO pins (example for EFR32)
  static const GPIO_Port_TypeDef led_ports[10] = {
    gpioPortA, gpioPortA, gpioPortB, gpioPortB, gpioPortC,
    gpioPortC, gpioPortD, gpioPortD, gpioPortF, gpioPortF
  };
  static const uint8_t led_pins[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  
  GPIO_PinModeSet(led_ports[led_num], led_pins[led_num], 
                  gpioModePushPull, turn_on ? 1 : 0);
  */

  // Option 3: Using switch statement for specific LEDs
  switch (led_num) {
    case 0:
      // Control LED 0 (example: relay LED or custom GPIO)
      if (turn_on) {
        // sl_led_turn_on(&sl_led_led0);
        printf("LED 0 turned ON\n");
      } else {
        // sl_led_turn_off(&sl_led_led0);
        printf("LED 0 turned OFF\n");
      }
      break;
    
    case 1:
      // Control LED 1
      if (turn_on) {
        printf("LED 1 turned ON\n");
      } else {
        printf("LED 1 turned OFF\n");
      }
      break;
    
    case 2:
      // Control LED 2
      if (turn_on) {
        printf("LED 2 turned ON\n");
      } else {
        printf("LED 2 turned OFF\n");
      }
      break;
    
    // Add cases 3-9 as needed
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      // Placeholder for LEDs 3-9
      printf("LED %d turned %s\n", led_num, turn_on ? "ON" : "OFF");
      break;
    
    default:
      snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
               "ERROR: LED %d not configured", led_num);
      return app_coap_reply(coap_response, req_packet);
  }

  // Build success response
  snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
           "LED %d turned %s", led_num, turn_on ? "ON" : "OFF");

  printf("CoAP Response: %s\n", coap_response);
  return app_coap_reply(coap_response, req_packet);
}
```

### Alternative: JSON Payload Format

For more structured control, you can also accept JSON payloads:

```c
// Alternative version accepting JSON: {"led": 0, "state": "on"}
sl_wisun_coap_packet_t *coap_callback_led_control_json(
    const sl_wisun_coap_packet_t *const req_packet)
{
  printf("CoAP /led/control Request received\n");

  if (req_packet->payload_len == 0) {
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "ERROR: Missing JSON payload. Format: {\"led\":0,\"state\":\"on\"}");
    return app_coap_reply(coap_response, req_packet);
  }

  char payload_str[64] = {0};
  size_t copy_len = (req_packet->payload_len < 63) ? 
                     req_packet->payload_len : 63;
  memcpy(payload_str, req_packet->payload_ptr, copy_len);
  payload_str[copy_len] = '\0';

  // Simple JSON parsing (for production, use proper JSON parser)
  int led_num = -1;
  char state_str[10] = {0};
  
  // Parse JSON manually (simplified)
  char *led_pos = strstr(payload_str, "\"led\"");
  char *state_pos = strstr(payload_str, "\"state\"");
  
  if (led_pos && state_pos) {
    sscanf(led_pos, "\"led\":%d", &led_num);
    sscanf(state_pos, "\"state\":\"%[^\"]", state_str);
  } else {
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "ERROR: Invalid JSON format");
    return app_coap_reply(coap_response, req_packet);
  }

  // Validate and control LED (same logic as text version)
  if (led_num < 0 || led_num > 9) {
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "ERROR: Invalid LED number %d", led_num);
    return app_coap_reply(coap_response, req_packet);
  }

  bool turn_on = (strcmp(state_str, "on") == 0);
  
  // Control LED (implement your LED control logic here)
  printf("LED %d turned %s\n", led_num, turn_on ? "ON" : "OFF");

  snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
           "{\"led\":%d,\"state\":\"%s\",\"success\":true}",
           led_num, turn_on ? "on" : "off");

  return app_coap_reply(coap_response, req_packet);
}
```

### Registration

```c
// In app_coap_resources_init():
coap_resource.data.uri_path = "/led/control";
coap_resource.data.resource_type = "txt";  // or "json" if using JSON version
coap_resource.data.interface = "actuator";
coap_resource.auto_response = coap_callback_led_control;  // or coap_callback_led_control_json
coap_resource.discoverable = true;
assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
count++;
```

### Testing

**Text Format:**
```bash
# Turn on LED 0
echo -n "led0 on" | coap-client -m post -f- coap://[device-ipv6]:5683/led/control
# Response: LED 0 turned ON

# Turn off LED 2
echo -n "led2 off" | coap-client -m post -f- coap://[device-ipv6]:5683/led/control
# Response: LED 2 turned OFF

# Turn on LED 9
echo -n "led9 on" | coap-client -m post -f- coap://[device-ipv6]:5683/led/control
# Response: LED 9 turned ON

# Invalid LED number
echo -n "led15 on" | coap-client -m post -f- coap://[device-ipv6]:5683/led/control
# Response: ERROR: Invalid LED number 15. Must be 0-9

# Invalid state
echo -n "led3 blink" | coap-client -m post -f- coap://[device-ipv6]:5683/led/control
# Response: ERROR: Invalid state 'blink'. Use 'on' or 'off'

# Missing payload
coap-client -m post coap://[device-ipv6]:5683/led/control
# Response: ERROR: Missing payload. Format: 'led<0-9> <on|off>'
```

**JSON Format:**
```bash
# Turn on LED 0 (JSON)
echo -n '{"led":0,"state":"on"}' | coap-client -m post -f- coap://[device-ipv6]:5683/led/control
# Response: {"led":0,"state":"on","success":true}

# Turn off LED 5 (JSON)
echo -n '{"led":5,"state":"off"}' | coap-client -m post -f- coap://[device-ipv6]:5683/led/control
# Response: {"led":5,"state":"off","success":true}
```

**Python Test Script:**
```python
import aiocoap
import asyncio

async def control_leds():
    context = await aiocoap.Context.create_client_context()
    device_ipv6 = "fd12:3456::eae:5fff:fe6d:1fe1"  # Replace with your device
    
    # Turn on all LEDs
    for led_num in range(10):
        payload = f"led{led_num} on"
        request = aiocoap.Message(
            code=aiocoap.POST,
            uri=f'coap://[{device_ipv6}]/led/control',
            payload=payload.encode()
        )
        response = await context.request(request).response
        print(f"LED {led_num}: {response.payload.decode()}")
        await asyncio.sleep(0.5)  # Small delay between commands
    
    # Turn off all LEDs
    for led_num in range(10):
        payload = f"led{led_num} off"
        request = aiocoap.Message(
            code=aiocoap.POST,
            uri=f'coap://[{device_ipv6}]/led/control',
            payload=payload.encode()
        )
        response = await context.request(request).response
        print(f"LED {led_num}: {response.payload.decode()}")
        await asyncio.sleep(0.5)

asyncio.run(control_leds())
```

### Benefits of This Approach

✅ **Scalability**: Control 10, 50, or 100 LEDs with one endpoint  
✅ **Maintainability**: Single function to update instead of many endpoints  
✅ **Flexibility**: Easy to add new commands (e.g., "toggle", "blink")  
✅ **Discovery**: Only one endpoint appears in `.well-known/core`  
✅ **Resource Efficiency**: Uses less memory and code space  

### Comparison: Multiple Endpoints vs Single Endpoint

**❌ Multiple Endpoints Approach (NOT Recommended):**
```c
// Would need 20 endpoints for 10 LEDs (on + off)!
coap_resource.uri_path = "/led0/on";
coap_resource.uri_path = "/led0/off";
coap_resource.uri_path = "/led1/on";
coap_resource.uri_path = "/led1/off";
// ... 16 more registrations ...
```

**✅ Single Endpoint Approach (Recommended):**
```c
// Just 1 endpoint handles everything!
coap_resource.uri_path = "/led/control";
```

### Extending the Example

You can extend this pattern to support:

**1. Toggle Command:**
```c
else if (strcmp(state_str, "toggle") == 0) {
  // Toggle LED state
  led_states[led_num] = !led_states[led_num];
  // Apply to hardware
}
```

**2. Brightness Control:**
```c
// Payload: "led3 brightness 75"
int brightness;
sscanf(payload_str, "led%d brightness %d", &led_num, &brightness);
```

**3. Blink Pattern:**
```c
// Payload: "led2 blink 500"  (blink with 500ms period)
int period_ms;
sscanf(payload_str, "led%d blink %d", &led_num, &period_ms);
```

**4. Multiple LEDs at Once:**
```c
// Payload: "led0,2,5 on"  (turn on LEDs 0, 2, and 5)
char led_list[32];
sscanf(payload_str, "led%s %s", led_list, state_str);
// Parse comma-separated list
```

### Silicon Labs CoAP Meter Pattern (Official Reference)

The official **Silicon Labs Wi-SUN CoAP Meter** application uses an even simpler approach for LED control. This is the **production-proven pattern** from Silicon Labs.

#### Key Differences from Above:

- **Simpler payload**: Just a single character `"0"` through `"9"` (no "led" prefix, no "on/off")
- **Implicit action**: Typically "toggle" (but you can adapt to on/off with separate endpoints)
- **Less parsing**: Extract one byte, convert to LED number, done
- **Lower bandwidth**: Minimal payload size

#### Implementation (Silicon Labs Style)

```c
// CoAP callback for /gpio/led - Toggle LED (Silicon Labs CoAP Meter pattern)
// Payload: Single character "0" through "9" representing LED number
// Reference: Wi-SUN CoAP Meter application
sl_wisun_coap_packet_t *coap_callback_gpio_led_toggle(
    const sl_wisun_coap_packet_t *const req_packet)
{
  printf("CoAP /gpio/led Request received\n");

  // Check if payload exists
  if (req_packet->payload_len == 0) {
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "ERROR: Missing LED number (0-9)");
    printf("%s\n", coap_response);
    return app_coap_reply(coap_response, req_packet);
  }

  // Parse LED number from payload (first byte/character)
  // Payload is just "0", "1", "2", ... "9"
  char led_char = req_packet->payload_ptr[0];
  int led_num = -1;

  // Convert character to LED number
  if (led_char >= '0' && led_char <= '9') {
    led_num = led_char - '0';  // Convert ASCII '0'-'9' to int 0-9
  } else {
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "ERROR: Invalid LED number. Send '0' through '9'");
    printf("%s\n", coap_response);
    return app_coap_reply(coap_response, req_packet);
  }

  // Track LED states (add this as global or static array)
  static bool led_states[10] = {false};  // false = off, true = on

  // Toggle the LED state
  led_states[led_num] = !led_states[led_num];

  // Control the actual hardware LED
  // Option 1: Using LED instances (if available)
  /*
  extern sl_led_t led_array[10];
  if (led_states[led_num]) {
    sl_led_turn_on(&led_array[led_num]);
  } else {
    sl_led_turn_off(&led_array[led_num]);
  }
  */

  // Option 2: Using switch for specific LEDs
  switch (led_num) {
    case 0:
      if (led_states[0]) {
        // Turn on LED 0
        // sl_led_turn_on(&sl_led_led0);
        printf("LED 0 toggled ON\n");
      } else {
        // sl_led_turn_off(&sl_led_led0);
        printf("LED 0 toggled OFF\n");
      }
      break;
    
    case 1:
      if (led_states[1]) {
        printf("LED 1 toggled ON\n");
      } else {
        printf("LED 1 toggled OFF\n");
      }
      break;
    
    // Add cases 2-9 as needed
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      printf("LED %d toggled %s\n", led_num, 
             led_states[led_num] ? "ON" : "OFF");
      break;
    
    default:
      snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
               "ERROR: LED %d not configured", led_num);
      return app_coap_reply(coap_response, req_packet);
  }

  // Build success response
  snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
           "LED %d toggled %s", led_num, 
           led_states[led_num] ? "ON" : "OFF");

  printf("CoAP Response: %s\n", coap_response);
  return app_coap_reply(coap_response, req_packet);
}
```

#### Alternative: Separate On/Off Endpoints (Silicon Labs Style)

Instead of toggle, you can use separate endpoints for explicit control:

```c
// /gpio/led/on - Turn on LED n
sl_wisun_coap_packet_t *coap_callback_gpio_led_on(
    const sl_wisun_coap_packet_t *const req_packet)
{
  if (req_packet->payload_len == 0) {
    return app_coap_reply("ERROR: Missing LED number", req_packet);
  }

  int led_num = req_packet->payload_ptr[0] - '0';  // Simple: "0" to 0
  
  if (led_num < 0 || led_num > 9) {
    return app_coap_reply("ERROR: LED number must be 0-9", req_packet);
  }

  // Turn on LED
  // sl_led_turn_on(&led_array[led_num]);
  printf("LED %d turned ON\n", led_num);
  
  snprintf(coap_response, COAP_MAX_RESPONSE_LEN, "LED %d ON", led_num);
  return app_coap_reply(coap_response, req_packet);
}

// /gpio/led/off - Turn off LED n
sl_wisun_coap_packet_t *coap_callback_gpio_led_off(
    const sl_wisun_coap_packet_t *const req_packet)
{
  if (req_packet->payload_len == 0) {
    return app_coap_reply("ERROR: Missing LED number", req_packet);
  }

  int led_num = req_packet->payload_ptr[0] - '0';
  
  if (led_num < 0 || led_num > 9) {
    return app_coap_reply("ERROR: LED number must be 0-9", req_packet);
  }

  // Turn off LED
  // sl_led_turn_off(&led_array[led_num]);
  printf("LED %d turned OFF\n", led_num);
  
  snprintf(coap_response, COAP_MAX_RESPONSE_LEN, "LED %d OFF", led_num);
  return app_coap_reply(coap_response, req_packet);
}
```

#### Registration (Silicon Labs Pattern)

```c
// In app_coap_resources_init():

// Option 1: Single toggle endpoint (like CoAP Meter)
coap_resource.data.uri_path = "/gpio/led";
coap_resource.data.resource_type = "txt";
coap_resource.data.interface = "actuator";
coap_resource.auto_response = coap_callback_gpio_led_toggle;
coap_resource.discoverable = true;
assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
count++;

// Option 2: Separate on/off endpoints (still just 2 endpoints total!)
coap_resource.data.uri_path = "/gpio/led/on";
coap_resource.data.resource_type = "txt";
coap_resource.data.interface = "actuator";
coap_resource.auto_response = coap_callback_gpio_led_on;
coap_resource.discoverable = true;
assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
count++;

coap_resource.data.uri_path = "/gpio/led/off";
coap_resource.data.resource_type = "txt";
coap_resource.data.interface = "actuator";
coap_resource.auto_response = coap_callback_gpio_led_off;
coap_resource.discoverable = true;
assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
count++;
```

#### Testing (Silicon Labs Pattern)

**Toggle endpoint (matching CoAP Meter example):**
```bash
# Toggle LED 0 (using PUT method like Silicon Labs example)
echo -n "0" | coap-client -m put -f- coap://[device-ipv6]:5683/gpio/led
# Response: LED 0 toggled ON

# Toggle LED 0 again
echo -n "0" | coap-client -m put -f- coap://[device-ipv6]:5683/gpio/led
# Response: LED 0 toggled OFF

# Toggle LED 3
echo -n "3" | coap-client -m put -f- coap://[device-ipv6]:5683/gpio/led
# Response: LED 3 toggled ON

# Toggle LED 9
echo -n "9" | coap-client -m put -f- coap://[device-ipv6]:5683/gpio/led
# Response: LED 9 toggled ON

# Using coap-client options like the official example
coap-client -t text -m put -N -B 1 coap://[device-ipv6]:5683/gpio/led -e "0"
# -t text   : Content format text/plain
# -m put    : PUT method (can also use POST)
# -N        : Non-confirmable message
# -B 1      : Block size
# -e "0"    : Payload
```

**Separate on/off endpoints:**
```bash
# Turn on LED 0
echo -n "0" | coap-client -m post -f- coap://[device-ipv6]:5683/gpio/led/on
# Response: LED 0 ON

# Turn off LED 0
echo -n "0" | coap-client -m post -f- coap://[device-ipv6]:5683/gpio/led/off
# Response: LED 0 OFF

# Turn on LED 5
echo -n "5" | coap-client -m post -f- coap://[device-ipv6]:5683/gpio/led/on
# Response: LED 5 ON

# Turn off all LEDs in sequence
for i in {0..9}; do
  echo -n "$i" | coap-client -m post -f- coap://[device-ipv6]:5683/gpio/led/off
done
```

**Python automation (Silicon Labs pattern):**
```python
import aiocoap
import asyncio

async def control_leds_silabs_pattern():
    context = await aiocoap.Context.create_client_context()
    device_ipv6 = "fd12:3456::eae:5fff:fe6d:1fe1"
    
    # Toggle all LEDs using Silicon Labs pattern
    for led_num in range(10):
        request = aiocoap.Message(
            code=aiocoap.PUT,  # PUT like Silicon Labs example
            uri=f'coap://[{device_ipv6}]/gpio/led',
            payload=str(led_num).encode()  # Just "0", "1", etc.
        )
        response = await context.request(request).response
        print(f"{response.payload.decode()}")
        await asyncio.sleep(0.5)

asyncio.run(control_leds_silabs_pattern())
```

#### Advantages of Silicon Labs Pattern

✅ **Minimal payload**: 1 byte instead of 7-10 bytes  
✅ **Faster parsing**: Direct character-to-int conversion  
✅ **Less code**: Simpler parsing logic  
✅ **Production proven**: Used in official Silicon Labs applications  
✅ **Standards compliant**: Follows CoAP best practices  
✅ **Bandwidth efficient**: Important for constrained networks  

#### When to Use Which Pattern

| Pattern | Best For | Pros | Cons |
|---------|----------|------|------|
| **Silicon Labs** (`"0"`) | Production systems, battery devices | Ultra-simple, minimal bandwidth | Less explicit, requires documentation |
| **Explicit** (`"led0 on"`) | Development, debugging, human-readable systems | Self-documenting, clear intent | More parsing, larger payload |
| **JSON** (`{"led":0}`) | Complex systems, web integration | Structured, extensible | Heaviest payload, parsing overhead |

#### Reference

This pattern is based on the **Wi-SUN CoAP Meter** application from Silicon Labs:
- Example command: `coap-client -t text -m put -N -B 1 coap://[fd12:3456::be33:acff:fef6:3161]:5683/gpio/led -e "0"`
- Documentation: Silicon Labs Wi-SUN CoAP communication guide
- Pattern: Single resource `/gpio/led`, LED index in payload as single character

**Recommendation**: Use the Silicon Labs pattern for production embedded systems where bandwidth and simplicity matter. Use the explicit `"led0 on"` pattern for user-facing or debugging scenarios where clarity is more important.

## Advanced Topics

### 1. Adding New Sensor Hardware

If you're adding a completely new sensor (e.g., BME680 air quality):

**Step 1: Add Component**
```
Simplicity Studio → Project Configurator → Software Components → 
Search for your sensor driver → Install
```

**Step 2: Add Data Structure**
```c
// In app_coap.c, near other sensor structures:
typedef struct {
  float temperature;
  float humidity;
  float pressure;
  float gas_resistance;
  bool available;
} bme680_data_t;

bme680_data_t latest_bme680_data = {0, 0, 0, 0, false};
uint64_t last_bme680_read_time = 0;
```

**Step 3: Create Read Function**
```c
// Read BME680 sensor data
void read_bme680_data(void)
{
  sl_status_t status;
  int32_t temp, hum, pres;
  uint32_t gas;

  status = sl_bme680_measure_iaq(sl_i2cspm_sensor, &temp, &hum, &pres, &gas);
  
  if (status == SL_STATUS_OK) {
    latest_bme680_data.temperature = temp / 100.0f;
    latest_bme680_data.humidity = hum / 1000.0f;
    latest_bme680_data.pressure = pres / 100.0f;
    latest_bme680_data.gas_resistance = gas;
    latest_bme680_data.available = true;
    last_bme680_read_time = now_sec();
    printf("BME680: Temp=%.2f°C, Hum=%.1f%%, Press=%.2fhPa, Gas=%lu Ohms\n",
           latest_bme680_data.temperature,
           latest_bme680_data.humidity,
           latest_bme680_data.pressure,
           latest_bme680_data.gas_resistance);
  } else {
    latest_bme680_data.available = false;
    printf("BME680 read failed: 0x%04lx\n", status);
  }
}
```

**Step 4: Initialize in app_task()**
```c
// In app.c, app_task() function:
void app_task(void *args)
{
  // ... existing initializations ...
  
  // Initialize BME680
  printf("\n=== Initializing BME680 Sensor ===\n");
  sl_status_t bme680_status = sl_bme680_init(sl_i2cspm_sensor);
  if (bme680_status == SL_STATUS_OK) {
    printf("BME680 initialized successfully\n");
  } else {
    printf("ERROR: BME680 initialization failed! Status: 0x%04lx\n", 
           bme680_status);
  }
  printf("=== BME680 Initialization Complete ===\n\n");

  // ... continue with network connection ...
}
```

**Step 5: Add CoAP Endpoint**
```c
// Create callback for /bme680
sl_wisun_coap_packet_t *coap_callback_bme680(
    const sl_wisun_coap_packet_t *const req_packet)
{
  read_bme680_data();
  
  if (!latest_bme680_data.available) {
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "BME680 sensor unavailable or disconnected");
  } else {
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "{\n"
             "  \"temperature\": %.2f,\n"
             "  \"humidity\": %.1f,\n"
             "  \"pressure\": %.2f,\n"
             "  \"gas_resistance\": %.0f\n"
             "}\n",
             latest_bme680_data.temperature,
             latest_bme680_data.humidity,
             latest_bme680_data.pressure,
             latest_bme680_data.gas_resistance);
  }
  
  return app_coap_reply(coap_response, req_packet);
}

// Register in app_coap_resources_init():
coap_resource.data.uri_path = "/bme680";
coap_resource.data.resource_type = "json";
coap_resource.data.interface = "sensor";
coap_resource.auto_response = coap_callback_bme680;
coap_resource.discoverable = true;
assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
count++;
```

### 2. Asynchronous Operations

For long-running operations (e.g., multi-second sensor warmup), use RTOS:

```c
// Global flag to track operation status
static volatile bool long_operation_complete = false;
static volatile uint32_t long_operation_result = 0;

// Worker thread for long operation
void long_operation_thread(void *args)
{
  // Perform time-consuming task
  osDelay(5000);  // 5 second operation
  long_operation_result = 12345;  // Store result
  long_operation_complete = true;
  osThreadExit();
}

// CoAP callback that triggers async operation
sl_wisun_coap_packet_t *coap_callback_async_operation(
    const sl_wisun_coap_packet_t *const req_packet)
{
  if (!long_operation_complete) {
    // Start operation if not already running
    if (long_operation_result == 0) {
      osThreadNew(long_operation_thread, NULL, NULL);
      snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
               "Operation started, check back later");
    } else {
      snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
               "Operation in progress...");
    }
  } else {
    // Operation complete, return result
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "Result: %lu", long_operation_result);
    // Reset for next operation
    long_operation_complete = false;
    long_operation_result = 0;
  }
  
  return app_coap_reply(coap_response, req_packet);
}
```

### 3. Handling Large Responses

If your response exceeds 1000 bytes (`COAP_MAX_RESPONSE_LEN`):

**Option 1: Increase Buffer Size**
```c
// In app_coap.h:
#define COAP_MAX_RESPONSE_LEN 2000  // Increased from 1000

// Also increase CoAP socket buffer:
// In config/sl_wisun_coap_config.h:
#define SL_WISUN_COAP_RESOURCE_HND_SOCK_BUFF_SIZE 4096UL
```

**Option 2: Split into Multiple Endpoints**
```c
// Instead of one large /data endpoint, create:
// /data/part1
// /data/part2
// /data/part3
```

**Option 3: Use Block-wise Transfer (Advanced)**
```c
// Implement CoAP Block-wise transfer (RFC 7959)
// This requires manual packet handling - consult Wi-SUN CoAP API docs
```

### 4. Custom Error Responses

Create helper function for error responses:

```c
sl_wisun_coap_packet_t *coap_error_reply(
    const char *error_msg,
    const sl_wisun_coap_packet_t *const req_packet)
{
  snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
           "{\"error\": \"%s\"}", error_msg);
  printf("CoAP Error: %s\n", error_msg);
  return app_coap_reply(coap_response, req_packet);
}

// Use in your callbacks:
if (some_error_condition) {
  return coap_error_reply("Sensor not initialized", req_packet);
}
```

### 5. Access Control

Add simple access control based on source address:

```c
bool is_authorized_client(const in6_addr_t *client_addr)
{
  // Example: Only allow requests from border router
  in6_addr_t border_router;
  if (sl_wisun_get_ip_address(SL_WISUN_IP_ADDRESS_TYPE_BORDER_ROUTER,
                               &border_router) == SL_STATUS_OK) {
    return (memcmp(client_addr, &border_router, sizeof(in6_addr_t)) == 0);
  }
  return false;  // Deny if we can't verify
}

// In your callback:
sl_wisun_coap_packet_t *coap_callback_secure_endpoint(
    const sl_wisun_coap_packet_t *const req_packet)
{
  // Check authorization (req_packet contains source address)
  // Note: Actual implementation requires accessing packet internals
  // This is a conceptual example
  
  if (!is_authorized) {
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "ERROR: Unauthorized");
    return app_coap_reply(coap_response, req_packet);
  }
  
  // Proceed with authorized operation
  // ...
}
```

## Common Pitfalls

### 1. ❌ Registering Resources After Network Connection

**Wrong:**
```c
// In app.c, app_task():
void app_task(void *args)
{
  sl_wisun_join(...);
  
  // Wait for connection
  while (join_state != 5) { osDelay(1000); }
  
  app_coap_resources_init();  // TOO LATE!
}
```

**Correct:**
```c
// In app_init.c, app_init():
void app_init(void)
{
  sl_wisun_crash_handler_init();
  app_coap_resources_init();  // BEFORE thread creation
  osThreadNew(app_task, ...);
}
```

### 2. ❌ Buffer Overflow

**Wrong:**
```c
sprintf(coap_response, "Temperature: %f", temp);  // No length check!
```

**Correct:**
```c
snprintf(coap_response, COAP_MAX_RESPONSE_LEN, 
         "Temperature: %.2f", temp);
```

### 3. ❌ Forgetting to Increment Count

**Wrong:**
```c
coap_resource.data.uri_path = "/myendpoint";
// ... configure resource ...
sl_wisun_coap_rhnd_resource_add(&coap_resource);
// Missing count++
```

**Correct:**
```c
coap_resource.data.uri_path = "/myendpoint";
// ... configure resource ...
assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
count++;  // Don't forget!
```

### 4. ❌ Blocking Operations in Callback

**Wrong:**
```c
sl_wisun_coap_packet_t *coap_callback_slow(
    const sl_wisun_coap_packet_t *const req_packet)
{
  osDelay(10000);  // Block for 10 seconds - BAD!
  return app_coap_reply("Done", req_packet);
}
```

**Correct:**
```c
// Use async pattern or ensure operations complete quickly (<1 second)
sl_wisun_coap_packet_t *coap_callback_fast(
    const sl_wisun_coap_packet_t *const req_packet)
{
  // Quick operation
  read_sensor();  // Completes in milliseconds
  return app_coap_reply(coap_response, req_packet);
}
```

### 5. ❌ Not Handling Sensor Failures

**Wrong:**
```c
read_si7021_data();
snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
         "%.2f", latest_si7021_data.temperature);  // What if read failed?
```

**Correct:**
```c
read_si7021_data();
if (!latest_si7021_data.available) {
  snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
           "Sensor unavailable");
} else {
  snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
           "%.2f", latest_si7021_data.temperature);
}
```

### 6. ❌ Exceeding Stack Size

If your callback has many local variables or deep call chains:

```c
// In config/sl_wisun_coap_config.h:
#define SL_WISUN_COAP_RESOURCE_HND_STACK_SIZE_WORD  1024UL  // 4096 bytes

// If still crashes, increase further or reduce local variables
```

### 7. ❌ Duplicate URI Paths

**Wrong:**
```c
coap_resource.data.uri_path = "/sensor/temp";
// ... register ...

// Later in same function:
coap_resource.data.uri_path = "/sensor/temp";  // DUPLICATE!
// ... register again ...
```

Only the first registration will work. Second will fail silently or cause undefined behavior.

## Testing Your Endpoint

### 1. Basic Testing with coap-client

```bash
# GET request
coap-client -m get coap://[device-ipv6]:5683/your/endpoint

# POST request with payload
echo -n "test_data" | coap-client -m post -f- coap://[device-ipv6]:5683/your/endpoint

# PUT request
coap-client -m put -e "new_value" coap://[device-ipv6]:5683/your/endpoint

# Observe (continuous updates)
coap-client -m get -s 10 coap://[device-ipv6]:5683/your/endpoint
```

### 2. Testing with Python

```python
import aiocoap
import asyncio

async def test_endpoint():
    context = await aiocoap.Context.create_client_context()
    
    # GET request
    request = aiocoap.Message(
        code=aiocoap.GET,
        uri=f'coap://[{device_ipv6}]/your/endpoint'
    )
    response = await context.request(request).response
    print(f"Response: {response.payload.decode()}")
    
    # POST request with payload
    request = aiocoap.Message(
        code=aiocoap.POST,
        uri=f'coap://[{device_ipv6}]/your/endpoint',
        payload=b'test_data'
    )
    response = await context.request(request).response
    print(f"Response: {response.payload.decode()}")

asyncio.run(test_endpoint())
```

### 3. Discover All Endpoints

```bash
# Get list of all registered endpoints
coap-client -m get coap://[device-ipv6]:5683/.well-known/core
```

### 4. Monitor with RTT

Connect SEGGER RTT Viewer to see real-time logs:
```
CoAP /your/endpoint Request received
Processing request...
CoAP Response: your_data_here
```

### 5. Automated Testing Script

```bash
#!/bin/bash

DEVICE_IPV6="fd12:3456::eae:5fff:fe6d:1fe1"

# Test all your custom endpoints
echo "Testing /your/endpoint1..."
coap-client -m get coap://[$DEVICE_IPV6]:5683/your/endpoint1

echo "Testing /your/endpoint2..."
coap-client -m get coap://[$DEVICE_IPV6]:5683/your/endpoint2

echo "Testing control endpoint..."
coap-client -m post coap://[$DEVICE_IPV6]:5683/your/control

echo "All tests complete!"
```

## Best Practices

1. **Keep Callbacks Fast**: Aim for <100ms response time
2. **Use JSON for Structured Data**: Easier for clients to parse
3. **Log Requests and Responses**: Helps with debugging
4. **Handle Errors Gracefully**: Always check return values
5. **Document Your Endpoints**: Comment the purpose and response format
6. **Test Incrementally**: Test each endpoint as you add it
7. **Version Your API**: If making breaking changes, use `/v2/endpoint`
8. **Use Consistent Naming**: Pick a convention (`/category/item` or `/item_category`)
9. **Validate Input**: Always check payload length and content
10. **Monitor Resource Usage**: Check stack and heap usage periodically

## Frequently Asked Questions (FAQ)

### Q: Do I need to specify GET, POST, PUT, or DELETE when creating an endpoint?

**A: No!** This is a common point of confusion. When you register a CoAP resource, you **do not specify which methods are allowed**. The registration looks like this:

```c
coap_resource.data.uri_path = "/my/endpoint";
coap_resource.data.resource_type = "json";
coap_resource.data.interface = "sensor";
coap_resource.auto_response = coap_callback_my_function;
coap_resource.discoverable = true;
// Notice: NO METHOD FIELD!
```

Your endpoint will accept **all methods** (GET, POST, PUT, DELETE) by default. The client decides which method to use.

### Q: How do I make a "GET-only" or "POST-only" endpoint?

**A: Add method checking in your callback function:**

```c
sl_wisun_coap_packet_t *coap_callback_get_only(
    const sl_wisun_coap_packet_t *const req_packet)
{
  // Check if GET was used
  if (req_packet->msg_code != COAP_MSG_CODE_REQUEST_GET) {
    snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
             "ERROR: This endpoint only supports GET");
    return app_coap_reply(coap_response, req_packet);
  }
  
  // Normal GET handling
  snprintf(coap_response, COAP_MAX_RESPONSE_LEN, "Data: ...");
  return app_coap_reply(coap_response, req_packet);
}
```

### Q: Why don't the existing endpoints in Silicon Labs example projects check the method?

**A: Simplicity.** For most sensor endpoints, clients naturally use GET. For control endpoints like `/ledon`, clients use POST. Since well-behaved clients use the right method, explicit checking is often unnecessary.

If you're building a public API or need strict REST compliance, add method checking.

### Q: Can one endpoint handle different methods differently?

**A: Yes!** Here's how:

```c
sl_wisun_coap_packet_t *coap_callback_multi_method(
    const sl_wisun_coap_packet_t *const req_packet)
{
  switch (req_packet->msg_code) {
    case COAP_MSG_CODE_REQUEST_GET:
      // Read current state
      snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
               "Current state: %d", state_value);
      break;
      
    case COAP_MSG_CODE_REQUEST_POST:
      // Update state from payload
      state_value = atoi((char*)req_packet->payload_ptr);
      snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
               "State updated to: %d", state_value);
      break;
      
    case COAP_MSG_CODE_REQUEST_DELETE:
      // Reset state
      state_value = 0;
      snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
               "State reset");
      break;
      
    default:
      snprintf(coap_response, COAP_MAX_RESPONSE_LEN,
               "Method not supported");
      break;
  }
  
  return app_coap_reply(coap_response, req_packet);
}
```

### Q: What if I forget to check the method?

**A: Nothing breaks.** The endpoint will respond to any method the same way. For read-only sensors, this is perfectly fine. For control endpoints, it means GET could trigger an action (which may not be RESTful, but it works).

### Q: Do I need different registrations for GET and POST endpoints?

**A: No.** Same registration for all endpoints, regardless of intended method:

```c
// This endpoint will accept GET, POST, PUT, DELETE with same registration
coap_resource.data.uri_path = "/any/endpoint";
coap_resource.data.resource_type = "txt";
coap_resource.data.interface = "node";
coap_resource.auto_response = coap_callback_any_endpoint;
coap_resource.discoverable = true;
assert(sl_wisun_coap_rhnd_resource_add(&coap_resource) == SL_STATUS_OK);
count++;
```

The method is **not part of the endpoint identity**. The URI path alone identifies the endpoint.

## Summary Checklist

When adding a new endpoint to your Wi-SUN project:

- [ ] Define URI path, method, and response format
- [ ] Create callback function in your CoAP handler file (e.g., `app_coap.c`, `wisun_coap.c`)
- [ ] Register resource in `app_coap_resources_init()` (or equivalent registration function)
- [ ] Increment `count++` after registration
- [ ] Build project successfully
- [ ] Flash to device
- [ ] Test with coap-client
- [ ] Verify response format
- [ ] Check RTT logs for errors
- [ ] Document the endpoint

## Additional Resources

- [Wi-SUN CoAP API Documentation](https://docs.silabs.com/wisun/latest/wisun-coap/)
- [RFC 7252: CoAP Protocol](https://tools.ietf.org/html/rfc7252)
- [libcoap Documentation](https://libcoap.net/doc/reference/4.3.0/)
- [Silicon Labs Community Wi-SUN Forum](https://community.silabs.com/s/topic/0TO1M000000qHc6WAE/wisun)

### SDK Header Files Reference

For low-level CoAP implementation details, refer to these SDK header files:

- **`sn_coap_header.h`**: CoAP message codes (GET/POST/PUT/DELETE), message types, protocol version
  - Location: `simplicity_sdk_<version>/app/wisun/component/coap/mbed-coap/sn_coap_header.h`
  - Contains: `sn_coap_msg_code_e` enumeration with all request and response codes

- **`sl_wisun_types.h`**: Wi-SUN specific types and structures
- **`sl_wisun_api.h`**: Wi-SUN API function declarations
- **CoAP Resource Handler**: `wisun_coap_rhnd` component headers for resource management

## Troubleshooting

**Endpoint returns 4.04 Not Found:**
- Verify resource registered in `app_coap_resources_init()`
- Check URI path matches exactly (case-sensitive)
- Ensure initialization happens before network connection

**Device crashes when accessing endpoint:**
- Increase `SL_WISUN_COAP_RESOURCE_HND_STACK_SIZE_WORD`
- Check for buffer overflows (use `snprintf`)
- Reduce local variables in callback

**Response truncated:**
- Increase `COAP_MAX_RESPONSE_LEN` and `SL_WISUN_COAP_RESOURCE_HND_SOCK_BUFF_SIZE`
- Or split into multiple endpoints

**Slow response time:**
- Profile callback execution time
- Move heavy operations to background thread
- Cache frequently-used data

---

**Need Help?**

If you encounter issues:
1. Check RTT console for error messages
2. Verify CoAP handler is running (check logs at boot)
3. Test with simple text endpoint first
4. Consult your project's README and Silicon Labs Wi-SUN documentation
5. Post questions on [Silicon Labs Community](https://community.silabs.com/s/topic/0TO1M000000qHc6WAE/wisun)
6. Review official Wi-SUN CoAP examples in Simplicity Studio
