#include "mppt_controller.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mppt_controller {

static const char *const TAG = "mppt";

// Add helper methods for text conversion
std::string MPPTController::get_error_code_text(uint8_t error_code) {
  switch(error_code) {
    case 0x00: return "No Error";
    case 0x01: return "Battery Overvoltage";
    case 0x02: return "Battery Undervoltage";
    case 0x04: return "PV Overvoltage";
    case 0x08: return "PV Overcurrent";
    case 0x10: return "Controller Overheating";
    case 0x20: return "Load Overcurrent";
    case 0x40: return "Short Circuit";
    default: return "Unknown Error";
  }
}

std::string MPPTController::get_working_mode_text(uint8_t working_mode) {
  switch(working_mode) {
    case 0x01: return "Tracking";
    case 0x02: return "Night";
    case 0x03: return "Constant Voltage";
    case 0x04: return "Equalizing";
    case 0x05: return "Boost";
    case 0x06: return "Float";
    default: return "Unknown";
  }
}
// Switch implementation
void MPPTLoadSwitch::write_state(bool state) {
  ESP_LOGD(TAG, "Load switch set to: %s", state ? "ON" : "OFF");
  if (controller_ != nullptr) {
    controller_->set_load_output(state ? 1 : 0);
  }
  this->publish_state(state);
}

void MPPTController::dump_config() {
  ESP_LOGCONFIG(TAG, "MPPT Controller:");
  ESP_LOGCONFIG(TAG, "  Update Interval: %u ms", this->get_update_interval());
}

void MPPTController::setup() {
  ESP_LOGI(TAG, "MPPT controller initialized");
  while (this->available() > 0) this->read();
}

void MPPTController::update() {
  ESP_LOGD(TAG, "Starting update cycle - Querying running status");
  
  // Build frame for QUERY RUNNING STATUS (Data Type 0x00)
  std::vector<uint8_t> frame(21, 0x00);
  frame[0] = 0x5A;  // Frame Header
  frame[1] = 0xA5;  // Frame Header  
  frame[2] = 0x01;  // IP Address = 1
  frame[3] = 0x00;  // Data Type = 0x00 (Query Running Status)
  frame[20] = checksum(frame);
  
  // Log request
  std::string request_hex;
  for (int i = 0; i < 21; i++) {
    char buf[4];
    sprintf(buf, "%02X ", frame[i]);
    request_hex += buf;
  }
  ESP_LOGD(TAG, "Sending query: %s", request_hex.c_str());
  
  // Clear buffer and send
  while (this->available() > 0) this->read();
  send_frame(frame);
  delay(300);
  
  auto resp = read_frame();
  
  // Log response
  if (resp.size() > 0) {
    std::string response_hex;
    for (auto byte : resp) {
      char buf[4];
      sprintf(buf, "%02X ", byte);
      response_hex += buf;
    }
    ESP_LOGD(TAG, "Received: %s (size: %d)", response_hex.c_str(), resp.size());
  }
  
  // Parse response according to SM20210402T1 (Running Status Response)
  if (resp.size() >= 21 && resp[0] == 0xAA && resp[1] == 0xBB) {
    // Validate checksum
    uint8_t calc_checksum = 0;
    for (int i = 2; i < 20; i++) calc_checksum += resp[i];
    calc_checksum %= 256;
    
    if (resp[20] != calc_checksum) {
      ESP_LOGW(TAG, "Checksum mismatch: got 0x%02X, expected 0x%02X", resp[20], calc_checksum);
      return;
    }
    
    ESP_LOGD(TAG, "Valid running status response, parsing...");
    parse_running_status(resp);
  } else {
    if (resp.size() > 0) {
      ESP_LOGW(TAG, "Invalid header: 0x%02X 0x%02X (expected AA BB)", resp[0], resp[1]);
    } else {
      ESP_LOGW(TAG, "No response received");
    }
  }
}

float MPPTController::round_to_precision(float value, int decimals) {
  float multiplier = pow(10, decimals);
  return round(value * multiplier) / multiplier;
}

