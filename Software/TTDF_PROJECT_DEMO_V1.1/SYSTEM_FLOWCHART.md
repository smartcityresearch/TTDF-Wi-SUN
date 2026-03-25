# TTDF WiSUN IoT System - Complete Flowchart

## Project Overview
**TTDF_PROJECT_DEMO_V1** - Wi-SUN Smart Energy Monitoring & Control System

This flowchart represents the complete system architecture and operational flow of the Wi-SUN IoT monitoring and control application built on Silicon Labs EFR32FG28 wireless MCU.

---

## Project Structure

```
TTDF_PROJECT_DEMO_V1.1/
│
├── main.c                              # Entry point - system initialization
├── app.c                               # Main application logic and state machine
├── app.h                               # Application header file
├── app_init.c                          # Application initialization (RTOS thread)
│
├── app_coap.c                          # CoAP request handlers
├── app_coap.h                          # CoAP function prototypes
│
├── modbusmaster.c                      # Modbus RTU master for energy meter
├── modbusmaster.h                      # Modbus function prototypes
│
├── app_tcp_server.c                    # TCP server implementation
├── app_tcp_server.h                    # TCP server header
├── app_udp_server.c                    # UDP server implementation
├── app_udp_server.h                    # UDP server header
├── app_direct_connect.c                # Direct Connect peer-to-peer
├── app_direct_connect.h                # Direct Connect header
├── app_check_neighbors.c               # Neighbor discovery & monitoring
├── app_check_neighbors.h               # Neighbor monitoring header
│
├── app_parameters.c                    # Persistent settings (NVM3)
├── app_parameters.h                    # Parameters header
├── app_timestamp.c                     # Time tracking utilities
├── app_timestamp.h                     # Timestamp header
├── app_reporter.c                      # RTT trace event reporter
├── app_reporter.h                      # Reporter header
├── app_rtt_traces.c                    # Debug trace filtering
├── app_rtt_traces.h                    # RTT traces header
├── sl_wisun_crash_handler.c            # Crash detection & logging
├── sl_wisun_crash_handler.h            # Crash handler header
├── app_list_configs.c                  # RF configuration listing
├── app_list_configs.h                  # Config list header
│
├── app_wisun_multicast_ota.c           # Multicast OTA DFU handler
├── app_wisun_multicast_ota.h           # OTA header
│
├── sl_wisun_regdb.c                    # Regional frequency database
│
├── config/                             # Configuration files folder
│   ├── app_project_info_config.h       # Project metadata
│   ├── app_properties_config.h         # App version (for OTA)
│   ├── pin_config.h                    # GPIO pin assignments
│   ├── sl_i2cspm_sensor_config.h       # I2C bus configuration
│   ├── psa_crypto_config.h             # Security configuration
│   ├── os_cfg.h                        # OS configuration
│   ├── rtos_cfg.h                      # RTOS settings
│   └── ...
│
├── autogen/                            # Auto-generated files
│   ├── sl_wisun_config.c               # Wi-SUN stack config
│   ├── sl_wisun_config.h               # Wi-SUN config header
│   ├── sl_board_default_init.c         # Board initialization
│   ├── rail_config.c                   # Radio configuration
│   ├── rail_config.h                   # Radio config header
│   ├── sl_event_handler.c              # Event dispatcher
│   ├── sl_event_handler.h              # Event handler header
│   ├── sl_iostream_init_eusart_instances.c  # UART initialization
│   ├── sl_iostream_init_eusart_instances.h  # UART init header
│   ├── sl_simple_led_instances.c       # LED control
│   ├── sl_simple_led_instances.h       # LED header
│   └── ...
│
├── README.md                           # Complete project guide
├── SYSTEM_FLOWCHART.md                 # This file - flow diagrams
├── COAP-INFO.md                        # CoAP API reference
├── PIN_REFERENCE.md                    # Pin configuration table
├── RPL_INSTANCE_ID.md                  # Routing instance info
├── OTA_DFU_SETUP.md                    # OTA update guide
├── TTDF_PROJECT_CIRUIT_DIAGRAM.png     # Hardware circuit diagram
│
├── simplicity_sdk_2025.6.2/            # Silicon Labs SDK
├── GNU ARM v12.2.1 - Debug/            # Build output directory
│
├── TTDF_PROJECT_DEMO_V1.slcp           # Simplicity Studio project file
├── TTDF_PROJECT_DEMO_V1.1.pintool      # Pin configuration tool
├── .cproject                           # Eclipse CDT project
├── .project                            # Eclipse project
└── .slps                               # Simplicity Studio project state
```

