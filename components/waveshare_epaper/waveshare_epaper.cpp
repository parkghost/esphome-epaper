#include "waveshare_epaper.h"

#include <cinttypes>

#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace waveshare_epaper {

static const char *const TAG = "waveshare_epaper";

void WaveshareEPaperBase::setup_pins_() {
  this->init_internal_(this->get_buffer_length_());
  this->dc_pin_->setup();  // OUTPUT
  this->dc_pin_->digital_write(false);
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();  // OUTPUT
    this->reset_pin_->digital_write(true);
  }
  if (this->busy_pin_ != nullptr) {
    this->busy_pin_->setup();  // INPUT
  }
  this->spi_setup();

  this->reset_();
}
float WaveshareEPaperBase::get_setup_priority() const {
  return setup_priority::PROCESSOR;
}
void WaveshareEPaperBase::command(uint8_t value) {
  this->start_command_();
  this->write_byte(value);
  this->end_command_();
}
void WaveshareEPaperBase::data(uint8_t value) {
  this->start_data_();
  this->write_byte(value);
  this->end_data_();
}

// write a command followed by one or more bytes of data.
// The command is the first byte, length is the total including cmd.
void WaveshareEPaperBase::cmd_data(const uint8_t *c_data, size_t length) {
  this->dc_pin_->digital_write(false);
  this->enable();
  this->write_byte(c_data[0]);
  this->dc_pin_->digital_write(true);
  this->write_array(c_data + 1, length - 1);
  this->disable();
}

void WaveshareEPaperBase::cmd_data(const uint8_t cmd, const uint8_t *c_data,
                                   size_t length) {
  this->dc_pin_->digital_write(false);
  this->enable();
  this->write_byte(cmd);
  this->dc_pin_->digital_write(true);
#ifdef USE_ESP32
  this->write_array(c_data, length);
#else
  unsigned int i;
  for (i = 0; i < length; i++) this->data(c_data[i]);
#endif
  this->disable();
}

bool WaveshareEPaperBase::wait_until_idle_() {
  auto is_busy = [this]() -> bool {
    return this->is_busy_pin_inverted_() ? !this->busy_pin_->digital_read()
                                         : this->busy_pin_->digital_read();
  };

  if (this->busy_pin_ == nullptr || !is_busy()) {
    return true;
  }

  const uint32_t start = millis();
  while (is_busy()) {
    if (millis() - start > this->idle_timeout_()) {
      ESP_LOGE(TAG, "Timeout while displaying image!");
      return false;
    }
    App.feed_wdt();
    delay(1);
  }
  return true;
}

void WaveshareEPaperBase::update() {
  this->do_update_();
  this->display();
}
void WaveshareEPaper::fill(Color color) {
  // flip logic
  const uint8_t fill = color.is_on() ? 0x00 : 0xFF;
  for (uint32_t i = 0; i < this->get_buffer_length_(); i++)
    this->buffer_[i] = fill;
}
void HOT WaveshareEPaper::draw_absolute_pixel_internal(int x, int y,
                                                       Color color) {
  if (x >= this->get_width_internal() || y >= this->get_height_internal() ||
      x < 0 || y < 0)
    return;

  const uint32_t pos = (x + y * this->get_width_controller()) / 8u;
  const uint8_t subpos = x & 0x07;
  // flip logic
  if (!color.is_on()) {
    this->buffer_[pos] |= 0x80 >> subpos;
  } else {
    this->buffer_[pos] &= ~(0x80 >> subpos);
  }
}

uint32_t WaveshareEPaper::get_buffer_length_() {
  return this->get_width_controller() * this->get_height_internal() / 8u;
}  // just a black buffer
uint32_t WaveshareEPaperBWR::get_buffer_length_() {
  return this->get_width_controller() * this->get_height_internal() / 4u;
}  // black and red buffer

void WaveshareEPaperBWR::fill(Color color) {
  this->filled_rectangle(0, 0, this->get_width(), this->get_height(), color);
}
void HOT WaveshareEPaperBWR::draw_absolute_pixel_internal(int x, int y,
                                                          Color color) {
  if (x >= this->get_width_internal() || y >= this->get_height_internal() ||
      x < 0 || y < 0)
    return;

  const uint32_t buf_half_len = this->get_buffer_length_() / 2u;

  const uint32_t pos = (x + y * this->get_width_internal()) / 8u;
  const uint8_t subpos = x & 0x07;
  // flip logic
  if (color.is_on()) {
    this->buffer_[pos] |= 0x80 >> subpos;
  } else {
    this->buffer_[pos] &= ~(0x80 >> subpos);
  }

  // draw red pixels only, if the color contains red only
  if (((color.red > 0) && (color.green == 0) && (color.blue == 0))) { // red
    this->buffer_[pos + buf_half_len] |= 0x80 >> subpos;
  } else if (((color.red == 255) && (color.green == 255) && (color.blue == 0))) { // yellow
    this->buffer_[pos + buf_half_len] |= 0x80 >> subpos;
  } else {
    this->buffer_[pos + buf_half_len] &= ~(0x80 >> subpos);
  }
}

void WaveshareEPaperBase::start_command_() {
  this->dc_pin_->digital_write(false);
  this->enable();
}
void WaveshareEPaperBase::end_command_() { this->disable(); }
void WaveshareEPaperBase::start_data_() {
  this->dc_pin_->digital_write(true);
  this->enable();
}
void WaveshareEPaperBase::end_data_() { this->disable(); }
void WaveshareEPaperBase::on_safe_shutdown() { this->deep_sleep(); }

}  // namespace waveshare_epaper
}  // namespace esphome