void MPPTController::parse_running_status(const std::vector<uint8_t> &frame) {
  // Parse values with precision rounding
  float pv_voltage = round_to_precision(((frame[4] << 8) | frame[5]) / 10.0f);
  float pv_current = round_to_precision(((frame[6] << 8) | frame[7]) / 10.0f);  
  float pv_power = round_to_precision(pv_voltage * pv_current);
  float battery_voltage = round_to_precision(((frame[8] << 8) | frame[9]) / 10.0f);
  float temperature = round_to_precision(((frame[10] << 8) | frame[11]) / 10.0f);
  float load_current = round_to_precision(((frame[12] << 8) | frame[13]) / 10.0f);
  
  float daily_energy = round_to_precision(((frame[15] << 8) | frame[16]) / 100.0f);
  float total_energy = round_to_precision(((frame[17] << 8) | frame[18]) / 10.0f);

  // Working mode from byte 14 (like Python)
  uint8_t working_mode = frame[14];
  // Error code from byte 19 (like Python)
  uint8_t error_code = frame[19];


  // Convert to text
  std::string working_mode_str = get_working_mode_text(working_mode);
  std::string error_code_str = get_error_code_text(error_code);

  ESP_LOGI(TAG, "Running Status:");
  ESP_LOGI(TAG, "  PV: %.1fV, %.1fA, %.1fW", pv_voltage, pv_current, pv_power);
  ESP_LOGI(TAG, "  Battery: %.1fV, Temp: %.1f°C", battery_voltage, temperature);
  ESP_LOGI(TAG, "  Load: %.1fA", load_current);
  ESP_LOGI(TAG, "  Energy: Daily %.2fkWh, Total %.1fkWh", daily_energy, total_energy);
  ESP_LOGI(TAG, "  Error Code: %d", error_code);

  // Publish sensor values
  if (pv_voltage_sensor_) pv_voltage_sensor_->publish_state(pv_voltage);
  if (pv_current_sensor_) pv_current_sensor_->publish_state(pv_current);
  if (pv_power_sensor_) pv_power_sensor_->publish_state(pv_power);
  if (battery_voltage_sensor_) battery_voltage_sensor_->publish_state(battery_voltage);
  if (temperature_sensor_) temperature_sensor_->publish_state(temperature);
  if (load_current_sensor_) load_current_sensor_->publish_state(load_current);
  if (daily_energy_sensor_) daily_energy_sensor_->publish_state(daily_energy);
  if (total_energy_sensor_) total_energy_sensor_->publish_state(total_energy);
  // Publish text sensor values
  if (working_mode_sensor_) working_mode_sensor_->publish_state(working_mode_str);
  if (error_code_sensor_) error_code_sensor_->publish_state(error_code_str);
  
}

// Configuration read/write methods
std::vector<uint8_t> MPPTController::read_configuration() {
  ESP_LOGD(TAG, "Reading MPPT configuration...");
  
  // Build QUERY PARAMETERS frame (Data Type 0x02)
  std::vector<uint8_t> frame(21, 0x00);
  frame[0] = 0x5A; frame[1] = 0xA5; frame[2] = 0x01; frame[3] = 0x02;
  frame[20] = checksum(frame);
  
  while (this->available() > 0) this->read();
  send_frame(frame);
  delay(300);
  
  auto resp = read_frame();
  
  if (resp.size() >= 21 && resp[0] == 0xAA && resp[1] == 0xBB && resp[2] == 0x02) {
    // Validate checksum
    uint8_t calc_checksum = 0;
    for (int i = 2; i < 20; i++) calc_checksum += resp[i];
    calc_checksum %= 256;
    
    if (resp[20] == calc_checksum) {
      ESP_LOGD(TAG, "Configuration read successfully");
      // Return the 16 data bytes (index 3-18) from parameter response
      return std::vector<uint8_t>(resp.begin() + 3, resp.begin() + 19);
    }
  }
  
  ESP_LOGW(TAG, "Failed to read configuration");
  return std::vector<uint8_t>();
}