### Key Files Explained:

| File | Purpose |
|------|---------|
| **main.c** | Entry point, calls `app_init()` and `app_process_action()` |
| **app.c** | Main application loop, Wi-SUN event handling, join state management |
| **app_init.c** | Creates RTOS task for application |
| **app_coap.c** | All CoAP endpoints - sensor data acquisition & relay control |
| **modbusmaster.c** | Modbus RTU master for energy meter communication |
| **app_parameters.c** | Persistent parameters stored in NVM3 |
| **sl_wisun_crash_handler.c** | Detects and logs system crashes |

---

## Simplified System Flow

```mermaid
flowchart TD
    Start([System Start]) --> Init[Initialize System<br/>Silicon Labs Stack<br/>RTOS Thread]
    
    Init --> HWInit[Initialize Hardware<br/>- Crash Handler<br/>- CoAP Resources<br/>- Relay Control]
    
    HWInit --> SensorInit[Initialize Sensors<br/>- Modbus Energy Meter RS485<br/>- Si7021 Temp/Humidity I2C<br/>- VEML6035 Light Sensor I2C]
    
    SensorInit --> Network[Connect to Wi-SUN Network<br/>Wait for OPERATIONAL state]
    
    Network --> OpenComm[Open Communication Sockets<br/>UDP & CoAP]
    
    OpenComm --> MainLoop{Main Loop<br/>Continuous Operation}
    
    MainLoop --> CheckConn{Network<br/>Connected?}
    
    CheckConn -->|Yes| HandleReq[Handle Incoming Requests<br/>TCP/UDP/CoAP]
    CheckConn -->|No| Wait[Wait & Monitor]
    
    HandleReq --> ProcessReq[Process CoAP Requests<br/>Read Sensors On-Demand<br/>Control Relay<br/>Return Data]
    
    Wait --> MainLoop
    ProcessReq --> SendStatus[Send Status Updates<br/>if Auto-Send Enabled]
    
    SendStatus --> MainLoop
    
    subgraph CoAP["CoAP API Endpoints"]
        API1["/sensorStatus - All sensor data"]
        API2["/meterParam - Energy meter"]
        API3["/si7021 - Temperature/Humidity"]
        API4["/luxData - Light sensor"]
        API5["/ledon, /ledoff - Relay control"]
    end
    
    subgraph Sensors["Connected Sensors"]
        S1["PZEM-004T Energy Meter<br/>RS485/Modbus"]
        S2["Si7021<br/>I2C Temp/Humidity"]
        S3["VEML6035<br/>I2C Light Sensor"]
        S4["Relay/LED Control<br/>GPIO"]
    end
    
    style Start fill:#90EE90
    style MainLoop fill:#FFB6C1
    style Network fill:#98FB98
    style ProcessReq fill:#87CEEB
    style CoAP fill:#E6E6FA
    style Sensors fill:#F0E68C
```

---

## Detailed System Flow

<details>
<summary>Click to expand detailed flowchart</summary>

