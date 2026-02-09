# TTDF_PROJECT_DEMO_V1

## Project Overview

TTDF_PROJECT_DEMO_V1 is an advanced Industrial IoT demonstration project built on Silicon Labs' Wi-SUN FAN (Field Area Network) technology. This project showcases a complete implementation of wireless mesh networking for industrial monitoring and control applications, featuring Modbus protocol integration for seamless communication with industrial equipment and sensors.

The project is specifically designed for Silicon Labs EFR32ZG28 Series 2 wireless SoCs and demonstrates best practices for building robust, scalable industrial IoT solutions with Wi-SUN mesh networking.

## Key Features

### Networking & Connectivity
- **Wi-SUN FAN Mesh Network:** Full implementation of IEEE 802.15.4g-based mesh networking with automatic routing and self-healing capabilities
- **CoAP Protocol Support:** Lightweight RESTful communication protocol optimized for constrained devices and networks
- **TCP/UDP Server:** Dual-stack server implementation for flexible data exchange with external systems
- **Multicast Communication:** Efficient one-to-many communication for network-wide operations
- **Direct Connect Capability:** Point-to-point connection support for specialized use cases

### Industrial Protocols
- **Modbus Master Implementation:** Industry-standard Modbus protocol support for communicating with PLCs, sensors, and actuators
- **Multiple Endpoint Support:** Flexible endpoint mapping system for diverse device integration
- **Real-time Data Acquisition:** High-performance sensor reading and data collection

### Network Management
- **Neighbor Discovery:** Automatic detection and tracking of nearby Wi-SUN nodes
- **Network Diagnostics:** Comprehensive network health monitoring and reporting
- **RPL Instance Management:** Proper routing protocol configuration for optimal mesh performance
- **Crash Handler:** Robust error handling and recovery mechanisms

### Firmware & Updates
- **OTA (Over-The-Air) Updates:** Wireless firmware updates via multicast for entire network segments
- **Bootloader Integration:** Secure bootloader configuration for safe firmware management

### Monitoring & Debugging
- **RTT (Real-Time Transfer) Traces:** High-speed debug output without UART overhead
- **Application Reporter:** Structured logging and event reporting system
- **Timestamp Management:** Accurate time synchronization across the network
- **Configuration Listing:** Runtime configuration inspection and management

## Hardware Requirements

- **Board:** Silicon Labs EFR32ZG28 development board (BRD2705A, BRD4163A, BRD4164A, BRD4170A, BRD4253A, BRD4254A, BRD4270B, BRD4271A, BRD4272A, BRD4400C, BRD4401A/B/C)
- **Toolchain:** ARM GCC v12.2.1 or later
- **IDE:** Simplicity Studio v5 with Simplicity SDK 2025.6.2 or later

## Project Architecture

### Core Application Modules

- **`app.c/h`** - Main application logic and state machine
- **`main.c`** - Application entry point and initialization
- **`app_init.c`** - Hardware and software component initialization

### Network Communication

- **`app_coap.c/h`** - CoAP server implementation and endpoint handlers
- **`app_tcp_server.c/h`** - TCP server for persistent connections
- **`app_udp_server.c/h`** - UDP server for connectionless communication
- **`app_direct_connect.c/h`** - Direct device-to-device communication

### Industrial Integration

- **`modbusmaster.c/h`** - Modbus Master protocol implementation
- **`app_parameters.c/h`** - Device parameter management and storage

### Network Services

- **`app_check_neighbors.c/h`** - Wi-SUN neighbor discovery and monitoring
- **`app_wisun_multicast_ota.c/h`** - Firmware update distribution system
- **`app_list_configs.c/h`** - Configuration management interface

### Utilities & Support

- **`app_reporter.c/h`** - Event logging and reporting framework
- **`app_timestamp.c/h`** - Time synchronization and timestamping
- **`app_rtt_traces.c/h`** - SEGGER RTT debug output
- **`sl_wisun_crash_handler.c/h`** - Error handling and recovery
- **`sl_wisun_regdb.c`** - Regulatory domain database

## Getting Started

### Build Instructions

1. Open Simplicity Studio 5
2. Import the project: `File -> Import -> MCU Project`
3. Navigate to `TTDF_PROJECT_DEMO_V1/TTDF_PROJECT_DEMO_V1.slcp`
4. Select your target board from the list
5. Build the project: `Project -> Build Project`

### Flashing the Device

1. Connect your EFR32ZG28 board via USB
2. Right-click the project → `Run As -> Silicon Labs ARM Program`
3. Or use Commander CLI:
   ```
   commander flash GNU\ ARM\ v12.2.1\ -\ Debug/TTDF_PROJECT_DEMO_V1.hex
   ```

### Configuration

Key configuration files located in `config/` directory:

- **Wi-SUN Settings:** `autogen/sl_wisun_config.c/h`
- **Network Parameters:** Modify via `app_parameters.c`
- **Pin Configuration:** `config/pin_config.h`
- **CoAP Endpoints:** See `COAP-INFO.md` for endpoint mapping

## Documentation

Comprehensive documentation available in the project directory:

- **`README.md`** - Project overview and quick start (this file)
- **`COAP-INFO.md`** - CoAP endpoint definitions and usage
- **`ENDPOINT_MAPPING.md`** - Device endpoint configuration
- **`NEW_ENDPOINT_SUMMARY.md`** - Latest endpoint additions
- **`PIN_REFERENCE.md`** - Hardware pin assignments
- **`RPL_INSTANCE_ID.md`** - Routing protocol configuration

## Network Setup

### Border Router Configuration

A Wi-SUN Border Router is required to connect the mesh network to external networks. Configure your border router with:

- Compatible regulatory domain
- Matching network name and security credentials
- Appropriate channel plan and PHY configuration

### Device Commissioning

1. Power on the device
2. Device automatically searches for Wi-SUN networks
3. Joins the configured network using pre-shared keys
4. Receives network parameters via bootstrap
5. Begins normal operation once fully connected

## Modbus Integration

The Modbus Master implementation supports:

- **RTU Mode:** Serial communication (configurable UART)
- **Read Operations:** Coils, Discrete Inputs, Holding Registers, Input Registers
- **Write Operations:** Single/Multiple Coils and Registers
- **Flexible Addressing:** Support for 247 slave devices

Configure Modbus parameters in `modbusmaster.c` or via CoAP endpoints.

## Development Notes

### Debugging

- Use SEGGER RTT Viewer for real-time trace output
- Network Analyzer for Wi-SUN packet inspection
- Commander CLI for device interrogation

### Power Optimization

- Configure sleep modes in `config/os_cfg.h`
- Tune Wi-SUN timing parameters for power/latency tradeoff
- Use event-driven architecture to minimize active time

### Security

- Pre-shared keys configured in keychain
- Support for AES-128 encryption
- Device authentication via EUI-64 identifiers

## Troubleshooting

**Device won't join network:**
- Verify Border Router is operational
- Check network credentials match
- Ensure regulatory domain compatibility
- Review `RPL_INSTANCE_ID.md` for configuration

**Modbus communication fails:**
- Verify physical wiring and UART configuration
- Check slave device address and register map
- Confirm baud rate and parity settings match

**OTA update issues:**
- Ensure sufficient flash space
- Verify multicast group membership
- Check firmware image compatibility

## License & Support

This project is experimental quality and provided as a demonstration platform. For production deployments, additional testing and certification may be required.

For technical support, consult Silicon Labs documentation or community forums.

## Author

Himanshu Fanibhare

## Version

Demo V1 - January 2026

