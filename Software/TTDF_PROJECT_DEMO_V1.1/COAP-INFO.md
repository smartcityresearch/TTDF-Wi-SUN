
# CoAP Info & Status

Example `coap-client-notls` commands and outputs for a Wi‑SUN node. Replace the IPv6 address in brackets with your node's address.

---

## On-Demand Data Fetching Architecture

### Key Principle
Sensor data is **NOT cached or periodically updated**. Instead, each CoAP request triggers a **real-time sensor read**, ensuring:
- ✅ **Data freshness** - Every response contains newly-acquired data
- ✅ **Reduced power consumption** - Sensors idle when not queried
- ✅ **Lower network overhead** - No periodic status broadcasts
- ✅ **Accurate timestamps** - Data read at request time

### Detailed Description

#### How It Works

When a CoAP request is received (e.g., `/sensorStatus` or `/meterParam`), the following sequence occurs:

1. **Request Reception**
   - CoAP packet arrives at the device
   - URI path is parsed to identify the endpoint
   - Request handler function is called

2. **Real-Time Sensor Reading**
   - **Modbus Energy Meter** (`/meterParam`, `/current`)
     - RS485 DE/RE control pin activated
     - Modbus RTU request sent to PZEM-004T
     - Wait for response (~500-800ms for all 6 parameters)
     - Parse register values (voltage, current, power, energy, frequency, PF)
   
   - **Si7021 Temperature/Humidity** (`/si7021`, `/sensorStatus`)
     - I2C transaction initiated
     - Temperature measurement triggered (~50ms)
     - Humidity measurement triggered (~50ms)
     - Values converted to human-readable format
   
   - **VEML6035 Light Sensor** (`/luxData`, `/sensorStatus`)
     - I2C read command sent
     - ALS (Ambient Light Sensor) register read (~100ms)
     - Lux value calculated based on gain/integration settings

3. **Data Processing**
   - Sensor raw values converted to engineering units
   - JSON string formatted with sensor data
   - Network statistics retrieved from Wi-SUN stack
   - Device status information added

4. **Response Transmission**
   - CoAP response packet built
   - JSON payload attached
   - Response sent back to requester

#### Timing Characteristics

| Endpoint | Sensors Read | Typical Response Time |
|----------|--------------|----------------------|
| `/current` | Modbus (1 register) | ~200-300ms |
| `/si7021` | Si7021 I2C | ~100-150ms |
| `/luxData` | VEML6035 I2C | ~100-150ms |
| `/meterParam` | Modbus (6 registers) | ~500-800ms |
| `/sensorStatus` | All sensors + network | ~700-1000ms |
| `/allData` | All sensors + network | ~700-1000ms |

#### Advantages Over Polling Architecture

**Traditional Polling (NOT used in this project):**
```
[Continuous Loop]
  ↓
Read Modbus → Cache data → Wait 1s
  ↓
Read Si7021 → Cache data → Wait 1s
  ↓
Read VEML → Cache data → Wait 1s
  ↓
CoAP request → Return cached data (may be 0-3s old)
```
**Problems:**
- ❌ Data can be stale (up to polling interval old)
- ❌ Continuous sensor activity (power waste)
- ❌ MCU always busy reading sensors
- ❌ I2C/RS485 bus constantly occupied

**On-Demand Fetching (THIS project):**
```
[Idle State - sensors OFF]
  ↓
CoAP request received
  ↓
Read sensors NOW → Format response
  ↓
Return fresh data → Back to idle
```
**Benefits:**
- ✅ Data is always current (read at request time)
- ✅ Sensors sleep between requests
- ✅ MCU can focus on network operations
- ✅ Bus available for other tasks
- ✅ Lower average power consumption

#### Implementation Example

**Code Flow for `/meterParam` Endpoint:**