```mermaid
flowchart TD
    Start([System Start]) --> MainInit[main.c]
    MainInit --> SLInit[sl_main_second_stage_init<br/>Initialize Silicon Labs Stack]
    SLInit --> AppInit[app_init.c]
    
    AppInit --> CrashHandlerInit[Initialize Crash Handler]
    CrashHandlerInit --> CoapInit[Initialize CoAP Resources]
    CoapInit --> CreateAppThread[Create app_task Thread<br/>5*2048 bytes stack]
    CreateAppThread --> AppTask[app_task Function]
    
    AppTask --> TimestampInit[Initialize Timestamp]
    TimestampInit --> LoadParams[Load Application Parameters]
    LoadParams --> InitRelay[Initialize Relay LED to OFF]
    InitRelay --> CheckCrash{Previous<br/>Crash?}
    
    CheckCrash -->|Yes| IncrementCrash[Increment Crash Counter<br/>Log Crash Info]
    CheckCrash -->|No| DeviceInfo
    IncrementCrash --> DeviceInfo[Get Device Info<br/>MAC, Tag, Type]
    
    DeviceInfo --> InitModbus[Initialize Modbus Master<br/>PZEM-004T Energy Meter<br/>RS485 @ 9600 baud]
    InitModbus --> InitSi7021[Initialize Si7021<br/>Temperature/Humidity Sensor]
    InitSi7021 --> InitVEML[Initialize VEML6035<br/>Ambient Light Sensor]
    
    InitVEML --> TCPCheck{TCP Server<br/>Enabled?}
    TCPCheck -->|Yes| InitTCP[Initialize TCP Server]
    TCPCheck -->|No| UDPCheck
    InitTCP --> UDPCheck
    
    UDPCheck{UDP Server<br/>Enabled?}
    UDPCheck -->|Yes| InitUDP[Initialize UDP Server]
    UDPCheck -->|No| WiSUNConnect
    InitUDP --> WiSUNConnect
    
    WiSUNConnect[Call sl_wisun_app_core_network_connect<br/>Register Event Callbacks]
    WiSUNConnect --> WaitConnection{Join State =<br/>OPERATIONAL?}
    
    WaitConnection -->|No| DirectConnectCheck{Direct Connect<br/>Enabled?}
    DirectConnectCheck -->|Yes| CheckDirectUDP[Check UDP Messages for<br/>Direct Connect]
    DirectConnectCheck -->|No| DelayWait
    CheckDirectUDP --> DelayWait[Delay 1 Second]
    DelayWait --> WaitConnection
    
    WaitConnection -->|Yes| Connected[Connected to WiSUN Network]
    Connected --> OpenSockets[Open UDP/CoAP Notification Sockets]
    OpenSockets --> PrintHelp[Print CoAP Help Information]
    PrintHelp --> SendConnectMsg[Send Initial Connection Message]
    SendConnectMsg --> MainLoop
    
    MainLoop{Main While Loop} --> GetJoinState[Get Current Join State]
    GetJoinState --> Operational{Join State =<br/>OPERATIONAL?}
    
    Operational -->|Yes| SetFlags[Set UDP/CoAP Flags to TRUE]
    SetFlags --> CheckTCPMsg{TCP Server<br/>Enabled?}
    CheckTCPMsg -->|Yes| ProcTCP[Check TCP Server Messages]
    CheckTCPMsg -->|No| CheckUDPMsg
    ProcTCP --> CheckUDPMsg
    
    CheckUDPMsg{UDP Server<br/>Enabled?}
    CheckUDPMsg -->|Yes| ProcUDP[Check UDP Server Messages]
    CheckUDPMsg -->|No| AutoSend
    ProcUDP --> AutoSend
    
    Operational -->|No| DisconnectedFlow[Set UDP/CoAP Flags to FALSE]
    DisconnectedFlow --> DirectCheck2{Direct Connect<br/>Enabled?}
    DirectCheck2 -->|Yes| CheckUDPDirect[Check UDP for Direct Connect]
    DirectCheck2 -->|No| AutoSend
    CheckUDPDirect --> AutoSend
    
    AutoSend{Auto Send<br/>Period?} -->|Yes| SendStatus[Send Status JSON Message]
    AutoSend -->|No| TrackHeap
    SendStatus --> TrackHeap
    
    TrackHeap{Heap Tracking<br/>Time?}
    TrackHeap -->|Yes| UpdateHeap[Update Heap Statistics]
    TrackHeap -->|No| Dispatch
    UpdateHeap --> Dispatch
    
    Dispatch[Dispatch RTOS Thread]
    Dispatch --> MainLoop
    
    subgraph CoAPEndpoints[CoAP Request Handlers - On-Demand Data]
        CoapReq[CoAP Request Received] --> ParseURI{Parse URI Path}
        
        ParseURI -->|/sensorStatus| ReadAllSensors[Read All Sensor Data]
        ReadAllSensors --> ReturnAll[Return Unified JSON:<br/>Energy + Environmental +<br/>Light + Network Stats]
        
        ParseURI -->|/meterParam| ReadMeter[Read Modbus Energy Meter<br/>Voltage, Current, Power,<br/>Energy, Frequency, PF]
        ReadMeter --> ReturnMeter[Return Meter JSON]
        
        ParseURI -->|/si7021| ReadTemp[Read Si7021<br/>Temperature & Humidity]
        ReadTemp --> ReturnTemp[Return Temp/Humidity JSON]
        
        ParseURI -->|/luxData| ReadLux[Read VEML6035<br/>Ambient Light Sensor]
        ReadLux --> ReturnLux[Return Lux JSON]
        
        ParseURI -->|/current| ReadCurrent[Read Modbus Current]
        ReadCurrent --> ReturnCurrent[Return Current JSON]
        
        ParseURI -->|/ledon| TurnRelayOn[Turn Relay ON<br/>Control LED]
        TurnRelayOn --> ReturnOK1[Return Success]
        
        ParseURI -->|/ledoff| TurnRelayOff[Turn Relay OFF<br/>Control LED]
        TurnRelayOff --> ReturnOK2[Return Success]
        
        ParseURI -->|/info/*| GetInfo[Return Device Info<br/>Version, MAC, etc.]
        
        ParseURI -->|/status/*| GetStatus[Return Network Status<br/>Parent, Rank, Statistics]
        
        ParseURI -->|/settings/*| UpdateSettings[Update Settings<br/>Trace Level, Parameters]
    end
    
    subgraph EventCallbacks[WiSUN Event Callbacks]
        Event[WiSUN Event Triggered] --> EventType{Event Type}
        
        EventType -->|JOIN_STATE| JoinStateHandler[_join_state_custom_callback]
        JoinStateHandler --> UpdateState[Update Join State]
        UpdateState --> StateCheck{New State?}
        StateCheck -->|Yes| LogTransition[Log State Transition<br/>Calculate Delay]
        StateCheck -->|No| EventEnd
        
        LogTransition --> IsOperational{State =<br/>OPERATIONAL?}
        IsOperational -->|Yes| TurnLEDOn[Turn Network LED ON<br/>Increment Connection Count<br/>Update Statistics]
        IsOperational -->|No| WasOperational
        
        TurnLEDOn --> GetParent[Get Parent MAC/Info]
        GetParent --> SendReconnect[Send Reconnection Message]
        SendReconnect --> SetTracesLow[Set Traces to Low Level]
        SetTracesLow --> EventEnd
        
        WasOperational{Was Previously<br/>OPERATIONAL?}
        WasOperational -->|Yes| Disconnected[Log Disconnection<br/>Update Disconnect Time<br/>Increase Trace Level]
        WasOperational -->|No| EventEnd
        Disconnected --> EventEnd[Return]
        
        EventType -->|TCP Event| TCPHandler[_tcp_custom_callback]
        EventType -->|UDP Event| UDPHandler[_udp_custom_callback]
        EventType -->|Direct Connect| DCHandler[app_direct_connect_custom_callback]
    end
    
    subgraph SensorModules[Sensor/Actuator Modules]
        ModbusM[Modbus Master Module<br/>modbusmaster.c]
        ModbusM -.->|RS485| EnergyMeter[PZEM-004T Energy Meter]
        
        I2CBus[I2C Bus Module]
        I2CBus -.->|I2C| Si7021Sensor[Si7021 Temp/Humidity]
        I2CBus -.->|I2C| VEMLSensor[VEML6035 Lux Sensor]
        
        GPIOCtrl[GPIO Control]
        GPIOCtrl -.->|Digital Out| RelayControl[Relay LED Control]
    end
    
    style Start fill:#90EE90
    style MainLoop fill:#87CEEB
    style CoAPEndpoints fill:#E6E6FA
    style EventCallbacks fill:#FFE4B5
    style SensorModules fill:#F0E68C
    style Connected fill:#98FB98
    style Operational fill:#87CEEB
```

