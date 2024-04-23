#include "seesaw_neopixel.h"
#include "esphome/core/log.h"

namespace esphome {
namespace seesaw {

static const char *const TAG = "seesaw.neopixel";

void SeesawNeopixel::setup() {
  this->parent_->setup_neopixel(this->pin_, this->num_leds_);
}

void SeesawNeopixel::set_num_leds(uint16_t num_leds) {
  this->num_leds_ = num_leds;

  // Cleanup if already allocated
  if (this->buf_ != nullptr) delete[] this->buf_;
  if (this->effect_data_ != nullptr) delete[] this->effect_data_;

  // Byte each for Red, Green, and Blue
  this->buf_ = new uint8_t[this->size() * 3];      // NOLINT
  this->effect_data_ = new uint8_t[this->size()];  // NOLINT

  // Clear buffer
  memset(this->buf_, 0x00, this->size() * 3);
  memset(this->effect_data_, 0x00, this->size());
}

light::LightTraits SeesawNeopixel::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::RGB});
  return traits;
}

void SeesawNeopixel::write_state(light::LightState *state) {
  for(uint16_t i = 0; i < this->size(); i++) {
    auto view = get_view_internal(i);
    this->parent_->color_neopixel(i, view.get_red(), view.get_green(), view.get_blue());
  }
}

}  // namespace seesaw
}  // namespace esphome

