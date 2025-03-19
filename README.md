# Hollowclock Standalone

A standalone version of the Hollowclock that works without Home Assistant.

## Installation

### Direct Installation (recommended)
The easiest way to install the firmware:

1. Visit [https://hollowclock.netlify.app](https://hollowclock.netlify.app) 
2. Connect your Wemos D1 Mini to the USB port
3. Click "Install" and follow the instructions
4. After installation, the clock is ready to use

*Note: This installer uses WebUSB and requires Chrome, Edge, or Opera browser. Firefox is not supported.*

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

![Wiring Diagram for Hollowclock](https://raw.githubusercontent.com/sogidaned/Hollowclock/main/docs/wiring_diagram.png)
*Wiring diagram: Wemos D1 Mini with ULN2003 driver and 28BYJ-48 stepper motor*

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


