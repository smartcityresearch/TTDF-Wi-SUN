# TTDF WiSUN IoT System - Complete Flowchart

## Project Overview
**TTDF_PROJECT_DEMO_V1** - Wi-SUN Smart Energy Monitoring & Control System

This flowchart represents the complete system architecture and operational flow of the Wi-SUN IoT monitoring and control application built on Silicon Labs EFR32FG28 wireless MCU.

---

## System Architecture Flowchart

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
