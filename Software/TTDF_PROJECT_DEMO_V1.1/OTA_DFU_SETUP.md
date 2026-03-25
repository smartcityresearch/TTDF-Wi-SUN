
# Wi-SUN OTA DFU Complete Setup Guide

> **Reference Document**: Tested and verified working configuration for Wi-SUN Over-The-Air Device Firmware Update

---

## 📋 Table of Contents

1. [Node Side Configuration](#node-side-configuration)
   - [Bootloader Setup](#bootloader-setup)
   - [Firmware Configuration](#firmware-configuration)
   - [Prepare GBL Firmware File](#prepare-gbl-firmware-file)
2. [Border Router Side Configuration](#border-router-side-configuration)
   - [Prerequisites](#prerequisites)
   - [Border Router Setup](#border-router-setup)
   - [OTA Execution Flow](#ota-execution-flow)
   - [Verification & Monitoring](#verification--monitoring)
   - [Troubleshooting](#troubleshooting)

---

# 🔷 SECTION 1: Node Side Configuration

## 🔧 Bootloader Setup

### Step 1: Open Simplicity Studio and Connect Device

1. Open **Simplicity Studio** and you will see the welcome page
2. Connect your **WSTK kit** to your PC. After successful connection, it will show in the **Connected Devices** section
3. Click on the **Start** button

![image](../Image/bb-1.png)

### Step 2: Create Bootloader Project

4. Click on **Example Projects and Demos**
5. Search for: **bootloader-storage-spiflash-single-1024k** or **1024k**
6. Click on the **Create** button, then open the project in Simplicity Studio

![image](../Image/bb00.png)

### Step 3: Configure Bootloader

7. Open the `.slcp` file

![image](../Image/bb01.png)

8. Go to **Software Components** and search for **lzma**

![image](../Image/bb02.png)

9. Install the LZMA component

### Step 4: Build the Bootloader

10. Build the project using the **Hammer** button
11. After successful build, the **Binaries** folder will be generated in the project folder

### Step 5: Flash Bootloader to Device

12. Open the **Binaries** folder and right-click on the `.s37` file

![image](../Image/bb03.png)

13. Click on **Flash to Device**

![image](../Image/bb04.png)

14. In the Flash Programmer window:
    - Click the **Erase** button
    - Then click the **Program** button

15. The bootloader will flash to your connected board. Check the console for any errors

![image](../Image/bb05.png)

---

## ⚙️ Main Project Configuration

Now we will configure the main project for OTA DFU.

### Step 1: Open Main Project

1. Open your main project in **Simplicity Studio**
2. Open the `.slcp` file

![image](../Image/mp01.png)

### Step 2: Install OTA DFU Component

3. Go to **Software Components** and search for **OTA DFU**

4. In **Filter Components** > **Quality**, check:
   - Production Ready
   - Evaluation

5. Navigate to: **Wi-SUN** > **Wi-SUN Service** > **Wi-SUN Over-The-Air Device Firmware Upgrade (OTA DFU)**

![image](../Image/mp02.png)

### Step 3: Configure OTA DFU Component

6. Click **Install**, then click the **Configure** button

![image](../Image/mp03.png)

7. In the configuration window, set the parameters as shown in the image below, then **Save** and close the configuration window

![image](../Image/mp04.png)

### Step 4: Build and Flash Main Project

8. Build your main project using the **Hammer** button
9. After successful build, the **Binaries** folder will be generated in the project folder
10. Open the **Binaries** folder and right-click on the `.s37` file
11. Click **Flash to Device**, then click **Program** in the Flash Programmer window to flash the main project to your connected board

> **📌 Note:** 
> 
> To perform OTA updates, you must:
> - Create a new version of the application (for example, Version 2)
> - Build it
> - Convert it into a `.gbl` file
> - Upload it to the TFTP server
> - Start the OTA process

---

## 📦 Prepare GBL Firmware File


### Step 1: Locate the Generated .s37 File

After building the updated application (e.g., Version 2), a new `.s37` file will be generated inside the **Binaries** folder.

This `.s37` file must be converted into a `.gbl` (Gecko Bootloader) file before it can be used for OTA DFU.

1. Open your project folder
2. Go to the **Binaries** directory
3. Locate the newly generated `.s37` file

**Example:**
```
"C:\Users\iamfa\SimplicityStudio\v5_workspace\TTDF_PROJECT_DEMO_V1.1\GNU ARM v12.2.1 - Debug\TTDF_PROJECT_DEMO_V1.1.s37"
```

### Step 2: Open Simplicity Commander (Windows)

On Windows, Simplicity Commander is typically installed at:
```
C:\SiliconLabs\SimplicityStudio\v5\developer\adapter_packs\commander
```

Open **Command Prompt** and navigate to that directory:

```bash
cd C:\SiliconLabs\SimplicityStudio\v5\developer\adapter_packs\commander
```

for the check run:

```bash
C:\SiliconLabs\SimplicityStudio\v5\developer\adapter_packs\commander>commander.exe
```
> **📌 Note:**  
>  Check commander window open or not if not means you are in wrong path 

### Step 3: Create the .gbl File

Run the following command to create the GBL file with LZMA compression:

```bash
C:\SiliconLabs\SimplicityStudio\v5\developer\adapter_packs\commander>commander gbl create --app "YOUR\PATH\TO\S37\FILE\YOUR_PROJECT.s37" YOUR\PATH\TO\STORE\GBL\FILE\wisun_firmware.gbl --compress lzma
```

**Replace the placeholders:**
- `"YOUR\PATH\TO\S37\FILE\YOUR_PROJECT.s37"` - Actual path to your generated `.s37` file
- `"YOUR\PATH\TO\STORE\GBL\FILE\wisun_firmware.gbl"` - Desired path and filename for your output `.gbl` file

**Example:**
```bash
commander gbl create --app "C:\Users\iamfa\SimplicityStudio\v5_workspace\TTDF_PROJECT_DEMO_V1.1\GNU ARM v12.2.1 - Debug\TTDF_PROJECT_DEMO_V1.1.s37" "C:\Users\iamfa\SimplicityStudio\v5_workspace\TTDF_PROJECT_DEMO_V1.1\GNU ARM v12.2.1 - Debug\wisun_firmware.gbl" --compress lzma
```

> **📌 Important:**  
> The output `.gbl` filename must match the name you specified in the **Wi-SUN Over-The-Air Device Firmware Upgrade (OTA DFU)** component configuration under **"Firmware image (gbl) file name on TFTP server"**.

After successful execution, you will get:
- A `.gbl` file at your specified location
- This file is ready for OTA DFU deployment

### Step 4: Compress the GBL File (Optional but Recommended)

To reduce OTA transfer time, you can compress the firmware.

**Using LZ4 Compression:**
```bash
commander gbl create --app wisun_soc_empty_version_2.s37 wisun_soc_empty_version_2_lz4.gbl --compress lz4
```

**Using LZMA Compression (Better Compression Ratio):**
```bash
commander gbl create --app wisun_soc_empty_version_2.s37 wisun_soc_empty_version_2_lzma.gbl --compress lzma
```

**Compression Comparison:**

| Compression | Size Reduction | Speed |
|-------------|----------------|-------|
| None | 100% | Fast |
| LZ4 | ~85% | Faster |
| LZMA | ~60% | Smaller file (slower compression, faster OTA) |

> **Recommendation:** LZMA gives the smallest file size and is generally recommended for OTA.

---

### Step 5: Transfer GBL File to Border Router

Now that you have your `.gbl` file, you need to copy it to the Raspberry Pi Linux Border Router. You can use either of these tools:

**Option A: Using SCP (Command Line)**

Open **Command Prompt** or **PowerShell** and run:

```bash
scp "C:\Path\To\Your\wisun_firmware.gbl" pi@<BORDER_ROUTER_IP>:/tmp/
```

**Example:**
```bash
scp "C:\Users\iamfa\SimplicityStudio\v5_workspace\TTDF_PROJECT_DEMO_V1.1\GNU ARM v12.2.1 - Debug\wisun_firmware.gbl" pi@192.168.1.100:/tmp/
```

**Option B: Using WinSCP (GUI Tool)**

1. Download and install [WinSCP](https://winscp.net/)
2. Connect to your Border Router using SSH protocol
3. Navigate to the local folder containing your `.gbl` file
4. Drag and drop the `.gbl` file to `/tmp/` directory on the Border Router




### Step 6: Move GBL File to TFTP Directory

Now transfer the `.gbl` file from `/tmp` directory to the `/srv/tftp/` server directory:

On the **Border Router**, run:

```bash
sudo cp /tmp/wisun_firmware.gbl /srv/tftp/
```

**Verify the file is present:**

```bash
ls /srv/tftp/
```

**Expected output:**
```
wisun_firmware.gbl
```

---

# 🔶 SECTION 2: Border Router Side Configuration

## 📐 Architecture Overview

### Component Layout

| Component | IPv6 Address | Port | Purpose |
|-----------|--------------|------|---------|
| **TFTP Server** | `fd12:3456::1` | `69` | Hosts `.gbl` firmware file for download |
| **Notification Server** | `fd12:3456::2` | `5685` | Receives OTA progress updates from node |
| **Node OTA Service** | `[Node IPv6]` | `5683` | Controls OTA start/stop/install operations |

### Data Flow

```
┌─────────────┐     ┌──────────────────┐     ┌─────────────────────┐
│   Node      │────▶│  TFTP Server     │     │  Notification       │
│  (Device)   │     │  fd12:3456::1:69 │     │  Server             │
│             │     └──────────────────┘     │  fd12:3456::2:5685  │
│             │                               │                     │
│             │──────────────────────────────▶│  Receives progress  │
└─────────────┘   POST /ota/dfu_notify       └─────────────────────┘
     │
     │ Download .gbl
     │ Send progress updates
     │ Verify & Install
     ▼
  Reboot with
  new firmware
```

---

## ✅ Prerequisites

### Hardware Requirements

- ✔ Wi-SUN Border Router running (`wsbrd` operational)
- ✔ Wi-SUN Node connected to Border Router
- ✔ Bootloader flashed on node (OTA-capable)
- ✔ Serial console access to node

### Software Requirements

**On Border Router:**

- ✔ `tftpd-hpa` (TFTP server)
- ✔ `libcoap3-bin` (CoAP client/server utilities)
- ✔ `ifconfig` / `ip` tools
- ✔ Router configured with `tun0` interface

**On Development PC:**

- ✔ Simplicity Studio v5
- ✔ Simplicity Commander
- ✔ Application firmware with OTA DFU component enabled

---

## 🔧 Border Router Setup

### Step 1: Install Required Packages

```bash
sudo apt-get update
sudo apt-get install tftpd-hpa tftp-hpa
sudo apt-get install libcoap3 libcoap3-bin
```

### Step 2: Configure TFTP Server

Edit TFTP configuration:

```bash
sudo nano /etc/default/tftpd-hpa
```

Set the following:

```ini
TFTP_USERNAME="tftp"
TFTP_DIRECTORY="/srv/tftp"
TFTP_ADDRESS=":69"
TFTP_OPTIONS="--secure"
```

Restart TFTP service:

```bash
sudo /etc/init.d/tftpd-hpa restart
```

**Verify TFTP is running:**

```bash
sudo ss -u -l -n | grep 69
```

Expected output:

```
UNCONN 0      0            0.0.0.0:69         0.0.0.0:*          
UNCONN 0      0                  *:69               *:* 
```

### Step 3: Add IPv6 Addresses to Border Router

Add both TFTP and notification server addresses to `tun0`:

```bash
sudo ip -6 address add fd12:3456::1/64 dev tun0
sudo ip -6 address add fd12:3456::2/64 dev tun0
```

**Verify addresses:**

```bash
ip address show tun0 | grep global
```

Expected output:

```
inet6 fd12:3456::2/64 scope global 
inet6 fd12:3456::1/64 scope global 
inet6 fd12:3456::eae:5fff:fe52:66da/64 scope global
```

### Step 4: Start Notification Server

**Open Terminal 1** (keep this running):

```bash
coap-server-notls -A fd12:3456::2 -p 5685 -d 10 -v 7
```

**⚠️ Important:** Do NOT close this terminal. Leave it running.

**Verify notification server is listening:**

**Open another terminal(Terminal 2) and run:**

```bash
sudo ss -u -l -n | grep 5685
```

Expected output:

```
UNCONN 0      0      [fd12:3456::2]:5685             *:*  
```

---

## 🚀 OTA Execution Flow

### Pre-Flight Checklist

Before starting OTA, verify:

```bash
# ✔ TFTP running
sudo ss -u -l -n | grep 69

# ✔ Notification server running (Terminal 1)
sudo ss -u -l -n | grep 5685

# ✔ GBL file present
ls /srv/tftp/wisun_firmware.gbl

# ✔ Both IPv6 addresses assigned
ip address show tun0 | grep "fd12:3456"

# ✔ Node connected (check serial console or ping)
```

### Set Node Address Variable

Find your node's global IPv6 address:

```bash
# From serial console output, or from Border Router:
wsbrd_cli status
```

Set environment variable:

```bash
WISUN_NODE_IPV6_ADDR="YOUR_NODE_IPV6_ADDRESS"
```

### Step 1: Verify Node Connectivity

**Ping the node:**

```bash
ping $WISUN_NODE_IPV6_ADDR -c 2
```

Expected: Replies received.

**Check OTA resource is available:**

```bash
coap-client-notls -m get coap://[$WISUN_NODE_IPV6_ADDR]:5683/ota/dfu
```

Expected response (JSON):

```json
{
"ip": "fd12:3456::eae:5fff:fe52:6695",
"elapsed_downl_t": "0-00:00:00",
"elapsed_upd_t": "0-00:00:00",
"elapsed_since_rst_t": "0-01:48:40",
"downl_bytes": 0,
"flags": "0x00000000",
"fw_update_started": 0,
"fw_downloaded": 0,
"fw_verified": 0,
"fw_set": 0,
"fw_stopped": 0,
"fw_download_error": 0,
"fw_verify_error": 0,
"fw_set_error": 0,
"resent/received": "0/0"
}

```

### Step 2: Start OTA Download

**Terminal 2:**

```bash
coap-client-notls -m post -t text coap://[$WISUN_NODE_IPV6_ADDR]:5683/ota/dfu -e "start"
```

**Expected response:**

```
[Ack: start]
```

### Step 3: Monitor Progress

**Terminal 1** (notification server) should now show:

```
Feb 11 22:42:13.131 DEBG created UDP  endpoint [fd12:3456::2]:5685
Feb 11 22:42:13.132 DEBG created TCP  endpoint [fd12:3456::2]:5685

```

**Terminal 3** (open a new terminal):

```bash
watch -n 2 coap-client-notls -m get coap://[fd12:3456::2]:5685/ota/dfu_notify
```

**Expected output (updating):**

```json
{
  "ip": "fd12:3456::eae:5fff:fe52:6695",
  "elapsed_downl_t": "0-00:01:23",
  "elapsed_upd_t": "0-00:01:30",
  "elapsed_since_rst_t": "0-01:50:03",
  "downl_bytes": 123456,      ← Increasing value
  "flags": "0x00000000",
  "fw_update_started": 1,
  "fw_downloaded": 0,
  "fw_verified": 0,
  "fw_set": 0,
  "fw_stopped": 0,
  "fw_download_error": 0,
  "fw_verify_error": 0,
  "fw_set_error": 0,
  "resent/received": "10/10"
}
```

**Wait until download completes:**

```json
{
  "ip": "fd12:3456::eae:5fff:fe52:6695",
  "elapsed_downl_t": "0-00:03:45",
  "elapsed_upd_t": "0-00:03:50",
  "elapsed_since_rst_t": "0-01:52:25",
  "downl_bytes": 350000,      ← Should match fw_size
  "flags": "0x00000000",
  "fw_update_started": 1,
  "fw_downloaded": 1,         ← Download complete
  "fw_verified": 1,           ← Verification complete
  "fw_set": 1,                ← Firmware set for install
  "fw_stopped": 0,
  "fw_download_error": 0,
  "fw_verify_error": 0,
  "fw_set_error": 0,
  "resent/received": "20/20"
}
```

### Step 4: Install Firmware

**If manual install is configured:**

```bash
coap-client-notls -m post -t text coap://[$WISUN_NODE_IPV6_ADDR]:5683/ota/dfu -e "install"
```


**Node will reboot automatically.**

### Step 5: Verify New Firmware

**Check serial console output after reboot:**

Expected:

```
--------------------------------------------------
 Wi-SUN Node Application
 Device:  efr32zg28b322f1024im68
 Board:   brd4401c
 Version: 2.0.0  ← Should show new version
 Compiled: Feb 11 2025 10:45:23
--------------------------------------------------
```

---

## 🔍 Verification & Monitoring

### During Download

| What to Watch | Where | Expected Behavior |
|---------------|-------|-------------------|
| **POST requests** | Terminal 1 (notification server) | Regular POST lines every ~10 chunks |
| **downl_bytes** | Terminal 3 (watch command) | Steadily increasing value |
| **Serial console** | Node serial output | OTA progress messages |

### Download Completion Indicators

✅ `"fw_downloaded": 1`
✅ `"fw_verified": 1`
✅ `"fw_set": 1`
✅ `"downl_bytes"` equals `"fw_size"`

### Post-Install Verification

```bash
# After reboot, check OTA status again
coap-client-notls -m get coap://[$WISUN_NODE_IPV6_ADDR]:5683/ota/dfu
```

All fields should reset to 0:

```json
{
  "fw_update_started": 0,
  "downl_bytes": 0,
  "fw_size": 0,
  "fw_downloaded": 0,
  "fw_verified": 0,
  "fw_set": 0
}
```

---

## 🔧 Troubleshooting

### Issue 1: Connection Refused on Notification Server

**Symptoms:**

- `coap-client: Connection refused` when accessing `/ota/dfu_notify`
- No POST lines in notification server terminal

**Causes & Fixes:**

| Cause | Fix |
|-------|-----|
| Notification server not running | Start `coap-server-notls` in Terminal 1 |
| Wrong port in firmware config | Verify `SL_WISUN_OTA_DFU_NOTIFY_PORT = 5685U` |
| IPv6 address not assigned | Run `sudo ip -6 address add fd12:3456::2/64 dev tun0` |
| Server listening on wrong address | Check with `ss -u -l -n \| grep 5685` |

### Issue 2: Download Not Starting (downl_bytes = 0)

**Symptoms:**

- `fw_update_started = 1` but `downl_bytes` stays at 0
- No TFTP activity

**Causes & Fixes:**

| Cause | Fix |
|-------|-----|
| TFTP server not running | `sudo /etc/init.d/tftpd-hpa restart` |
| Wrong TFTP address in firmware | Verify `SL_WISUN_OTA_DFU_HOST_ADDR = "fd12:3456::1"` |
| GBL file missing | Check `/srv/tftp/wisun_firmware.gbl` exists |
| Wrong filename | Verify `SL_WISUN_OTA_DFU_GBL_FILE = "wisun_firmware.gbl"` |
| TFTP port blocked | Check `ss -u -l -n \| grep 69` |

### Issue 3: No POST Requests to Notification Server

**Symptoms:**

- Terminal 1 shows no POST lines
- `watch` command shows error or timeout

**Causes & Fixes:**

| Cause | Fix |
|-------|-----|
| Notification disabled in firmware | Set `SL_WISUN_OTA_DFU_HOST_NOTIFY_ENABLED = 1U` |
| Wrong notification address | Verify `SL_WISUN_OTA_DFU_NOTIFY_HOST_ADDR = "fd12:3456::2"` |
| Port mismatch | Ensure firmware port matches server port (5685) |
| Routing issue | Check node can reach ::2 with `ping` |

### Issue 4: Firmware Verification Failed

**Symptoms:**

- `fw_downloaded = 1` but `fw_verified = 0`
- Serial console shows verification error

**Causes & Fixes:**

| Cause | Fix |
|-------|-----|
| Corrupted GBL file | Recreate GBL with Simplicity Commander |
| Incomplete download | Check `downl_bytes = fw_size` |
| Wrong bootloader version | Update bootloader on device |
| GBL encryption mismatch | Ensure GBL signing keys match bootloader |

### Issue 5: Install Timeout

**Symptoms:**

- `install` command timeout
- Node doesn't reboot

**Causes & Fixes:**

| Cause | Fix |
|-------|-----|
| Firmware not fully downloaded | Wait until `fw_set = 1` |
| Auto-install enabled but broken | Set `SL_WISUN_OTA_DFU_AUTO_INSTALL_ENABLED = 0U` |
| Bootloader issue | Reflash bootloader |
| Node lost connection | Verify node still reachable with ping |

### Debug Commands

**Check all ports:**

```bash
sudo ss -u -l -n | grep -E "69|5683|5685"
```

**Check IPv6 routing:**

```bash
ip -6 route show dev tun0
```

**Test TFTP manually:**

```bash
tftp fd12:3456::1 69 -c get wisun_firmware.gbl
```

**Check notification server manually:**

```bash
coap-client-notls -m post -t text \
  coap://[fd12:3456::2]:5685/ota/dfu_notify \
  -e "test"
```

---

## 📚 Command Reference

### Quick Command Sheet

```bash
# ═══════════════════════════════════════════════════════
#  SETUP COMMANDS (Run Once)
# ═══════════════════════════════════════════════════════

# Install packages
sudo apt-get install tftpd-hpa tftp-hpa libcoap3-bin

# Configure TFTP
sudo nano /etc/default/tftpd-hpa
sudo mkdir -p /srv/tftp
sudo /etc/init.d/tftpd-hpa restart

# Add IPv6 addresses
sudo ip -6 address add fd12:3456::1/64 dev tun0
sudo ip -6 address add fd12:3456::2/64 dev tun0

# Copy firmware
sudo cp wisun_firmware.gbl /srv/tftp/

# ═══════════════════════════════════════════════════════
#  VERIFICATION COMMANDS
# ═══════════════════════════════════════════════════════

# Check TFTP
sudo ss -u -l -n | grep 69
ls /srv/tftp/

# Check notification server
sudo ss -u -l -n | grep 5685

# Check addresses
ip address show tun0 | grep "fd12:3456"

# ═══════════════════════════════════════════════════════
#  RUNTIME COMMANDS
# ═══════════════════════════════════════════════════════

# Terminal 1: Start notification server (keep running)
coap-server-notls -A fd12:3456::2 -p 5685 -d 10 -v 7

# Terminal 2: Set node address and run OTA
export WISUN_NODE_IPV6_ADDR="fd12:3456::xxxx:xxxx:xxxx:xxxx"

# Verify node
ping $WISUN_NODE_IPV6_ADDR -c 2
coap-client-notls -m get coap://[$WISUN_NODE_IPV6_ADDR]:5683/ota/dfu

# Start OTA
coap-client-notls -m post -t text \
  coap://[$WISUN_NODE_IPV6_ADDR]:5683/ota/dfu -e "start"

# Install firmware (after download completes)
coap-client-notls -m post -t text \
  coap://[$WISUN_NODE_IPV6_ADDR]:5683/ota/dfu -e "install"

# Terminal 3: Monitor progress
watch -n 2 coap-client-notls -m get \
  coap://[fd12:3456::2]:5685/ota/dfu_notify
```

---

## 🎯 Success Criteria

Your OTA is successful when:

- [x] TFTP server responds on port 69
- [x] Notification server shows POST requests
- [x] `downl_bytes` reaches `fw_size`
- [x] `fw_verified = 1`
- [x] `fw_set = 1`
- [x] Install command succeeds
- [x] Node reboots
- [x] Serial console shows new version

---

## 🔄 Typical Timeline

| Phase | Duration | What to Expect |
|-------|----------|----------------|
| **Start command** | 1-2 seconds | Ack received, node begins TFTP |
| **Download** | 2-5 minutes | `downl_bytes` increases, POST requests every 10 chunks |
| **Verification** | 5-10 seconds | `fw_verified` changes to 1 |
| **Set** | 1 second | `fw_set` changes to 1 |
| **Install** | 1-2 seconds | Ack received |
| **Reboot** | 10-15 seconds | Node restarts with new firmware |

**Total time:** ~3-7 minutes for typical 350KB GBL file

---

## 📝 Notes

### Port Summary

Never mix these ports:

- **69** = TFTP (file download)
- **5683** = Node CoAP control
- **5685** = Notification server

### Address Summary

- **fd12:3456::1** = TFTP server (hosts file)
- **fd12:3456::2** = Notification server (receives updates)
- **[Node IPv6]** = Wi-SUN device (downloads firmware)

### Important Behaviors

- **UDP session closed** after download = Normal, not an error
- **resent/received: 0/0** after completion = Normal, OTA idle
- **EVENT: 0x4002** = UDP cleanup, expected behavior
- Node must be connected to Border Router before OTA
- Notification server must run throughout entire OTA process

---

## 🚀 Advanced Topics

### Enable Auto-Install

In `sl_wisun_ota_dfu_config.h`:

```c
#define SL_WISUN_OTA_DFU_AUTO_INSTALL_ENABLED  1U
```

Node will reboot automatically after download completes.

### Increase TFTP Timeout

For unreliable networks:

```c
#define SL_WISUN_OTA_DFU_TFTP_TIMEOUT_SEC  8U
```

### Use LZMA Compression

Create GBL with:

```bash
commander gbl create --app app.s37 wisun_firmware.gbl --compress lzma
```

Smaller file size but higher decompression overhead.

### Multiple Nodes

Script OTA for multiple nodes:

```bash
for node in $NODE_LIST; do
  coap-client-notls -m post -t text \
    coap://[$node]:5683/ota/dfu -e "start"
  sleep 10
done
```

### Multicast OTA

See `app_wisun_multicast_ota.c` for multicast OTA implementation (different approach).

---

## 📄 References

- [Wi-SUN OTA DFU Application Note](https://www.silabs.com/documents/public/application-notes/)
- [Simplicity Commander User Guide](https://www.silabs.com/documents/public/user-guides/ug162-simplicity-commander-reference-guide.pdf)
- [libcoap Documentation](https://libcoap.net/)
- [RFC 7252 - CoAP Protocol](https://tools.ietf.org/html/rfc7252)

---

**Document Version:** 1.0  
**Last Updated:** February 11, 2025  
**Tested On:** Simplicity SDK 2025.6.2, EFR32ZG28

---

**© Your Organization | Wi-SUN OTA Reference**
