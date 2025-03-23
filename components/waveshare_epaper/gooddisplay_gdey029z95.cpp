#include "gooddisplay_gdey029z95.h"

#include <cstdint>

#include "esphome/core/log.h"

// Reference: https://www.good-display.com/product/527.html

namespace esphome {
namespace waveshare_epaper {
const char *const GDEY029Z95::TAG = "gdey029z95";

#ifdef USE_ESP32
RTC_DATA_ATTR uint32_t GDEY029Z95::at_update_ = 0;
#endif

int GDEY029Z95::get_width_internal() { return WIDTH; }

int GDEY029Z95::get_height_internal() { return HEIGHT; }

uint32_t GDEY029Z95::idle_timeout_() { return IDLE_TIMEOUT; }

void GDEY029Z95::set_full_update_every(uint32_t full_update_every) {
  this->full_update_every_ = full_update_every;
}

void GDEY029Z95::full_refresh() {
  this->at_update_ = 0;
  this->update();
}

void GDEY029Z95::initialize() {
#ifdef USE_ESP32
  esp_reset_reason_t reason = esp_reset_reason();
  if (reason == ESP_RST_EXT) this->at_update_ = 0;
#endif
}

void GDEY029Z95::display() {
  RefreshMode mode = this->at_update_ == 0 ? FULL_REFRESH : FAST_REFRESH;
  this->at_update_ = (this->at_update_ + 1) % this->full_update_every_;

  this->init_display_();

  uint32_t buf_len_half = this->get_buffer_length_() >> 1;

  // Write black Data
  this->command(0x24);
  this->start_data_();
  this->write_array(this->buffer_, buf_len_half);
  this->end_data_();

  // Write red Data
  this->command(0x26);
  this->start_data_();
  this->write_array(this->buffer_ + buf_len_half, buf_len_half);
  this->end_data_();

  switch (mode) {
    case FULL_REFRESH:
      this->command(0x22);  // Display Update Control
      this->data(0xF7);

      this->command(0x20);  // Activate Display Update Sequence
      break;

    case FAST_REFRESH:
      this->command(0x1A);  // Write to temperature register
      this->data(0x5a);     // 90
      this->data(0x00);

      this->command(0x22);  // Display Update Sequence Options
      this->data(0x91);     // Load LUT for temperature value

      this->command(0x20);  // Master Activation
      delay(2);             // less than 1 ms measured

      this->command(0x22);  // Display Update Sequence Options
      this->data(0xC7);     //

      this->command(0x20);  // Master Activation

    default:
      ESP_LOGE(TAG, "unsupported refresh mode, mode:%d", mode);
      break;
  }

  if (!this->wait_until_idle_())  // waiting for the electronic paper IC to
                                  // release the idle signal
  {
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();

  this->deep_sleep();
}

void GDEY029Z95::init_display_() {
  if (!initial_) {
    reset_();
    initial_ = true;
  }

  if (hibernating_) reset_();

  this->wait_until_idle_();
  this->command(0x12);  // SWRESET
  delay(10);            // 4ms meaured

  this->command(0x01);  // Driver output control
  this->data((HEIGHT - 1) % 256);
  this->data((HEIGHT - 1) / 256);
  this->data(0x00);

  this->command(0x3C);  // BorderWavefrom
  this->data(0x05);

  this->command(0x18);  // Read built-in temperature sensor
  this->data(0x80);

  this->command(0x11);  // set ram entry mode
  this->data(0x03);     // x increase, y increase : normal mode

  this->command(0x44);
  this->data(0x00);
  this->data((WIDTH - 1) / 8);

  this->command(0x45);
  this->data(0x00);
  this->data(0x00);
  this->data((HEIGHT - 1) % 256);
  this->data((HEIGHT - 1) / 256);

  this->command(0x4e);
  this->data(0x00);

  this->command(0x4f);
  this->data(0x00);
  this->data(0x00);
}

void GDEY029Z95::reset_() {
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->digital_write(false);
    delay(10);
    this->reset_pin_->digital_write(true);
    delay(10);
    hibernating_ = false;
  }
  this->wait_until_idle_();
}

void GDEY029Z95::deep_sleep() {
  if (hibernating_) return;

  // power off
  this->command(0x22);
  this->data(0xc3);
  this->command(0x20);

  if (this->reset_pin_ != nullptr) {
    this->command(0x10);  // deep sleep
    this->data(0x11);     // deep sleep
  }
  hibernating_ = true;
}

void GDEY029Z95::dump_config() {
  LOG_DISPLAY("", "Good Display e-Paper", this)
  ESP_LOGCONFIG(TAG, "  Model: GDEY029Z95");
  LOG_PIN("  CS Pin: ", this->cs_)
  LOG_PIN("  Reset Pin: ", this->reset_pin_)
  LOG_PIN("  DC Pin: ", this->dc_pin_)
  LOG_PIN("  Busy Pin: ", this->busy_pin_)
  LOG_UPDATE_INTERVAL(this)
}

}  // namespace waveshare_epaper
}  // namespace esphome