# components/mppt_controller/mppt_controller.h
#pragma once
#include "esphome.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace mppt_controller {

class MPPTController : public PollingComponent, public uart::UARTDevice {
 public:
  MPPTController() = default;
  void setup() override;
  void update() override;

  void set_pv_voltage_sensor(sensor::Sensor *s) { pv_voltage_sensor_ = s; }
  void set_pv_current_sensor(sensor::Sensor *s) { pv_current_sensor_ = s; }
  void set_battery_voltage_sensor(sensor::Sensor *s) { battery_voltage_sensor_ = s; }
  void set_temperature_sensor(sensor::Sensor *s) { temperature_sensor_ = s; }
  void set_load_current_sensor(sensor::Sensor *s) { load_current_sensor_ = s; }
  void set_daily_energy_sensor(sensor::Sensor *s) { daily_energy_sensor_ = s; }
  void set_total_energy_sensor(sensor::Sensor *s) { total_energy_sensor_ = s; }
  void set_load_switch(switch_::Switch *s) { load_switch_ = s; }

  void reset_energy(bool clear = true, bool reboot = false);
  void set_charging_params(uint8_t charge_current, uint8_t battery_type, uint16_t const_voltage, uint16_t load_undervoltage);
  void set_load_output(uint8_t mode);

 protected:
  sensor::Sensor *pv_voltage_sensor_{nullptr};
  sensor::Sensor *pv_current_sensor_{nullptr};
  sensor::Sensor *battery_voltage_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *load_current_sensor_{nullptr};
  sensor::Sensor *daily_energy_sensor_{nullptr};
  sensor::Sensor *total_energy_sensor_{nullptr};
  switch_::Switch *load_switch_{nullptr};

  void send_frame(const std::vector<uint8_t> &frame);
  std::vector<uint8_t> read_frame();
  uint8_t checksum(const std::vector<uint8_t> &frame);
  void parse_response(const std::vector<uint8_t> &frame);
};

}  // namespace mppt_controller
}  // namespace esphome

