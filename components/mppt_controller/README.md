# MPPT Controller ESPHome Component

This is a complete **ESPHome external component** for communicating with an RS485-based MPPT solar charge controller using the custom protocol described in your manual.

---

## 📁 File Structure

```
/mppt_controller/
├── __init__.py
├── mppt_controller.h
├── mppt_controller.cpp
├── sensor.py
├── services.py
├── switches.py
└── README.md  ← (this file)
```
# Assumes all files are placed in /config/esphome/mppt_controller and yaml in /config/esphome/
Add the folder under your ESPHome configuration directory (e.g., `/config/esphome/components/`).

---

## ⚙️ ESPHome YAML Example

```yaml
# Example YAML configuration
substitutions:
  name: mpptcontrol
  device_description: "Monitor and control a MPPT Charge Controller via UART"
  rx_pin: GPIO17
  tx_pin: GPIO16

esphome:
  name: ${name}
  comment: ${device_description}


esp32:
  board: esp32dev
  framework:
    type: esp-idf

# web_server:
#   port: 80

logger:

# Required sensor platform
sensor:

# Required switch platform (for load_switch)
switch:

# Required text_sensor for any text-based sensors
text_sensor:

# Required binary_sensor platform
binary_sensor:

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
#  use_address: 192.168.0.194
  
  ap:
    ssid: "MPPT Fallback Hotspot"
    password: "fallback_password"

api:
ota:
  - platform: esphome
    password: !secret ota_password

external_components:
  - source:
      type: local
      path: .
    components: mppt_controller

## if all files are placed in /config/esphome/mppt_controller and yaml in /config/esphome/
##otherwise comment above and use below.
#external_components:
#  - source:
#      type: git
#      url: https://github.com/Mr-EJ/art-mppt-controller.git
#      ref: main

uart:
  - id: mppt_bus
    tx_pin: ${tx_pin}
    rx_pin: ${rx_pin}
    baud_rate: 9600  

mppt_controller:
  id: mppt1
  update_interval: 8s

  pv_voltage:
    name: "PV Voltage"
    accuracy_decimals: 3
  pv_current:
    name: "PV Current"
    accuracy_decimals: 3
  pv_power:
    name: "PV Power"
    accuracy_decimals: 3
  battery_voltage:
    name: "Battery Voltage"
    accuracy_decimals: 3
  temperature:
    name: "MPPT Temperature"
    accuracy_decimals: 3
  load_current:
    name: "Load Current"
    accuracy_decimals: 3
  daily_energy:
    name: "Daily Energy"
    accuracy_decimals: 3
  total_energy:
    name: "Total Energy"
    accuracy_decimals: 3
  error_code:
    name: "MPPT Error Code"
  working_mode:
    name: "MPPT Working Mode"
    
  load_switch:
    name: "MPPT Load Output"
    icon: "mdi:power"
    entity_category: config
    mppt_controller: mppt1 

script:
  - id: reset_mppt_energy
    then:
      - mppt_controller.reset_energy:  # This now calls the safe method automatically
          id: mppt1
          clear: true
          reboot: false

  - id: set_mppt_params
    then:
      - mppt_controller.set_charging_params:  # This now calls the safe method automatically
          id: mppt1
          charge_current: 16
          battery_type: 4
          const_voltage: 288
          load_undervoltage: 224
```

---

## 🪛 Wiring Guide
==Option 1
| Pin | Function | Connect To |
|-----|-----------|-------------|
| TX  | RS485 TX  | RS485 Adapter RX |
| RX  | RS485 RX  | RS485 Adapter TX |
| GND | Ground    | Common ground |
| DE/RE | Always High (Transmit Enable) | Tie to 3.3V |

Recommended RS485 transceiver: **MAX485 or SN65HVD230**.

==Option 2
| ESP32-WROOM |Direction | TTL-to-RS485 auto flowctl | 
|-----|-----------|-------------|
| GND     | ----------> |GND|
| GPIO17  | ----------> |DI (Data In - TX from ESP32)|
| GPIO16  | ----------> |RO (Data Out - RX to ESP32)|
| RS485 device| |TTL-Converter|
|A         |----------> |A (or D+)|
|B         |----------> |B (or D-)|


---

## 🧠 Component Overview

This component:
- Sends **status query** (`Data Type 0x00`) every `update_interval` seconds.
- Receives `AA BB` binary response from MPPT.
- Parses and publishes:
  - PV Voltage
  - Battery Voltage
  - (Extendable for PV Current, Load Current, Temperature, Energy, etc.)

---

## 🧩 Home Assistant Integration

Once compiled and flashed, the sensors appear automatically via the **ESPHome API** integration.

1. In Home Assistant → *Settings → Devices & Services → Add Integration → ESPHome*.
2. Enter the ESP IP or hostname.
3. Your MPPT sensors (e.g., PV Voltage, Battery Voltage) will show under the ESPHome device.

---

## 🔧 Service Calls

You can extend the component to include:
- **`mppt.reset`** → clear energy counters safely (as per your Python reset logic)
- **`mppt.reboot`** → trigger device reboot
- **`mppt.set_parameter`** → write new charging current or voltage parameters

These would be added in the C++ file with `register_service()` in the `setup()` function, like:

```cpp
register_service(&MPPTController::safe_reset, "mppt_reset");
register_service(&MPPTController::set_parameter, "mppt_set_param", {"param", "value"});
```

Then callable from Home Assistant as:

```yaml
service: esphome.mppt_reset
```

---

## ✅ Next Steps


- Add multi device support.

---

**Maintainer:** Mr-EJ  
**Version:** 1.0.0  
**Protocol:** (RS485 9600 8N1)

---


