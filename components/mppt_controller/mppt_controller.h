#pragma once
#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace mppt_controller {

// Forward declarations
class MPPTController;

// Switch class definition
class MPPTLoadSwitch : public switch_::Switch {
 public:
  void set_controller(MPPTController *controller) { controller_ = controller; }
  void write_state(bool state) override;

 protected:
  MPPTController *controller_{nullptr};
};

// Main controller class
class MPPTController : public PollingComponent, public uart::UARTDevice {
 public:
  MPPTController() = default;
  void setup() override;
  void update() override;
  void dump_config() override;

  void set_pv_voltage_sensor(sensor::Sensor *s) { pv_voltage_sensor_ = s; }
  void set_pv_current_sensor(sensor::Sensor *s) { pv_current_sensor_ = s; }
  void set_pv_power_sensor(sensor::Sensor *s) { pv_power_sensor_ = s; }
  void set_battery_voltage_sensor(sensor::Sensor *s) { battery_voltage_sensor_ = s; }
  void set_temperature_sensor(sensor::Sensor *s) { temperature_sensor_ = s; }
  void set_load_current_sensor(sensor::Sensor *s) { load_current_sensor_ = s; }
  void set_daily_energy_sensor(sensor::Sensor *s) { daily_energy_sensor_ = s; }
  void set_total_energy_sensor(sensor::Sensor *s) { total_energy_sensor_ = s; }
  // Add text sensor setters
  void set_error_code_sensor(text_sensor::TextSensor *s) { error_code_sensor_ = s; }
  void set_working_mode_sensor(text_sensor::TextSensor *s) { working_mode_sensor_ = s; }

  // Safe methods only
  void reset_energy(bool clear = true, bool reboot = false);
  void set_charging_params(uint8_t charge_current, uint8_t battery_type, uint16_t const_voltage, uint16_t load_undervoltage);
  void set_load_output(uint8_t mode);

 protected:
  sensor::Sensor *pv_voltage_sensor_{nullptr};
  sensor::Sensor *pv_current_sensor_{nullptr};
  sensor::Sensor *pv_power_sensor_{nullptr};
  sensor::Sensor *battery_voltage_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *load_current_sensor_{nullptr};
  sensor::Sensor *daily_energy_sensor_{nullptr};
  sensor::Sensor *total_energy_sensor_{nullptr};

  // Add text sensor pointers
  text_sensor::TextSensor *error_code_sensor_{nullptr};
  text_sensor::TextSensor *working_mode_sensor_{nullptr};

  float round_to_precision(float value, int decimals = 3);
  // Configuration read/write methods
  std::vector<uint8_t> read_configuration();
  bool write_configuration(const std::vector<uint8_t> &data_bytes);
  
  // Helper methods
  void send_frame(const std::vector<uint8_t> &frame);
  std::vector<uint8_t> read_frame();
  uint8_t checksum(const std::vector<uint8_t> &frame);
  void parse_running_status(const std::vector<uint8_t> &frame);

  // Add helper methods for text conversion
  std::string get_error_code_text(uint8_t error_code);
  std::string get_working_mode_text(uint8_t working_mode);

};

// Action classes
template<typename... Ts> class MPPTResetAction : public Action<Ts...> {
 public:
  explicit MPPTResetAction(MPPTController *controller) : controller_(controller) {}
  
  void play() override {
    if (controller_ != nullptr) {
      controller_->reset_energy(clear_, reboot_);
    }
  }
  
  void set_clear(bool clear) { clear_ = clear; }
  void set_reboot(bool reboot) { reboot_ = reboot; }

 protected:
  MPPTController *controller_;
  bool clear_{true};
  bool reboot_{false};
};

template<typename... Ts> class MPPTSetChargingParamsAction : public Action<Ts...> {
 public:
  explicit MPPTSetChargingParamsAction(MPPTController *controller) : controller_(controller) {}
  
  void play() override {
    if (controller_ != nullptr) {
      controller_->set_charging_params(charge_current_, battery_type_, const_voltage_, load_undervoltage_);
    }
  }
  
  void set_charge_current(uint8_t current) { charge_current_ = current; }
  void set_battery_type(uint8_t type) { battery_type_ = type; }
  void set_const_voltage(uint16_t voltage) { const_voltage_ = voltage; }
  void set_load_undervoltage(uint16_t voltage) { load_undervoltage_ = voltage; }

 protected:
  MPPTController *controller_;
  uint8_t charge_current_{0};
  uint8_t battery_type_{0};
  uint16_t const_voltage_{0};
  uint16_t load_undervoltage_{0};
};

}  // namespace mppt_controller
}  // namespace esphome
