# MPPT Controller ESPHome Component

This is a complete **ESPHome external component** for communicating with an RS485-based MPPT solar charge controller using the custom protocol described in your manual.

---

## 📁 File Structure

```
components/mppt_controller/
├── __init__.py
├── mppt_controller.h
├── mppt_controller.cpp
├── sensor.py
└── README.md  ← (this file)
```

Add the folder under your ESPHome configuration directory (e.g., `/config/esphome/components/`).

---

## ⚙️ ESPHome YAML Example

```yaml
esphome:
  name: mppt_monitor
  platform: ESP32
  board: esp32dev

external_components:
  - source: ./components/mppt_controller

uart:
  id: mppt_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600

mppt_controller:
  id: mppt1
  update_interval: 5s

sensor:
  - platform: template
    name: "MPPT PV Voltage"
    id: mppt_pv_voltage
  - platform: template
    name: "Battery Voltage"
    id: mppt_battery_voltage
```

---

## 🪛 Wiring Guide

| Pin | Function | Connect To |
|-----|-----------|-------------|
| TX  | RS485 TX  | RS485 Adapter RX |
| RX  | RS485 RX  | RS485 Adapter TX |
| GND | Ground    | Common ground |
| DE/RE | Always High (Transmit Enable) | Tie to 3.3V |

Recommended RS485 transceiver: **MAX485 or SN65HVD230**.

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

- Extend parsing in `parse_response()` to publish all metrics (PV current, load, temperature, etc.)
- Implement parameter write and reset functions following your verified Python examples.
- Add error code translation for better diagnostics.

---

**Maintainer:** Edward Allen  
**Version:** 1.0.0  
**Protocol:** MPPT SI20210402T Series (RS485 9600 8N1)

---

📘 For protocol reference, see: *Solar Charge Controller (MPPT) Protocol Manual* (provided in your documentation).

