#include "p750057_mf1_a.h"
#include <cstdint>
#include "esphome/core/log.h"

// Reference: https://github.com/gooddisplayshare/ESP32epdx/tree/main/examples/3-Colors%20(BWR)/7.5/P750057MF1A

namespace esphome
{
  namespace waveshare_epaper
  {

    const char *const P750057MF1A::TAG = "p750057-mf1-a";

    RTC_DATA_ATTR uint32_t P750057MF1A::at_update_ = 0;

    int P750057MF1A::get_width_internal() { return WIDTH; }

    int P750057MF1A::get_width_controller() { return WIDTH; }

    int P750057MF1A::get_height_internal() { return HEIGHT; }

    uint32_t P750057MF1A::idle_timeout_()
    {
      return IDLE_TIMEOUT;
    }

    void P750057MF1A::set_full_update_every(uint32_t full_update_every)
    {
      this->full_update_every_ = full_update_every;
    }

    void P750057MF1A::full_refresh()
    {
      this->at_update_ = 0;
      this->update();
    }

    void P750057MF1A::initialize()
    {
    }

    void P750057MF1A::display()
    {
      bool full_update = this->at_update_ == 0;
      this->at_update_ = (this->at_update_ + 1) % this->full_update_every_;

      this->init_display_(full_update ? FULL_REFRESH : FAST_REFRESH);

      uint32_t buf_len_half = this->get_buffer_length_() >> 1;

      // Write black Data
      this->command(0x10);
      this->start_data_();
      this->write_array(this->buffer_, buf_len_half);
      this->end_data_();

      // Write red Data
      this->command(0x13);
      this->start_data_();
      this->write_array(this->buffer_ + buf_len_half, buf_len_half);
      this->end_data_();

      this->command(0x12);           // DISPLAY REFRESH
      delay(100);                    //!!!The delay here is necessary, 200uS at least!!!
      if (!this->wait_until_idle_()) // waiting for the electronic paper IC to release the idle signal
      {
        this->status_set_warning();
        return;
      }

      this->status_clear_warning();

      this->deep_sleep();
    }

    // Fast update 1 initialization
    void P750057MF1A::init_display_(RefreshMode mode)
    {
      if (hibernating_)
        reset_(); // Electronic paper IC reset

      this->wait_until_idle_();

      if (mode == FULL_REFRESH)
      {
        this->command(0x01); // POWER SETTING
        this->data(0x07);
        this->data(0x07); // VGH=20V,VGL=-20V
        this->data(0x3f); // VDH=15V
        this->data(0x3f); // VDL=-15V

        // Enhanced display drive(Add 0x06 command)
        this->command(0x06); // Booster Soft Start
        this->data(0x17);
        this->data(0x17);
        this->data(0x28);
        this->data(0x17);

        this->command(0x04);      // POWER ON
        this->wait_until_idle_(); // waiting for the electronic paper IC to release the idle signal

        this->command(0X00); // PANNEL SETTING
        this->data(0x0F);    // KW-3f   KWR-2F	BWROTP 0f	BWOTP 1f

        this->command(0x61); // resolution setting
        this->data(WIDTH / 256);
        this->data(WIDTH % 256);
        this->data(HEIGHT / 256);
        this->data(HEIGHT % 256);

        this->command(0X15);
        this->data(0x00);

        this->command(0X50); // VCOM AND DATA INTERVAL SETTING
        this->data(0x11);    // 0x10  --------------
        this->data(0x07);

        this->command(0X60); // TCON SETTING
        this->data(0x22);
      }
      else if (mode == FAST_REFRESH)
      {

        this->command(0X00); // PANNEL SETTING
        this->data(0x0F);    // KW-3f   KWR-2F	BWROTP 0f	BWOTP 1f

        this->command(0x04); // POWER ON
        delay(100);
        this->wait_until_idle_(); // waiting for the electronic paper IC to release the idle signal

        // Enhanced display drive(Add 0x06 command)
        this->command(0x06); // Booster Soft Start
        this->data(0x27);
        this->data(0x27);
        this->data(0x18);
        this->data(0x17);

        this->command(0xE0);
        this->data(0x02);
        this->command(0xE5);
        this->data(0x5A);

        this->command(0X50); // VCOM AND DATA INTERVAL SETTING
        this->data(0x11);
        this->data(0x07);
      }
    }

    void P750057MF1A::reset_()
    {
      if (this->reset_pin_ != nullptr)
      {
        this->reset_pin_->digital_write(false);
        delay(100);
        this->reset_pin_->digital_write(true);
        delay(100);
      }

      hibernating_ = false;
    }

    void P750057MF1A::deep_sleep()
    {
      if (hibernating_)
        return;

      this->command(0X50); // VCOM AND DATA INTERVAL SETTING
      this->data(0xf7);    // WBmode:VBDF 17|D7 VBDW 97 VBDB 57    WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7

      this->command(0X02);           // power off
      if (!this->wait_until_idle_()) // waiting for the electronic paper IC to release the idle signal
        return;

      // delay(100);          //!!!The delay here is necessary,100mS at least!!!
      this->command(0X07); // deep sleep
      this->data(0xA5);
      hibernating_ = true;
    }

    void P750057MF1A::dump_config()
    {
      LOG_DISPLAY("", "Good Display e-Paper", this)
      ESP_LOGCONFIG(TAG, "  Model: P750057MF1A");
      LOG_PIN("  CS Pin: ", this->cs_)
      LOG_PIN("  Reset Pin: ", this->reset_pin_)
      LOG_PIN("  DC Pin: ", this->dc_pin_)
      LOG_PIN("  Busy Pin: ", this->busy_pin_)
      LOG_UPDATE_INTERVAL(this)
    }

  } // namespace waveshare_epaper
} // namespace esphome