</details>

---

## Key System Components

### 1. Initialization Phase
- **Silicon Labs Stack**: Core Wi-SUN protocol stack initialization
- **Crash Handler**: Monitors and logs system crashes
- **CoAP Resources**: REST API endpoint initialization
- **RTOS Thread**: Creates main application task with 10KB stack

### 2. Hardware Initialization
- **Modbus Master**: RS485 communication with PZEM-004T energy meter
- **Si7021 Sensor**: I2C temperature and humidity sensor
- **VEML6035 Sensor**: I2C ambient light sensor
- **Relay Control**: GPIO-based relay/LED control

### 3. Network Connection
- **Wi-SUN Network**: Connects to mesh network
- **Join States**: Monitors connection progress (0-5)
- **Socket Management**: Opens UDP/CoAP notification channels

### 4. Main Operational Loop
- **Join State Monitoring**: Continuously checks network connection status
- **Server Message Handling**: Processes TCP/UDP incoming requests
- **Periodic Tasks**: Auto-send status updates, heap tracking
- **RTOS Dispatch**: Thread scheduling and task management

### 5. CoAP API Endpoints (On-Demand Data Acquisition)

#### Sensor Data Endpoints:
- `/sensorStatus` - Unified sensor status (all data in one request)
- `/meterParam` - Complete energy meter parameters
- `/si7021` - Temperature and humidity
- `/luxData` - Ambient light level
- `/current` - Real-time current measurement

