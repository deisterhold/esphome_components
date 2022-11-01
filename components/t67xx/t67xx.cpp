#include "t67xx.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace t67xx {

static const char *const TAG = "t67xx";

static const uint8_t status[5] = {0x04, 0x13, 0x8a, 0x00, 0x01};
static const uint8_t gas_ppm[5] = {0x04, 0x13, 0x8b, 0x00, 0x01};
static const uint8_t calibrate[5] = {0x05, 0x03, 0xec, 0xff, 0x00};

void T67xx::loop() {
  if (this->status_has_warning())
    return;

  if (this->calibrating_ != nullptr) {
    uint8_t data[4];
    if (this->write(status, 5) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "error writing to sensor");
      this->set_has_warning();
      return;
    }
    if (this->read(data, 4) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "error reading status");
      this->set_has_warning();
      return;
    }
    this->calibrating_->publish_state(data[2] & 0x80);
  }
}

void T67xx::dump_config() {
  LOG_SENSOR("", "T67XX", this);
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  LOG_BINARY_SENSOR("  ", "Is calibrating", this->calibrating_);
}

void T67xx::update() {
  uint8_t data[4];
  this->status_clear_warning();
  if (this->write(status, 5) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "error writing to sensor");
    this->set_has_warning();
    return;
  }
  if (this->read(data, 4) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "error reading status");
    this->set_has_warning();
    return;
  }
  if (data[3] & 2) {
    ESP_LOGE(TAG, "flash error");
    this->set_has_warning();
    return;
  }
  if (data[3] & 4) {
    ESP_LOGE(TAG, "calibration error");
    this->start_calibration();
    return;
  }
  if (data[3] & 1) {
    ESP_LOGE(TAG, "unkown error");
    this->set_has_warning();
    return;
  }
  if (data[2] & 8) {
    ESP_LOGD(TAG, "still warming up");
    return;
  }
  if (data[2] & 0x80) {
    ESP_LOGD(TAG, "calibrating");
    return;
  }

  if (this->write(gas_ppm, 5) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "error writing to sensor");
    this->set_has_warning();
    return;
  }
  if (this->read(data, 4) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "error reading data");
    this->set_has_warning();
    return;
  }

  this->publish_state((data[2] << 8) + data[3]);
}

void T67xx::start_calibration() {
  uint8_t data[5];
  ESP_LOGD(TAG, "starting calibration");
  if (this->write(calibrate, 5) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "error writing to sensor");
    this->set_has_warning();
    return;
  }
  if (this->read(data, 5) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "error reading data");
    this->set_has_warning();
    return;
  }
}

}  // namespace t67xx
}  // namespace esphome