```c
// app_coap.c - Meter Parameters Handler
sl_wisun_coap_rhnd_resource_hnd_t _coap_meter_param_get_cb(...)
{
    // 1. Activate RS485 transmitter
    GPIO_PinOutSet(gpioPortC, 2);  // DE/RE HIGH
    
    // 2. Request all meter parameters via Modbus
    uint8_t result = ModbusMaster_readInputRegisters(0x0000, 10);
    
    // 3. Read response (~500-800ms wait)
    if (result == ModbusMaster_ku8MBSuccess) {
        // 4. Extract values from Modbus buffer
        voltage = ModbusMaster_getResponseBuffer(0) / 10.0;      // Register 0
        current = ((uint32_t)ModbusMaster_getResponseBuffer(1) << 16 | 
                   ModbusMaster_getResponseBuffer(2)) / 1000.0;  // Registers 1-2
        power = ((uint32_t)ModbusMaster_getResponseBuffer(3) << 16 | 
                 ModbusMaster_getResponseBuffer(4)) / 10.0;      // Registers 3-4
        // ... (energy, frequency, power factor)
        
        // 5. Format JSON response
        snprintf(payload, size,
            "{\n"
            "  \"voltage\": %.1f,\n"
            "  \"current\": %.3f,\n"
            "  \"power\": %.1f,\n"
            "  \"energy\": %.3f,\n"
            "  \"frequency\": %.1f,\n"
            "  \"pf\": %.2f\n"
            "}",
            voltage, current, power, energy, frequency, pf
        );
    }
    
    // 6. Deactivate RS485 transmitter
    GPIO_PinOutClear(gpioPortC, 2);  // DE/RE LOW
    
    // 7. Return response to CoAP client
    return payload;
}
```

**Key Points:**
- No global variables storing sensor data
- No cache refresh timers
- Sensors read only when endpoint is called
- Fresh data guaranteed on every request

#### Error Handling

If a sensor read fails:
- The endpoint returns an error message in JSON
- Example: `{"error": "Modbus read failed", "code": 2}`
- Network stack statistics still included when possible
- Prevents stale data from being returned

#### Power Consumption Impact

**Scenario: 1 CoAP request every 5 minutes**

| Approach | Sensor Activity | Avg Power |
|----------|----------------|-----------|
| Polling (1s interval) | 300 reads in 5 min | ~HIGH |
| On-Demand | 1 read in 5 min | ~LOW |



---

## CoAP Client Command Options

### Basic vs Extended Commands

| Command | Use Case |
|---------|----------|
| `coap-client-notls -m get coap://[device]:5683/current` | Simple, fast endpoints |
| `coap-client-notls -m get -N -B 10 coap://[device]:5683/sensorStatus` | Complex endpoints that read multiple sensors |

### Command Flags Explained

| Flag | Meaning | When to Use |
|------|---------|-------------|
| `-m get` | HTTP-like GET request | All read operations |
| `-N` | **Non-confirmable** mode | Endpoints that take longer to respond |
| `-B 10` | **Block timeout** of 10 seconds | Complex endpoints reading multiple sensors |

### Why Use `-N -B 10` for `/sensorStatus`?

The `/sensorStatus` endpoint reads data from **multiple sources**:
- Modbus energy meter (~500-800ms for all registers)
- Si7021 temperature/humidity sensor (~50ms)
- VEML6035 light sensor (~100ms)
- Wi-SUN network stack APIs

**Total response time**: 700-1000ms

Without `-N -B 10`:
- Default timeout is ~2-3 seconds
- May timeout before all sensors are read
- Confirmable mode adds ACK overhead

With `-N -B 10`:
- 10-second timeout allows ample time
- Non-confirmable reduces round-trip overhead
- More reliable for complex multi-sensor reads

### Quick Reference