bool MPPTController::write_configuration(const std::vector<uint8_t> &data_bytes) {
  if (data_bytes.size() != 16) {
    ESP_LOGE(TAG, "Invalid data bytes size");
    return false;
  }
  
  // Build SET PARAMETERS frame (Data Type 0x01)
  std::vector<uint8_t> frame(21, 0x00);
  frame[0] = 0x5A; frame[1] = 0xA5; frame[2] = 0x01; frame[3] = 0x01;
  
  for (int i = 0; i < 16; i++) {
    frame[4 + i] = data_bytes[i];
  }
  
  frame[20] = checksum(frame);
  
  while (this->available() > 0) this->read();
  send_frame(frame);
  delay(800);
  
  auto resp = read_frame();
  
  // MPPT should respond with ACK, NAK, or RAK
  if (resp.size() >= 3) {
    std::string response(resp.begin(), resp.end());
    ESP_LOGD(TAG, "MPPT response: %s", response.c_str());
    return response.find("ACK") != std::string::npos;
  }
  
  return false;
}

// Safe public methods
void MPPTController::reset_energy(bool clear, bool reboot) {
  ESP_LOGI(TAG, "Safe reset energy: clear=%s, reboot=%s", clear?"true":"false", reboot?"true":"false");
  auto config_data = read_configuration();
  if (config_data.empty()) {
    ESP_LOGE(TAG, "Cannot read configuration");
    return;
  }
  
  // Modify reset bits (byte 10 in data bytes)
  uint8_t reset_bits = 0;
  if (clear) reset_bits |= 0x01;
  if (reboot) reset_bits |= 0x02;
  config_data[10] = reset_bits;
  
  if (write_configuration(config_data)) {
    ESP_LOGI(TAG, "Safe reset completed");
  } else {
    ESP_LOGE(TAG, "Safe reset failed");
  }
}

void MPPTController::set_charging_params(uint8_t charge_current, uint8_t battery_type, 
                                        uint16_t const_voltage, uint16_t load_undervoltage) {
  ESP_LOGI(TAG, "Safe set charging params");
  auto config_data = read_configuration();
  if (config_data.empty()) return;
  
  config_data[0] = charge_current;                    // Byte 0
  config_data[1] = battery_type;                      // Byte 1
  config_data[2] = (const_voltage >> 8) & 0xFF;       // Byte 2
  config_data[3] = const_voltage & 0xFF;              // Byte 3
  config_data[8] = (load_undervoltage >> 8) & 0xFF;   // Byte 8
  config_data[9] = load_undervoltage & 0xFF;          // Byte 9
  
  if (write_configuration(config_data)) {
    ESP_LOGI(TAG, "Safe charging params set");
  } else {
    ESP_LOGE(TAG, "Safe charging params failed");
  }
}

void MPPTController::set_load_output(uint8_t mode) {
  ESP_LOGI(TAG, "Safe set load output: %d", mode);
  auto config_data = read_configuration();
  if (config_data.empty()) return;
  
  config_data[11] = mode;  // Byte 11 is LOAD Output Control
  
  if (write_configuration(config_data)) {
    ESP_LOGI(TAG, "Safe load output set");
  } else {
    ESP_LOGE(TAG, "Safe load output failed");
  }
}

// Helper methods
void MPPTController::send_frame(const std::vector<uint8_t> &frame) {
  this->write_array(frame.data(), frame.size());
  this->flush();
}

std::vector<uint8_t> MPPTController::read_frame() {
  std::vector<uint8_t> resp;
  int max_bytes = 64;
  while (this->available() > 0 && max_bytes-- > 0) {
    resp.push_back(this->read());
  }
  return resp;
}

uint8_t MPPTController::checksum(const std::vector<uint8_t> &frame) {
  uint16_t sum = 0;
  for (int i = 2; i < 20; i++) sum += frame[i];
  return sum % 256;
}

}  // namespace mppt_controller
}  // namespace esphome