#### Control Endpoints:
- `/ledon` - Turn relay ON
- `/ledoff` - Turn relay OFF

#### Information Endpoints:
- `/info/*` - Device information (MAC, version, chip, etc.)
- `/status/*` - Network status (parent, rank, statistics)
- `/settings/*` - Configuration management

### 6. Event-Driven Callbacks
- **Join State Events**: Connection/disconnection handling
- **TCP/UDP Events**: Server communication events
- **Direct Connect Events**: Peer-to-peer communication

### 7. Hardware Interfaces
- **RS485/Modbus RTU**: Energy meter communication
- **I2C Bus**: Sensor communication (Si7021, VEML6035)
- **GPIO**: Relay/LED control

---

## Unique Features

### On-Demand Data Acquisition Strategy
Unlike traditional polling systems, this implementation reads sensor data **only when CoAP requests are received**:

**Benefits**:
- ⚡ **Reduced Power Consumption**: Sensors idle when not queried
- 📡 **Lower Network Traffic**: No periodic status broadcasts
- 🔄 **Fresh Data Guarantee**: Every response contains newly-read data
- 🎯 **Efficient Resource Usage**: MCU and bus resources freed between requests

### Dual Join State Checks

1. **Initial Connection Wait** (Startup):
   - Blocking loop waiting for first connection
   - System cannot proceed until network is operational
   
2. **Runtime Connection Monitoring** (Main Loop):
   - Continuous monitoring for disconnections/reconnections
   - Adaptive behavior based on connection status

---

## Technical Specifications

| Component | Details |
|-----------|---------|
| **MCU** | EFR32FG28B322F1024IM68 |
| **Radio Board** | BRD4401C |
| **SDK** | Simplicity SDK v2025.6.2 |
| **Wireless Protocol** | Wi-SUN FAN 1.1 |
| **Frequency** | Sub-GHz (868/915 MHz) |
| **API Protocol** | CoAP (RFC 7252) |
| **RTOS** | Micrium OS |
| **Main Task Stack** | 10,240 bytes (5×2048) |

---

## Usage Instructions

### Viewing the Diagram

#### Option 1: GitHub (Recommended)
1. Push this file to GitHub repository
2. GitHub will automatically render the Mermaid diagram

#### Option 2: VS Code
1. Install **Markdown Preview Mermaid Support** extension
2. Open this file in VS Code
3. Press `Ctrl+Shift+V` to preview

#### Option 3: Online Mermaid Editor
1. Visit [Mermaid Live Editor](https://mermaid.live/)
2. Copy the Mermaid code
3. Paste and edit
4. Export as SVG/PNG/PDF

#### Option 4: Documentation Tools
- **GitLab**: Native Mermaid support
- **Confluence**: Use Mermaid macro
- **Notion**: Use Mermaid code blocks
- **Obsidian**: Native Mermaid support

### Exporting for Reports
1. Open in Mermaid Live Editor
2. Export as **SVG** (vector graphics, scalable)
3. Import into:
   - Microsoft Word
   - LaTeX documents
   - PowerPoint presentations
   - PDF reports

---

## File Information

**Created**: March 2, 2026  
**Project**: TTDF_PROJECT_DEMO_V1.1  
**Technology**: Wi-SUN Mesh Networking, IoT, Embedded Systems  
**License**: Zlib (Silicon Labs)

---

## Related Documentation

- [README.md](README.md) - Complete project documentation
- [COAP-INFO.md](COAP-INFO.md) - CoAP API reference
- [OTA_DFU_SETUP.md](OTA_DFU_SETUP.md) - Over-the-air firmware update guide
- [PIN_REFERENCE.md](PIN_REFERENCE.md) - Hardware pin configuration
- [RPL_INSTANCE_ID.md](RPL_INSTANCE_ID.md) - Routing protocol configuration

---

## Notes

- This flowchart represents the complete system flow from initialization to runtime operation
- Color coding indicates different operational phases and component types
- Subgraphs group related functionality (CoAP handlers, event callbacks, sensor modules)
- The diagram can be split into smaller diagrams for specific documentation needs

---

**For questions or modifications, contact the development team.**
