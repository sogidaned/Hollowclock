# Hollowclock Standalone

A standalone version of the Hollowclock that works without Home Assistant.

## Installation

### Direct Installation (recommended)
The easiest way to install the firmware:

1. Visit [install.hollowclock.local](https://install.hollowclock.local)
2. Connect your Wemos D1 Mini to the USB port
3. Click "Install" and follow the instructions
4. After installation, the clock is ready to use

*For advanced users: Alternative installation methods are available using PlatformIO.*

## First Setup

1. After the first start, the clock creates a WiFi access point named "Hollowclock-XXXXXX"
2. Connect to this network (password: 12345678)
3. A configuration portal will open automatically (if not, open http://192.168.4.1)
4. Select your WiFi network and enter the password
5. The clock will restart and connect to your WiFi

## Hardware Connections

Connect the ULN2003 stepper motor driver as follows:
- IN1 -> D8
- IN2 -> D7
- IN3 -> D6
- IN4 -> D5

## Usage

1. After startup, the clock automatically connects to your WiFi
2. Open http://hollowclock.local in a web browser (or use the IP address shown in the serial monitor)
3. Through the web interface, you can:
   - Set the time manually
   - Make fine adjustments (+30 seconds)
   - Reset WiFi settings
   - Manage MQTT configuration (optional)
   - Perform firmware updates

## MQTT Integration (Optional)

The clock supports optional MQTT integration with Home Assistant:

1. Open the clock's web interface
2. Go to MQTT configuration
3. Enable MQTT using the switch
4. Enter the MQTT server data:
   - Broker address (e.g., homeassistant.local)
   - Port (default: 1883)
   - Client ID (unique name)
   - Topic (unique topic prefix)
   - Optional: Username and password
5. Click "Save MQTT Settings"

### Home Assistant Integration

The clock sends the following information to Home Assistant:
- Current time in HH:MM format
- Current position of the stepper motor

You can control the clock via Home Assistant with the following MQTT command:
```yaml
service: mqtt.publish
data:
  topic: hollowclock/set
  payload: |
    {"set_time": {"hour": 3, "minute": 30}}
```

#### Setting up Home Assistant Automation

For better integration, you can use an `input_datetime` helper in Home Assistant to control the clock. Create the following:

1. Add an `input_datetime` helper named `hollowclock` in Home Assistant (Configuration → Helpers)
2. Create an automation that updates the clock whenever the input_datetime value changes:

```yaml
alias: Hollowclock set
description: ""
triggers:
  - trigger: state
    entity_id:
      - input_datetime.hollowclock
    for:
      hours: 0
      minutes: 0
      seconds: 5
    from: null
    to: null
conditions: []
actions:
  - action: mqtt.publish
    metadata: {}
    data:
      evaluate_payload: false
      qos: 0
      retain: false
      topic: Hollowclock/set
      payload: >-
        {"set_time": {"hour": {{
        states('input_datetime.hollowclock').split(':')[0] | int }}, "minute":
        {{ states('input_datetime.hollowclock').split(':')[1] | int }}}}
mode: single
```

This automation will send the time from the input_datetime helper to the clock 5 seconds after any change.

## Firmware Update

You can update the firmware via the web interface:

1. Open the clock's web interface
2. Click on "Firmware Update"
3. Select the new .bin file
4. Click "Update"
5. Wait for the update to complete

### Important: After Major Updates

**After major updates, you should also update the web interface files (file system):**

1. Download the latest `littlefs.bin` file from the [GitHub Releases page](https://github.com/sogidaned/hollowclock/releases)
2. On the Firmware Update page, select "File System" instead of "Firmware"
3. Select the downloaded `littlefs.bin` file
4. Click "Update" and wait for completion
5. After the file system update, the clock will restart

### Auto-Update Feature (coming soon)

In a future update, a new button will be added to the web interface that allows you to check for and download the latest firmware and file system directly from GitHub, making the update process even simpler.

## Reset WiFi Settings

If you want to change the WiFi settings:
1. Open the clock's web interface
2. Click "Reset WiFi Settings"
3. The clock will restart and create an access point again
4. Follow the steps under "First Setup"

## Troubleshooting

If the clock is not working properly:

1. Make sure the hardware connections are correct
2. Check that the power supply is sufficient (5V/1A recommended)
3. If the time is wrong, check your internet connection and NTP settings
4. For WiFi problems, reset the WiFi settings as described above
5. If the web interface looks broken after a firmware update, update the file system as described in the "Firmware Update" section

## Source Code

The complete source code for this project is available on GitHub: [sogidaned/hollowclock](https://github.com/sogidaned/hollowclock)


