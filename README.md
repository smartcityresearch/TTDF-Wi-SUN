# TTDF Wi-SUN Projects

## Overview

This repository contains two advanced Wi-SUN mesh networking projects built on Silicon Labs' EFR32ZG28 Series 2 wireless SoCs. Both projects demonstrate best practices for industrial IoT, sensor integration, and mesh networking applications.

### Available Projects

#### 1. TTDF_PROJECT_DEMO_V1
An industrial-grade IoT demonstration project with comprehensive networking and Modbus integration.

**Location:** `Software/TTDF_PROJECT_DEMO_V1.1/`

**Key Features:**
- Wi-SUN FAN mesh networking with IEEE 802.15.4g
- CoAP protocol support with multiple endpoints
- TCP/UDP server implementation
- Modbus Master protocol for industrial equipment
- Over-The-Air (OTA) firmware updates via multicast
- Network diagnostics and monitoring
- Real-time RTT tracing

**Use Case:** Industrial monitoring and control, equipment integration, smart energy management

---

#### 2. PHY_COAP_CODE
A focused Wi-SUN sensor hub application with rich CoAP REST API endpoints.

**Location:** `Software/PHY_COAP_CODE/`

**Key Features:**
- 40+ CoAP RESTful endpoints
- Multi-sensor integration (Si7021, VEML6035)
- Modbus RTU energy meter support
- Relay control via CoAP commands
- Network diagnostics and RPL routing info
- Persistent storage (NVM3) for configuration
- Full connection statistics and neighbor tracking

**Use Case:** Environmental monitoring, sensor data collection, remote device control

---

## Hardware Requirements

- **Board:** Silicon Labs EFR32ZG28 development board
  - Compatible models: BRD2705A, BRD4163A, BRD4164A, BRD4170A, BRD4253A, BRD4254A, BRD4270B, BRD4271A, BRD4272A, BRD4400C, BRD4401A/B/C
- **Toolchain:** ARM GCC v12.2.1 or later
- **IDE:** Simplicity Studio v5 with Simplicity SDK 2025.6.2 or later

## Quick Start

### Importing a Project

1. Open **Simplicity Studio 5**
2. Select **File → Import → MCU Project**
3. Browse to the desired project location:
   - TTDF Project: `Software/TTDF_PROJECT_DEMO_V1.1/TTDF_PROJECT_DEMO_V1.slcp`
   - PHY CoAP: `Software/PHY_COAP_CODE/PHY_COAP_CODE.slcp`
4. Select your target board
5. Click **Import** and **Build Project**

### Flashing the Device

Using Simplicity Studio:
- Right-click project → `Run As → Silicon Labs ARM Program`

Using Commander CLI:
```bash
# TTDF Project
commander flash Software/TTDF_PROJECT_DEMO_V1.1/GNU\ ARM\ v12.2.1\ -\ Debug/TTDF_PROJECT_DEMO_V1.hex

# PHY CoAP Code
commander flash Software/PHY_COAP_CODE/GNU\ ARM\ v12.2.1\ -\ Debug/PHY_COAP_CODE.hex
```

## Project Details

For detailed documentation, configuration guides, and API specifications, refer to the README.md files within each project folder:

- [TTDF_PROJECT_DEMO_V1 - Detailed Documentation](Software/TTDF_PROJECT_DEMO_V1.1/README.md)
- [PHY_COAP_CODE - Detailed Documentation](Software/PHY_COAP_CODE/readme.md)

## Key Features Comparison

| Feature | TTDF Project | PHY CoAP |
|---------|--------------|----------|
| CoAP Endpoints | ✓ | ✓✓ (40+) |
| Modbus Support | ✓✓ (Master) | ✓ (RTU) |
| TCP/UDP Servers | ✓ | - |
| OTA Updates | ✓ | - |
| Sensor Integration | ✓ | ✓✓ (Multiple) |
| Network Diagnostics | ✓ | ✓ |
| Multicast Support | ✓ | - |

## Common Configuration

### Wi-SUN Network Setup

Both projects require a Wi-SUN Border Router for mesh network connectivity. Configure your border router with:

- Compatible regulatory domain
- Matching network name and security credentials
- Appropriate channel plan and PHY configuration

### Device Commissioning

1. Power on the device
2. Device automatically searches for Wi-SUN networks
3. Joins the configured network using pre-shared keys
4. Receives network parameters via bootstrap
5. Begins normal operation once fully connected

## Debugging & Analysis

### Tools

- **SEGGER RTT Viewer** - Real-time trace output and debugging
- **Network Analyzer** - Wi-SUN packet inspection and analysis
- **Commander CLI** - Device interrogation and firmware management
- **Simplicity Studio Debugger** - Step-through debugging

### RTT Output

Both projects support high-speed RTT (Real-Time Transfer) for fast debug output without UART overhead.

## Troubleshooting

**Device won't join network:**
- Verify Border Router is operational
- Check network credentials match configuration
- Ensure regulatory domain compatibility

**Build errors after import:**
- Verify SDK version (2025.6.2 or later)
- Check compiler path configuration
- Rebuild the project with clean build

**Sensor not responding:**
- Verify I2C/UART connections (PHY CoAP specific)
- Check peripheral addresses and pull-up resistors
- Review pin configuration in respective projects

## License & Support

These projects are experimental quality and provided as demonstration platforms. For production deployments, additional testing and certification may be required.

For technical support, consult:
- Silicon Labs official documentation
- Community forums and documentation
- Project-specific README files in each directory

## Author

Himanshu Fanibhare

## Version

Multi-Project Release V1.0 - March 2026