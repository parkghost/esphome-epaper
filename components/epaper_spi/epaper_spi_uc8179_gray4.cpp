#include "epaper_spi_uc8179_gray4.h"

#include <algorithm>

#include "esphome/core/log.h"

namespace esphome::epaper_spi {

static const char *const TAG = "epaper_spi.uc8179_gray4";

void EPaperUC8179Gray4Base::setup() {
  EPaperBase::setup();
  if (this->is_using_partial_update_()) {
    ESP_LOGW(TAG, "full_update_every is ignored: 4 gray levels always need a full refresh");
  }
}

void EPaperUC8179Gray4Base::draw_pixel_at(int x, int y, Color color) {
  if (!this->rotate_coordinates_(x, y))
    return;

  const uint32_t pos = (x / 8) + (y * this->row_width_);
  const uint8_t bit = 0x80 >> (x & 0x07);
  const uint32_t lsb_offset = this->buffer_length_ / 2u;
  const uint8_t level = color_to_gray4(color);

  // MSB plane
  if (level & 0x02) {
    this->buffer_[pos] |= bit;
  } else {
    this->buffer_[pos] &= ~bit;
  }

  // LSB plane
  if (level & 0x01) {
    this->buffer_[lsb_offset + pos] |= bit;
  } else {
    this->buffer_[lsb_offset + pos] &= ~bit;
  }
}

void EPaperUC8179Gray4Base::fill(Color color) {
  // Honor active clipping like the base fill(); the base path also sets bounds.
  if (this->get_clipping().is_set()) {
    Display::fill(color);
    return;
  }

  const size_t half_buffer = this->buffer_length_ / 2u;
  const uint8_t level = color_to_gray4(color);
  const uint8_t msb = (level & 0x02) ? 0xFF : 0x00;
  const uint8_t lsb = (level & 0x01) ? 0xFF : 0x00;

  if (msb == lsb) {
    this->buffer_.fill(msb);  // black or white: both planes identical
  } else {
    for (size_t i = 0; i < half_buffer; i++) {
      this->buffer_[i] = msb;
      this->buffer_[half_buffer + i] = lsb;
    }
  }

  // Mark the whole panel dirty so a fill-only update is not skipped by the FSM.
  this->x_low_ = 0;
  this->y_low_ = 0;
  this->x_high_ = this->width_;
  this->y_high_ = this->height_;
}

bool HOT EPaperUC8179Gray4Base::transfer_data() {
  const uint32_t start_time = millis();
  const size_t buffer_length = this->buffer_length_;
  const size_t half_buffer = buffer_length / 2u;

  uint8_t bytes_to_send[MAX_TRANSFER_SIZE];

  // Phase 1: MSB plane (first half), inverted so a set bit is ink
  if (this->current_data_index_ < half_buffer) {
    if (this->current_data_index_ == 0) {
      this->command(this->swap_planes_ ? 0x13 : 0x10);  // DATA START TRANSMISSION 2 / 1
    }
    this->start_data_();
    while (this->current_data_index_ < half_buffer) {
      const size_t n = std::min(MAX_TRANSFER_SIZE, half_buffer - this->current_data_index_);
      for (size_t i = 0; i < n; i++) {
        bytes_to_send[i] = ~this->buffer_[this->current_data_index_ + i];
      }
      this->write_array(bytes_to_send, n);
      this->current_data_index_ += n;
      if (millis() - start_time > MAX_TRANSFER_TIME) {
        this->disable();
        return false;
      }
    }
    this->disable();
  }

  // Phase 2: LSB plane (second half), likewise inverted
  if (this->current_data_index_ < buffer_length) {
    if (this->current_data_index_ == half_buffer) {
      this->command(this->swap_planes_ ? 0x10 : 0x13);  // DATA START TRANSMISSION 1 / 2
    }
    this->start_data_();
    while (this->current_data_index_ < buffer_length) {
      const size_t n = std::min(MAX_TRANSFER_SIZE, buffer_length - this->current_data_index_);
      for (size_t i = 0; i < n; i++) {
        bytes_to_send[i] = ~this->buffer_[this->current_data_index_ + i];
      }
      this->write_array(bytes_to_send, n);
      this->current_data_index_ += n;
      if (millis() - start_time > MAX_TRANSFER_TIME) {
        this->disable();
        return false;
      }
    }
    this->disable();
  }

  this->current_data_index_ = 0;
  return true;
}

void EPaperUC8179Gray4::power_on() {
  // No POWER SETTING (0x01) here: the vendor's OTP 4-gray demo sends none,
  // leaving the OTP values in place.
  //
  // The booster values are the one place this departs from that demo, which
  // uses {0x27, 0x27, 0x18, 0x17}. Byte 3 is phase C1, the phase that sustains
  // VDH/VDL through the refresh, and bits 5:3 are its driving strength: the
  // vendor value is strength 4, this one is strength 6. Strength 4 carries the
  // flat gray bands the vendor demo displays, but not a dithered image, whose
  // far denser source switching drooped the rails and left vertical streaks
  // through the dark areas. Ruled out on hardware before landing here, none of
  // which changed the streaks: POWER SETTING (0x01), VCOM_DC (0x82), PLL
  // (0x30), clearing to white first, refreshing twice, and coarser dither
  // patterns. Strength 6 is what every other UC8179 driver here already sends.
  this->cmd_data(0x06, {0x17, 0x17, 0x28, 0x17});  // Booster Soft Start
  this->command(0x04);                             // POWER ON
}

void EPaperUC8179Gray4Lut::power_on() {
  // Byte 2 is VCOM_SLEW + VG_LVL[2:0]; the datasheet fixes VCOM_SLEW at 1, so
  // 0x17 (not the mono path's 0x07) is the correct value, and it is what the
  // vendor register-LUT demo sends. If the panel shows uneven shading, dropping
  // 0x01 entirely is the vendor-sanctioned workaround.
  this->cmd_data(0x01, {0x07, 0x17, 0x3F, 0x3F});  // POWER SETTING: VGH/VGL 20V, VDH/VDL 15V
  this->cmd_data(0x06, {0x17, 0x17, 0x28, 0x17});  // Booster Soft Start
  this->command(0x04);                             // POWER ON
}

}  // namespace esphome::epaper_spi
