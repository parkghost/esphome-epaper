#include "gooddisplay_gdew042z15.h"

#include <cstdint>

#include "esphome/core/log.h"

// Reference: https://www.e-paper-display.com/products_detail/productId=322.html

namespace esphome {
namespace waveshare_epaper {

const char *const GDEW042Z15::TAG = "gdew042z15";

int GDEW042Z15::get_width_internal() { return WIDTH; }

int GDEW042Z15::get_width_controller() { return WIDTH; }

int GDEW042Z15::get_height_internal() { return HEIGHT; }

uint32_t GDEW042Z15::idle_timeout_() { return IDLE_TIMEOUT; }

void GDEW042Z15::full_refresh() { this->update(); }

void GDEW042Z15::initialize() {}

void GDEW042Z15::display() {
  uint32_t buf_len_half = this->get_buffer_length_() >> 1;
  this->init_display_();

  // Write black Data
  this->command(0x10);
  this->start_data_();
  this->write_array(this->buffer_, buf_len_half);
  this->end_data_();

  // Write red Data
  this->command(0x13);
  this->start_data_();
  for (uint32_t i = buf_len_half; i < buf_len_half * 2u; i++)
    this->write_byte(~this->buffer_[i]);
  this->end_data_();

  this->command(0x12);  // DISPLAY REFRESH
  delay(100);           //!!!The delay here is necessary, 200uS at least!!!
  if (!this->wait_until_idle_())  // waiting for the electronic paper IC to
                                  // release the idle signal
  {
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();

  this->deep_sleep();
}

void GDEW042Z15::init_display_() {
  if (!initial_) {
    reset_();
    initial_ = true;
  }

  if (hibernating_) reset_();  // Electronic paper IC reset

  this->command(0x06);  // boost soft start
  this->data(0x17);     // A
  this->data(0x17);     // B
  this->data(0x17);     // C

  this->command(0x04);            // Power on
  if (!this->wait_until_idle_())  // waiting for the electronic paper IC to
                                  // release the idle signal
  {
    this->status_set_warning();
    return;
  }

  this->command(0x00);  // panel setting
  this->data(0x0f);     // LUT from OTP£¬400x300
  this->data(0x0d);     // VCOM to 0V fast
}

void GDEW042Z15::reset_() {
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->digital_write(false);
    delay(100);
    this->reset_pin_->digital_write(true);
    delay(100);
  }

  hibernating_ = false;
}

void GDEW042Z15::deep_sleep() {
  if (hibernating_) return;

  this->command(0x50);  // VCOM AND DATA INTERVAL SETTING
  this->data(0xf7);     // WBmode:VBDF 17|D7 VBDW 97 VBDB 57    WBRmode:VBDF F7
                        // VBDW 77 VBDB 37  VBDR B7

  this->command(0x02);            // power off
  if (!this->wait_until_idle_())  // waiting for the electronic paper IC to
                                  // release the idle signal
    return;

  // delay(100);          //!!!The delay here is necessary,100mS at least!!!
  this->command(0x07);  // deep sleep
  this->data(0xA5);
  hibernating_ = true;
}

void GDEW042Z15::dump_config() {
  LOG_DISPLAY("", "Good Display e-Paper", this)
  ESP_LOGCONFIG(TAG, "  Model: GDEW042Z15");
  LOG_PIN("  CS Pin: ", this->cs_)
  LOG_PIN("  Reset Pin: ", this->reset_pin_)
  LOG_PIN("  DC Pin: ", this->dc_pin_)
  LOG_PIN("  Busy Pin: ", this->busy_pin_)
  LOG_UPDATE_INTERVAL(this)
}

}  // namespace waveshare_epaper
}  // namespace esphome