```bash
# Fast endpoints (single sensor/simple data) - no extra flags needed
coap-client-notls -m get coap://[device]:5683/current
coap-client-notls -m get coap://[device]:5683/si7021
coap-client-notls -m get coap://[device]:5683/luxData
coap-client-notls -m get coap://[device]:5683/info/all

# Slow endpoints (multi-sensor/complex) - use -N -B 10
coap-client-notls -m get -N -B 10 coap://[device]:5683/sensorStatus
coap-client-notls -m get -N -B 10 coap://[device]:5683/allData
coap-client-notls -m get -N -B 10 coap://[device]:5683/meterParam
```

---

## Info Endpoints

### 1. Get Device ID

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/info/device
```

**Output:**
```json
69de
```

### 2. Get Chip

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/info/chip
```

**Output:**
```
xG28
```

### 3. Get Board

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/info/board
```

**Output:**
```
BRD4401C
```

### 4. Get Device Type

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/info/device_type
```

**Output:**
```
FFN with No LFN support
```

### 5. Get Version

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/info/version
```
**Output:**
```
Compiled on Jan  6 2026 at 16:37:15 (flash partSize 1048576)
```

### 6. Get Application

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/info/application
```

**Output:**
```
xG28 BRD4401C Wi-SUN Node Monitoring V6.2
```

### 7. Get All Info

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/info/all
```

**Output:**
```json
{
  "device": "69de",
  "chip": "xG28",
  "board": "BRD4401C",
  "device_type": "FFN with No LFN support",
  "application": "xG28 BRD4401C Wi-SUN Node Monitoring V6.2",
  "version": "Compiled on Dec 12 2025 at 20:53:22 (flash partSize 1048576)",
  "stack_version": "2.8.0_b436",
  "MAC": "0C:AE:5F:FF:FE:52:69:DE"
}
```

---

## Status Endpoints

### 1. Get Running Time

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/status/running
```

**Output:**
```
0-02:46:40
```

### 2. Get Parent

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/status/parent
```

**Output:**
```
b292
```

### 3. Get Parent Info (with RPL Rank)

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/status/parent_info
```

**Output:**
```json
{
  "tag": "b292",
  "index": 0,
  "type": 1,
  "lifetime": 3600,
  "mac_tx_count": 123,
  "mac_tx_failed_count": 0,
  "mac_tx_ms_count": 0,
  "mac_tx_ms_failed_count": 0,
  "rpl_rank": 256,
  "etx": 128,
  "rsl_in": -45,
  "rsl_out": -47,
  "is_lfn": 0
}
```

