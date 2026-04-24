# BRD4401C (EFR32xG28) – GPIO / UART / I2C / SPI Pin Reference

This README documents the **actual pin usage** in the TTDF_PROJECT_DEMO_V1.1 project.

---

## Board
- **Radio board**: BRD4401C
- **MCU**: EFR32ZG28 (xG28 family)
- **Base board**: Wireless Starter Kit (BRD4001)

---

## 📍 PINS ACTIVELY USED IN THIS PROJECT

### GPIO / LED Outputs

| Function | EFR32 Pin | Config File | Notes |
|----------|-----------|-------------|-------|
| Relay Control | PA11 | `sl_simple_led_relay_config.h` | Active HIGH |
| Network Status LED | PA12 | `sl_simple_led_network_status_config.h` | Active HIGH |
| Relay Status LED | PA13 | `sl_simple_led_relay_status_config.h` | Active HIGH |

---

### 🔌 UART – VCOM (EUSART0) – USB Debug Console

| Signal | EFR32 Pin | Peripheral | Notes |
|--------|-----------|------------|-------|
| TX | PA08 | EUSART0 | VCOM output to PC |
| RX | PA09 | EUSART0 | VCOM input from PC |
| CTS | PA10 | EUSART0 | Flow control (optional) |
| RTS | PA00 | EUSART0 | Flow control (optional) |

**Baud**: 115200  
**Config**: `sl_iostream_eusart_vcom_config.h`

---

### 🔌 UART – EXP Header (EUSART2) – Modbus RTU / External Device

| Signal | EFR32 Pin | EXP Header | Peripheral | Notes |
|--------|-----------|------------|------------|-------|
| TX | PD11 | EXP 12 | EUSART2 | Modbus RTU TX |
| RX | PD12 | EXP 14 | EUSART2 | Modbus RTU RX |

**Baud**: 9600   
**Config**: `sl_iostream_eusart_exp_config.h`

---

### 🔌 RS485 Direction Control (Modbus RTU)

| Signal | EFR32 Pin | Notes |
|--------|-----------|-------|
| DE/RE | PC02 | RS485 transceiver direction control (HIGH=TX, LOW=RX) |

**Config**: Hardcoded in `modbusmaster.c`

⚠️ **Note**: PC02 is not used in this project because we used the PZE<-004t current sensor which needs only TX and RX pins for Modbus communication, so the DE/RE control is not required. 


---

### 🔁 I2C – Sensor Bus (I2C1)

| Signal | EFR32 Pin | EXP Header | Peripheral |
|--------|-----------|------------|------------|
| SCL | PC05 | EXP 15 | I2C1 |
| SDA | PC07 | EXP 16 | I2C1 |

**Speed**: Standard mode (100 kbit/s)  
**Config**: `sl_i2cspm_sensor_config.h`

---

### 💾 SPI – MX25 Serial Flash (EUSART1)

| Signal | EFR32 Pin | Peripheral | Notes |
|--------|-----------|------------|-------|
| MOSI/TX | PC01 | EUSART1 | Flash data out |
| MISO/RX | PC02 | EUSART1 | Flash data in |
| SCLK | PC03 | EUSART1 | Flash clock |
| CS | PC04 | GPIO | Flash chip select |

**Config**: `sl_mx25_flash_shutdown_eusart_config.h`

⚠️ **Conflict**: PC02 is shared with RS485 DE/RE control!

---

### 📡 RF Path Switch

| Signal | EFR32 Pin | Notes |
|--------|-----------|-------|
| Radio Active | PD02 | RF path switch radio active indicator |
| Control | PC00 | RF path switch control |

**Config**: `sl_rail_util_rf_path_switch_config.h`

---

### 📊 PTI (Packet Trace Interface)

| Signal | EFR32 Pin | Notes |
|--------|-----------|-------|
| DOUT | PD04 | PTI data output |
| DFRAME | PD05 | PTI frame signal |

**Config**: `sl_rail_util_pti_config.h`

---

### 🔧 Debug (SWD/SWV)

| Signal | EFR32 Pin | Notes |
|--------|-----------|-------|
| SWV | PA03 | Serial Wire Viewer trace output |

**Config**: `sl_debug_swo_config.h`

---

## ❌ DO NOT USE THESE PINS

| Pin | Reason |
|-----|--------|
| PA00 | VCOM RTS |
| PA03 | SWV debug trace |
| PA08, PA09, PA10 | VCOM UART (EUSART0) |
| PA11, PA12, PA13 | **In use**: Relay + LEDs |
| PC00 | RF Path Switch Control |
| PC01–PC04 | MX25 Flash SPI |
| PC02 | RS485 DE/RE control |
| PC05, PC07 | I2C1 sensor bus |
| PD02 | RF Path Switch Radio Active |
| PD04, PD05 | PTI (packet trace) |
| PD11, PD12 | EXP UART / Modbus RTU |

---

## ✅ AVAILABLE GPIO PINS

These pins are **not used** in this project and are safe for additional peripherals:

| EFR32 Pin | EXP Header | Notes |
|-----------|------------|-------|
| PA14 | EXP 9 | Free GPIO |
| PB4 | EXP 11 | Free GPIO |
| PB5 | EXP 13 | Free GPIO |

---



## 📋 Pin Summary Table

| Port | Pin | Function | Peripheral |
|------|-----|----------|------------|
| PA | 00 | VCOM RTS | EUSART0 |
| PA | 03 | SWV Debug | GPIO |
| PA | 08 | VCOM TX | EUSART0 |
| PA | 09 | VCOM RX | EUSART0 |
| PA | 10 | VCOM CTS | EUSART0 |
| PA | 11 | Relay Control | GPIO (LED driver) |
| PA | 12 | Network Status LED | GPIO (LED driver) |
| PA | 13 | Relay Status LED | GPIO (LED driver) |
| PC | 00 | RF Path Switch Ctrl | GPIO |
| PC | 01 | Flash MOSI | EUSART1 |
| PC | 02 | Flash MISO / RS485 DE | EUSART1 / GPIO |
| PC | 03 | Flash SCLK | EUSART1 |
| PC | 04 | Flash CS | GPIO |
| PC | 05 | I2C SCL | I2C1 |
| PC | 07 | I2C SDA | I2C1 |
| PD | 02 | RF Path Switch Active | GPIO |
| PD | 04 | PTI DOUT | PTI |
| PD | 05 | PTI DFRAME | PTI |
| PD | 11 | EXP UART TX / Modbus | EUSART2 |
| PD | 12 | EXP UART RX / Modbus | EUSART2 |

---

## 💡 LED Electrical Guidelines

- GPIO max current ≈ **10 mA** (don't exceed)
- Use **330Ω – 1kΩ** resistor per LED
- All LEDs in this project use **Active HIGH** polarity

---

## Bottom Line

**This project uses:**
- 3 GPIO outputs (PA11, PA12, PA13) for relay/LED control
- 2 UARTs: VCOM (EUSART0) + Modbus RTU (EUSART2)
- 1 I2C bus (I2C1) for sensors
- 1 SPI bus (EUSART1) for MX25 flash
- PTI for RF debugging

**Free pins for expansion**: PA14, PB4, PB5

