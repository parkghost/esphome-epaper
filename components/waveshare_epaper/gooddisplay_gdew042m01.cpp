#include "gooddisplay_gdew042m01.h"

#include <cstdint>

#include "esphome/core/log.h"

// This implementation is based on the GxEPD2 library
// https://github.com/ZinggJM/GxEPD2/blob/master/src/epd/GxEPD2_420_M01.cpp

namespace esphome {
namespace waveshare_epaper {
const char *const GDEW042M01::TAG = "gdew042m01";

#define T1 20  // charge balance pre-phase
#define T2 20  // optional extension
#define T3 40  // color change phase (b/w)
#define T4 40  // optional extension for one color
#define T5 3   // white sustain phase
#define T6 3   // black sustain phase

const unsigned char GDEW042M01::LUT_VCOM1_PARTIAL[] = {
    0x00, T1,   T2,   T3,   T4,   1,    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const unsigned char GDEW042M01::LUT_WW1_PARTIAL[] = {  // 10 w
    0x02, T1,   T2,   T3,   T5,   1,    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const unsigned char GDEW042M01::LUT_BW1_PARTIAL[] = {  // 10 w
    0x5A, T1,   T2,   T3,   T4,   1,    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const unsigned char GDEW042M01::LUT_WB1_PARTIAL[] = {  // 01 b
    0x84, T1,   T2,   T3,   T4,   1,    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const unsigned char GDEW042M01::LUT_BB1_PARTIAL[] = {  // 01 b
    0x01, T1,   T2,   T3,   T6,   1,    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

int GDEW042M01::get_width_internal() { return WIDTH; }

int GDEW042M01::get_width_controller() { return WIDTH; }

int GDEW042M01::get_height_internal() { return HEIGHT; }

uint32_t GDEW042M01::idle_timeout_() { return IDLE_TIMEOUT; }

void GDEW042M01::set_full_update_every(uint32_t full_update_every) {
  this->full_update_every_ = full_update_every;
}

void GDEW042M01::full_refresh() {
  this->at_update_ = 0;
  this->update();
}

void GDEW042M01::initialize() {}

void GDEW042M01::display() {
  bool full_update = this->at_update_ == 0;
  this->at_update_ = (this->at_update_ + 1) % this->full_update_every_;

  this->init_display_();

  if (full_update) {
    this->command(0x10);  // Transfer old data
    for (uint32_t i = 0; i < this->get_buffer_length_(); i++) {
      this->data(0xff);
    }

    this->command(0x13);  // Transfer new data
    this->start_data_();
    for (uint32_t i = 0; i < this->get_buffer_length_(); i++) {
      this->write_byte(this->buffer_[i]);
      this->oldData[i] = this->buffer_[i];
    }
    this->end_data_();
  } else {
    this->init_part_();

    uint32_t xe = (WIDTH - 1) | 0x0007;

    this->command(0x91);
    this->command(0x90);  // partial window
    this->data(0x00);
    this->data(0x00);
    this->data(xe / 256);
    this->data(xe % 256);
    this->data(0x00);
    this->data(0x00);
    this->data((HEIGHT - 1) / 256);
    this->data((HEIGHT - 1) % 256);
    this->data(0x01);

    this->command(0x10);  // Transfer old data
    this->start_data_();
    this->write_array(
        this->oldData,
        this->get_buffer_length_());  // Transfer the actual displayed data
    this->end_data_();

    this->command(0x13);  // Transfer new data
    this->start_data_();
    this->write_array(
        this->buffer_,
        this->get_buffer_length_());  // Transfer the actual displayed data
    this->end_data_();
    for (uint32_t i = 0; i < this->get_buffer_length_(); i++)
      this->oldData[i] = this->buffer_[i];

    this->command(0x92);
  }

  this->command(0x12);  // DISPLAY REFRESH
  delay(1);             //!!!The delay here is necessary, 200uS at least!!!
  if (!this->wait_until_idle_())  // waiting for the electronic paper IC to
                                  // release the idle signal
  {
    this->status_set_warning();
    return;
  }

  if (!full_update) {
    this->command(0x92);
  }

  this->status_clear_warning();

  this->deep_sleep();
}

void GDEW042M01::init_display_() {
  if (!initial_) {
    reset_();
    initial_ = true;
  }

  if (hibernating_) reset_();

  this->command(0x00);  // panel setting
  this->data(0x1f);     // LUT from OTP��?KW-BF   KWR-AF  BWROTP 0f BWOTP 1f
  this->data(0x0D);

  this->command(0x61);  // resolution setting
  this->data(WIDTH / 256);
  this->data(WIDTH % 256);
  this->data(HEIGHT / 256);
  this->data(HEIGHT % 256);

  this->command(0x04);
  if (!this->wait_until_idle_())  // waiting for the electronic paper IC to
                                  // release the idle signal
    return;

  this->command(0X50);  // VCOM AND DATA INTERVAL SETTING
  this->data(0x97);  // WBmode:VBDF 17|D7 VBDW 97 VBDB 57   WBRmode:VBDF F7 VBDW
                     // 77 VBDB 37  VBDR B7
}

void GDEW042M01::reset_() {
  if (this->reset_pin_ != nullptr) {
    for (uint32_t i = 0; i < 3; i++) {
      this->reset_pin_->digital_write(false);
      delay(10);
      this->reset_pin_->digital_write(true);
      delay(10);
    }
  }

  hibernating_ = false;
}

void GDEW042M01::init_part_() {
  this->command(0x01);  // POWER SETTING
  this->data(0x03);     // VDS_EN, VDG_EN internal
  this->data(0x00);     // VCOM_HV, VGHL_LV=16V
  this->data(0x2b);     // VDH=11V
  this->data(0x2b);     // VDL=11V

  this->command(0x06);  // boost soft start
  this->data(0x17);     // A
  this->data(0x17);     // B
  this->data(0x17);     // C

  this->command(0x00);  // panel setting
  this->data(0x3f);     // 300x400 B/W mode, LUT set by register

  this->command(0x30);  // PLL setting
  this->data(0x3a);     // 3a 100HZ   29 150Hz 39 200HZ 31 171HZ

  this->command(0x61);  // resolution setting
  this->data(WIDTH / 256);
  this->data(WIDTH % 256);
  this->data(HEIGHT / 256);
  this->data(HEIGHT % 256);

  this->command(0x82);  // vcom_DC setting
  this->data(0x1A);     // -0.1 + 26 * -0.05 = -1.4V from OTP

  this->command(0x50);  // VCOM AND DATA INTERVAL SETTING
  this->data(0xd7);     // border floating to avoid flashing

  write_lut_();

  this->command(0x04);
  if (!this->wait_until_idle_())  // waiting for the electronic paper IC to
                                  // release the idle signal
  {
    this->status_set_warning();
    return;
  }
}

void GDEW042M01::write_lut_() {
  this->command(0x20);
  for (uint8_t count = 0; count < 44; count++) {
    this->data(LUT_VCOM1_PARTIAL[count]);
  }

  this->command(0x21);
  for (uint8_t count = 0; count < 42; count++) {
    this->data(LUT_WW1_PARTIAL[count]);
  }

  this->command(0x22);
  for (uint8_t count = 0; count < 42; count++) {
    this->data(LUT_BW1_PARTIAL[count]);
  }

  this->command(0x23);
  for (uint8_t count = 0; count < 42; count++) {
    this->data(LUT_WB1_PARTIAL[count]);
  }

  this->command(0x24);
  for (uint8_t count = 0; count < 42; count++) {
    this->data(LUT_BB1_PARTIAL[count]);
  }
}

void GDEW042M01::deep_sleep() {
  if (hibernating_) return;

  this->command(0x02);            // power off
  if (!this->wait_until_idle_())  // waiting for the electronic paper IC to
                                  // release the idle signal
    return;

  delay(100);           //!!!The delay here is necessary,100mS at least!!!
  this->command(0x07);  // deep sleep
  this->data(0xA5);
  hibernating_ = true;
}

void GDEW042M01::dump_config() {
  LOG_DISPLAY("", "Good Display e-Paper", this)
  ESP_LOGCONFIG(TAG, "  Model: GDEW042M01");
  LOG_PIN("  CS Pin: ", this->cs_)
  LOG_PIN("  Reset Pin: ", this->reset_pin_)
  LOG_PIN("  DC Pin: ", this->dc_pin_)
  LOG_PIN("  Busy Pin: ", this->busy_pin_)
  LOG_UPDATE_INTERVAL(this)
}

}  // namespace waveshare_epaper
}  // namespace esphome