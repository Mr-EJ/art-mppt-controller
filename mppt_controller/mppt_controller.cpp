# external_components/mppt_controller/mppt_controller.cpp
#include "mppt_controller.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mppt_controller {

static const char *const TAG = "mppt";

void MPPTController::setup() {
  ESP_LOGI(TAG, "MPPT controller initialized");
  if (load_switch_ != nullptr)
    this->load_switch_->add_on_state_callback([this](bool state) {
      set_load_output(state ? 1 : 0);
    });
}

void MPPTController::update() {
  std::vector<uint8_t> frame = {0x5A, 0xA5, 0x01, 0x00};
  frame.resize(21, 0x00);
  frame[20] = checksum(frame);
  send_frame(frame);

  auto resp = read_frame();
  if (resp.size() >= 21 && resp[0] == 0xAA && resp[1] == 0xBB) {
    parse_response(resp);
  }
}

void MPPTController::send_frame(const std::vector<uint8_t> &frame) {
  this->write_array(frame.data(), frame.size());
}

std::vector<uint8_t> MPPTController::read_frame() {
  std::vector<uint8_t> resp;
  int timeout = 1000;
  while (timeout-- > 0 && this->available() == 0) delay(1);
  while (this->available() > 0) resp.push_back(this->read());
  return resp;
}

uint8_t MPPTController::checksum(const std::vector<uint8_t> &frame) {
  uint16_t sum = 0;
  for (int i = 2; i < 20; i++) sum += frame[i];
  return sum % 256;
}

void MPPTController::parse_response(const std::vector<uint8_t> &frame) {
  float pv_voltage = ((frame[3] << 8) | frame[4]) / 10.0f;
  float pv_current = ((frame[5] << 8) | frame[6]) / 10.0f;
  float battery_voltage = ((frame[7] << 8) | frame[8]) / 10.0f;
  float temp = ((frame[9] << 8) | frame[10]) / 10.0f;
  float load_current = ((frame[11] << 8) | frame[12]) / 10.0f;
  float daily = ((frame[15] << 8) | frame[16]) / 100.0f;
  float total = ((frame[17] << 8) | frame[18]) / 10.0f;

  if (pv_voltage_sensor_) pv_voltage_sensor_->publish_state(pv_voltage);
  if (pv_current_sensor_) pv_current_sensor_->publish_state(pv_current);
  if (battery_voltage_sensor_) battery_voltage_sensor_->publish_state(battery_voltage);
  if (temperature_sensor_) temperature_sensor_->publish_state(temp);
  if (load_current_sensor_) load_current_sensor_->publish_state(load_current);
  if (daily_energy_sensor_) daily_energy_sensor_->publish_state(daily);
  if (total_energy_sensor_) total_energy_sensor_->publish_state(total);
}

void MPPTController::reset_energy(bool clear, bool reboot) {
  std::vector<uint8_t> frame(21, 0);
  frame[0] = 0x5A; frame[1] = 0xA5; frame[2] = 0x01; frame[3] = 0x01;
  frame[16] = (clear ? 0x01 : 0x00) | (reboot ? 0x02 : 0x00);
  frame[20] = checksum(frame);
  send_frame(frame);
}

void MPPTController::set_charging_params(uint8_t charge_current, uint8_t battery_type, uint16_t const_voltage, uint16_t load_undervoltage) {
  std::vector<uint8_t> frame(21, 0);
  frame[0] = 0x5A; frame[1] = 0xA5; frame[2] = 0x01; frame[3] = 0x01;
  frame[4] = charge_current;
  frame[5] = battery_type;
  frame[6] = (const_voltage >> 8) & 0xFF;
  frame[7] = const_voltage & 0xFF;
  frame[12] = (load_undervoltage >> 8) & 0xFF;
  frame[13] = load_undervoltage & 0xFF;
  frame[20] = checksum(frame);
  send_frame(frame);
}

void MPPTController::set_load_output(uint8_t mode) {
  std::vector<uint8_t> frame(21, 0);
  frame[0] = 0x5A; frame[1] = 0xA5; frame[2] = 0x01; frame[3] = 0x01;
  frame[14] = mode;
  frame[20] = checksum(frame);
  send_frame(frame);
}

}  // namespace mppt_controller
}  // namespace esphome