**Note:** This endpoint returns detailed parent information including:
- `rpl_rank` - RPL rank of the parent node ([API Reference](https://docs.silabs.com/wisun/latest/wisun-stack-api/sl-wisun-neighbor-info-t#rpl-rank))
- `etx` - Expected Transmission Count (link cost from your node to the parent - lower is better)
- `rsl_in` / `rsl_out` - Received Signal Level (in/out)
- `routing_cost` - Routing cost to parent
- Other neighbor metrics

**Important:** The `etx` value represents the link quality between **your node and the parent**. It measures how many transmissions are expected to successfully send a packet from your node to reach that parent.

*Available after rebuilding firmware with the updated code.*

### 4. Get My RPL Rank

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/status/my_rank
```

**Output:**
```json
{
  "dodag_rank": 384,
  "instance_id": 0
}
```

**Note:** This endpoint returns the node's own RPL information ([API Reference](https://docs.silabs.com/wisun/latest/wisun-stack-api/sl-wisun-rpl-info-t)):
- `dodag_rank` - **The RPL rank of this node itself** ([API Reference](https://docs.silabs.com/wisun/latest/wisun-stack-api/sl-wisun-rpl-info-t#dodag-rank))
- `instance_id` - RPL instance identifier

**RPL Rank Calculation Formula:**
```
Your Node's RPL Rank = Parent's RPL Rank + ETX to Parent
```

This is how RPL works - each node's rank is determined by adding the link cost (ETX) to its parent's rank.

**Example:**
From the `/status/parent_info` endpoint you'll get:
- `rpl_rank`: 256 (parent's rank)
- `etx`: 128 (link cost from your node to parent)

So your node's rank = 256 + 128 = **384**

The ETX measures the quality of the wireless link between your node and its parent. Lower ETX values indicate better link quality.


### 5. Get Neighbor Count

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/status/neighbor
```

**Output:**
```
neighbor_count: 2
```

### Get Neighbor Details by Index (with RPL Rank)

**Command (requires POST with payload):**
```bash
coap-client-notls -m post coap://[fd12:3456::eae:5fff:fe52:69de]:5683/status/neighbor -e "0"
```

**Output:**
```json
{
  "tag": "9d54",
  "index": 0,
  "type": 0,
  "lifetime": 2200,
  "mac_tx_count": 14,
  "mac_tx_failed_count": 0,
  "mac_tx_ms_count": 0,
  "mac_tx_ms_failed_count": 0,
  "rpl_rank": 128,
  "etx": 160,
  "rsl_in": -75,
  "rsl_out": -38,
  "is_lfn": 0
}
```

**Note:** This endpoint returns detailed neighbor information including:
- `tag` - Last 4 hex digits of neighbor's MAC address
- `index` - Neighbor index (0-based, must be < neighbor_count)
- `type` - Neighbor type (0=parent, 1=secondary parent or neighbor)
- `rpl_rank` - RPL rank of the neighbor
- `etx` - Expected Transmission Count (link cost, lower is better)
- `rsl_in` / `rsl_out` - Received Signal Level in dBm
- `lifetime` - Remaining lifetime in seconds
- `is_lfn` - Whether the neighbor is a Low Frequency Node (0=FFN, 1=LFN)

**Important:** Index values are 0-based. If neighbor_count is 2, valid indices are 0 and 1. Requesting index 2 or higher will return no data.

**Example - Query all neighbors:**
```bash
# First, get neighbor count
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/status/neighbor
# Output: neighbor_count: 2

# Query each neighbor (indices 0 to neighbor_count-1)
coap-client-notls -m post coap://[fd12:3456::eae:5fff:fe52:69de]:5683/status/neighbor -e "0"
coap-client-notls -m post coap://[fd12:3456::eae:5fff:fe52:69de]:5683/status/neighbor -e "1"
# Index 2 would return nothing since neighbor_count is 2
```

### 6. Get Connected Time

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/status/connected
```

**Output:**
```
0-02:45:31
```

### 7. Get All Status

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/status/all
```

**Output:**
```json
{
  "running": "0-02:47:15",
  "connected": "0-02:45:38",
  "parent": "b292",
  "neighbor_count": "2"
}
```

### 8. Send Status

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/status/send
```

**Output:**
```
send_asap flag set to true
```

---

## Statistics Endpoints

### 1. Get join state

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/statistics/app/join_state_secs
```

**Output:**
```
4.04
```

### 2.Get Disconnected Total

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/statistics/app/disconnected_total
```

**Output:**
```
0-00:00:00
```

### 3. Get Connections

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/statistics/app/connections
```

**Output:**
```
1 / 1
```

### 4.Get Connected Total

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/statistics/app/connected_total
```

**Output:**
```
0-02:55:09
```

### 5.Get Availability

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/statistics/app/availability
```

**Output:**
```
100.00
```

### 6.Get All Statistics

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/statistics/app/all
```

**Output:**
```json
{
  "join_states_sec":[0,49,11,21,16],
  "connections": "1",
  "network_connections": "1",
  "connected_total": "0-02:55:30",
  "disconnected_total": "0-00:00:00",
  "availability": "100.00"
}
```

### 7. Get Stack PHY Statistics

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/statistics/stack/phy
```

**Output:**
```json
{
  "crc_fails": "0",
  "tx_timeouts": "0",
  "rx_timeouts": "0"
}
```

### 8. Get Stack MAC Statistics

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/statistics/stack/mac
```

**Output:**
```json
{
  "tx_queue_size": "0",
  "tx_queue_peak": "2",
  "rx_count": "774",
  "tx_count": "509",
  "bc_rx_count": "709",
  "bc_tx_count": "372",
  "tx_bytes": "46277",
  "rx_bytes": "78652",
  "tx_failed_count": "0",
  "retry_count": "25",
  "cca_attempts_count": "16",
  "failed_cca_count": "0",
  "rx_ms_count": "0",
  "tx_ms_count": "0",
  "rx_ms_failed_count": "0",
  "tx_ms_failed_count": "0",
  "rx_availability_percentage": "99"
}
```

### 9. Get Stack FHSS Statistics

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/statistics/stack/fhss
```

**Output:**
```json
{
  "drift_compensation": "0",
  "hop_count": "0",
  "synch_interval": "0",
  "prev_avg_synch_fix": "0",
  "synch_lost": "0",
  "unknown_neighbor": "0"
}
```

### 10. Get Stack Wi-SUN Statistics

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/statistics/stack/wisun
```

**Output:**
```json
{
  "pan_control_tx_count": "289",
  "pan_control_rx_count": "396"
}
```

### 11. Get Stack Network Statistics

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/statistics/stack/network
```

**Output:**
```json
{
  "ip_rx_count": "371",
  "ip_tx_count": "214",
  "ip_rx_drop": "0",
  "ip_cksum_error": "0",
  "ip_tx_bytes": "25217",
  "ip_rx_bytes": "43157",
  "ip_routed_up": "0",
  "ip_no_route": "0",
  "frag_rx_errors": "0",
  "frag_tx_errors": "0",
  "rpl_route_routecost_better_change": "1",
  "ip_routeloop_detect": "0",
  "rpl_memory_overflow": "0",
  "rpl_parent_tx_fail": "0",
  "rpl_unknown_instance": "0",
  "rpl_local_repair": "0",
  "rpl_global_repair": "0",
  "rpl_malformed_message": "0",
  "rpl_time_no_next_hop": "0",
  "rpl_total_memory": "1124",
  "buf_alloc": "585",
  "buf_headroom_realloc": "77",
  "buf_headroom_shuffle": "71",
  "buf_headroom_fail": "0",
  "etx_1st_parent": "128",
  "etx_2nd_parent": "133",
  "adapt_layer_tx_queue_size": "0",
  "adapt_layer_tx_queue_peak": "0"
}
```

### 12. Get Stack Regulation Statistics

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/statistics/stack/regulation
```

**Output:**
```json
{
  "arib.tx_duration_ms": "16271"
}
```

---

## Settings Endpoints

### 1. Get Auto Send Interval

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/settings/auto_send
```

**Output:**
```
900
```

### 2. Get Trace Level

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/settings/trace_level
```

**Output:**
```
Error app_set_all_traces 0x1
```

### 3. Get Parameter

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/settings/parameter
```

**Output:**
```
Error : 0xF
```

---

## Reporter Endpoints

### 1. Get Crash Info

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/reporter/crash
```

**Output:**
```
No previous crash info
```

### 2. Start Reporter

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/reporter/start
```

**Output:**
```
started
```

### 3. Stop Reporter

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/reporter/stop
```

**Output:**
```
stopped
```

---

## Sensor Data Endpoints

### 1. Get All Sensor Data

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/allData
```

**Output:**
```json
{
  "current": 0.523,
  "energy": 12345.000,
  "power": 123.4,
  "temperature": 25.34,
  "humidity": 45.67,
  "ALS": 1234,
  "Relay": "ON",
  "lamp": "ON"
}
```

**Note:** This endpoint returns all sensor data from multiple sources:
- Energy meter data (current, energy, power via Modbus)
- Environmental data (temperature, humidity from Si7021)
- Light data (Ambient Light Sensor from VEML6035)
- Control status (Relay and lamp status)

### 2. Get Si7021 Temperature & Humidity

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/si7021
```

**Output:**
```json
{
  "temperature": 25.34,
  "humidity": 45.67
}
```

**Note:** Returns temperature in °C and relative humidity in % from the Si7021 sensor.

### 3. Get Current Measurement

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/current
```

**Output:**
```json
{
  "current": 0.523
}
```

**Note:** Returns the current measurement in Amps from the energy meter via Modbus.

### 4. Get Lux Data

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/luxData
```

**Output:**
```json
{
  "als": 8,
  "white": 16
}
```

**Note:** Returns ambient light sensor data from VEML6035:
- `als` - Ambient Light Sensor value in lux(Uses a photodiode with a filter that approximates the human eye response (V-lambda curve))
- `white` - White light channel value in lux(Measures all visible light plus some near-infrared (IR),No human-eye correction filter)

---

## Control Endpoints

### 1. Turn Relay ON

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/ledon
```

**Output:**
```
Relay turned ON (Relay Status LED OFF)
```

**Note:** Turns the relay ON. The relay is active-low, so this actually turns the Relay Status LED OFF.

### 2. Turn Relay OFF

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/ledoff
```

**Output:**
```
Relay turned OFF (Relay Status LED ON)
```

**Note:** Turns the relay OFF. The relay is active-low, so this actually turns Relay Status LED ON.

---

## Sensor Reinitialization Endpoints

### 1. Reinitialize Si7021 Sensor

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/sensors/reinit_si7021
```

**Output:**
```
Si7021 sensor reinitialized successfully
```

**Note:** This endpoint reinitializes the Si7021 temperature and humidity sensor. Use this endpoint when:
- The sensor is not responding correctly
- You want to restart sensor communications
- The sensor needs to be reset after an error condition

**Example:** If the Si7021 stops returning valid readings, send this command to restart the sensor.

### 2. Reinitialize VEML6035 Lux Sensor

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/sensors/reinit_veml
```

**Output:**
```
VEML6035 sensor reinitialized successfully
```

**Note:** This endpoint reinitializes the VEML6035 (VEML7700) ambient light sensor. Use this endpoint when:
- The sensor is not responding or returning invalid values
- You want to restart sensor communications
- The sensor needs to be reset after an I2C communication error
- The light sensor readings become stuck or unresponsive

**Troubleshooting:**
- If reinitialization fails, check:
  1. External wiring: SCL→PC05, SDA→PC07, VCC→3.3V, GND→GND
  2. I2C address configuration (default 0x10 for VEML6035)
  3. Pull-up resistors on SCL/SDA lines

---

## Energy Meter Endpoints

### PZEM-004T Modbus Register Map

The energy meter (PZEM-004T) communicates via Modbus RTU using **Input Registers (Function Code 0x04)**.

| Parameter | Register Address | Size | Resolution | Unit | Formula |
|-----------|------------------|------|------------|------|---------|
| Voltage | 0x0000 | 1 register (16-bit) | 0.1 | V | value × 0.1 |
| Current | 0x0001-0x0002 | 2 registers (32-bit) | 0.001 | A | value × 0.001 |
| Power | 0x0003-0x0004 | 2 registers (32-bit) | 0.1 | W | value × 0.1 |
| Energy | 0x0005-0x0006 | 2 registers (32-bit) | 1 | Wh | value × 1 |
| Frequency | 0x0007 | 1 register (16-bit) | 0.1 | Hz | value × 0.1 |
| Power Factor | 0x0008 | 1 register (16-bit) | 0.01 | - | value × 0.01 |

**Modbus Configuration:**
| Parameter | Value |
|-----------|-------|
| Function Code | 0x04 (Read Input Registers) |
| Slave Address | 1 (default) |
| Baud Rate | 9600 |
| Data Bits | 8 |
| Stop Bits | 1 |
| Parity | None |

### 1. Get Meter Parameters

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/meterParam
```

**Output:**
```json
{
  "voltage": 230.5,
  "current": 0.523,
  "frequency": 50.0,
  "power": 123.4,
  "energy": 12345.000,
  "power_factor": 0.98
}
```

**Note:** Returns complete energy meter parameters read via Modbus:
- `voltage` - Voltage in Volts
- `current` - Current in Amps
- `frequency` - Frequency in Hz
- `power` - Power in Watts
- `energy` - Energy in Watt-hours
- `power_factor` - Power factor (0-1)

---

## Unified Sensor Status Endpoint

### Overview

The `/sensorStatus` endpoint is a comprehensive, all-in-one endpoint that returns **actual real-time values** from all sensors and network status in a single request.

- **URI**: `/sensorStatus`
- **Method**: GET (CoAP)
- **Response Type**: JSON
- **Content Type**: application/json

### Command

```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/sensorStatus
```

### Response Example

```json
{
  "lux": 450,
  "humidity": 52.9,
  "temperature": 24.38,
  "rpl": 512,
  "ip_v6": "fd12:3456::eae:5fff:fe52:69de",
  "availability": 99.87,
  "connectivity": "0-02:45:31",
  "disconnectivity": "0-00:05:12",
  "rsl_in": -48,
  "rsl_out": -45,
  "current": 20.27,
  "power": 79.53,
  "energy": 33.71,
  "relay_status": "ON",
  "lamp_status": "ON"
}
```

### Field Descriptions

#### Environmental Sensors (Real-time values)
| Field | Description | Unit |
|-------|-------------|------|
| `lux` | Ambient light level from VEML6035 sensor | lux |
| `humidity` | Relative humidity from Si7021 sensor | % |
| `temperature` | Temperature from Si7021 sensor | °C |

#### Network Parameters (Actual values from Wi-SUN stack)
| Field | Description | Unit |
|-------|-------------|------|
| `rpl` | RPL DODAG rank from `sl_wisun_get_rpl_info()` | - |
| `ip_v6` | Device's global IPv6 address | - |
| `availability` | Network availability (connected_time / total_time × 100) | % |
| `connectivity` | Current connection duration | d-hh:mm:ss |
| `disconnectivity` | Total disconnected time | d-hh:mm:ss |
| `rsl_in` | Incoming Radio Signal Level from parent | dBm |
| `rsl_out` | Outgoing Radio Signal Level to parent | dBm |

#### Power/Energy Parameters (Real-time values via Modbus)
| Field | Description | Unit |
|-------|-------------|------|
| `current` | Current measurement from energy meter | A |
| `power` | Power consumption from energy meter | W |
| `energy` | Total energy from energy meter | Wh |

#### Control Status (Real-time values)
| Field | Description |
|-------|-------------|
| `relay_status` | Current relay state ("ON" or "OFF") |
| `lamp_status` | Lamp state based on current detection ("ON" if current > 0) |

### Parameter to Source Mapping

| Parameter | Source Endpoint | API/Function Used |
|-----------|----------------|-------------------|
| `lux` | `/luxData` | `sl_veml6035_get_als_lux()` |
| `humidity` | `/si7021` | `sl_si70xx_measure_rh_and_temp()` |
| `temperature` | `/si7021` | `sl_si70xx_measure_rh_and_temp()` |
| `rpl` | `/status/my_rank` | `sl_wisun_get_rpl_info()` → `rpl_info.dodag_rank` |
| `ip_v6` | Device info | `sl_wisun_get_ip_address()` + `sl_wisun_ip6tos()` |
| `availability` | `/statistics/app/availability` | `connected_total_sec / total_time × 100` |
| `connectivity` | `/status/connected` | `dhms(now_sec() - connection_time_sec)` |
| `disconnectivity` | Statistics | `dhms(disconnected_total_sec)` |
| `rsl_in` | `/status/parent_info` | `parent_info.rsl_in - 174` (dBm) |
| `rsl_out` | `/status/parent_info` | `parent_info.rsl_out - 174` (dBm) |
| `current` | `/meterParam` | `read_meter_data()` via Modbus |
| `power` | `/meterParam` | `read_meter_data()` via Modbus |
| `energy` | `/meterParam` | `read_meter_data()` via Modbus |
| `relay_status` | `/ledon`, `/ledoff` | `led1_on` flag state |
| `lamp_status` | Derived | `"ON"` if `current > 0`, else `"OFF"` |

### Before vs After Comparison

**Before: Multiple Requests Required**
```bash
# To get all this data, you needed multiple CoAP requests:
coap-client-notls -m get coap://[device]:5683/si7021
coap-client-notls -m get coap://[device]:5683/luxData
coap-client-notls -m get coap://[device]:5683/current
coap-client-notls -m get coap://[device]:5683/meterParam
coap-client-notls -m get coap://[device]:5683/status/my_rank
coap-client-notls -m get coap://[device]:5683/status/parent_info
coap-client-notls -m get coap://[device]:5683/statistics/app/availability
coap-client-notls -m get coap://[device]:5683/status/connected
# ... 8+ separate requests!
```

**After: Single Request**
```bash
# Now you get everything in one request:
coap-client-notls -m get coap://[device]:5683/sensorStatus
```

### Data Freshness

All sensor data is read **on-demand** when the endpoint is called:
- **Meter data**: `read_meter_data()` - reads fresh from Modbus
- **Si7021 data**: `read_si7021_data()` - reads fresh from I2C
- **VEML6035 data**: `read_veml_data()` - reads fresh from I2C
- **Network stats**: Retrieved live from Wi-SUN stack APIs

This ensures the response contains the most current data available.

### Benefits

- ✅ **Reduced Network Traffic**: 1 request instead of 8+ requests
- ✅ **Lower Latency**: Single round-trip instead of multiple
- ✅ **Atomic Snapshot**: All data from approximately the same point in time
- ✅ **Simpler Client Code**: One API call to get all sensor/status data
- ✅ **Bandwidth Efficiency**: Single CoAP response with all data
- ✅ **Real-time Data**: All sensors read fresh on-demand

### Perfect for:
- Dashboard applications
- Monitoring systems
- Data logging
- Status displays
- IoT platforms

### Notes
- RPL rank of 0 means not yet joined to the network
- IPv6 shows "N/A" if not yet assigned
- RSL values of 0 mean no parent connection established
- Availability is calculated as: (time_connected / total_time) × 100

---

## Multicast OTA Endpoints

### Get Multicast OTA Missed Packets

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/multicast_ota/missed
```

**Note:** Returns information about missed packets during multicast OTA updates.

### Get Multicast OTA RX Information

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/multicast_ota/rx
```

**Note:** Returns reception information for multicast OTA updates.

### Get Multicast OTA Info

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/multicast_ota/info
```

**Note:** Returns general information about multicast OTA status and configuration.

---

## Notes

- Replace `[fd12:3456::eae:5fff:fe52:69de]` with your node's IPv6 address
- The `-m get` option performs a GET request
- `coap-client-notls` is the unencrypted CoAP client
- `/info/*` endpoints return device metadata
- `/status/*` endpoints return runtime and network state
- `/statistics/*` endpoints return application statistics and metrics
- `/settings/*` endpoints allow configuration retrieval
- `/reporter/*` endpoints control and query reporting functions
- `/allData` - Combined sensor data endpoint
- `/si7021` - Temperature and humidity sensor endpoint
- `/luxData` - Light sensor endpoint
- `/meterParam` - Energy meter parameters endpoint
- `/ledon`, `/ledoff` - Relay control endpoints
- `/multicast_ota/*` - OTA update monitoring endpoints

