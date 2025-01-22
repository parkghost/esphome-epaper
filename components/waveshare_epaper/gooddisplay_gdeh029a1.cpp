#include "gooddisplay_gdeh029a1.h"
#include <cstdint>
#include "esphome/core/log.h"

// This implementation is based on the GxEPD2 library
// https://github.com/ZinggJM/GxEPD2/blob/master/src/epd/GxEPD2_290.cpp

namespace esphome
{
  namespace waveshare_epaper
  {

    static const char *const TAG = "gdeh029a1";

    static const uint16_t WIDTH = 128;

    static const uint16_t HEIGHT = 296;

    static const uint16_t IDLE_TIMEOUT = 1000;

    static const uint8_t LUT_SIZE = 30;

    static const uint8_t LUT_DATA_full[LUT_SIZE] =
        {
            0x50, 0xAA, 0x55, 0xAA, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    static const uint8_t LUT_DATA_part[LUT_SIZE] =
        {
            0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    int GDEH029A1::get_width_internal() { return WIDTH; }

    int GDEH029A1::get_width_controller() { return WIDTH; }

    int GDEH029A1::get_height_internal() { return HEIGHT; }

    uint32_t GDEH029A1::idle_timeout_()
    {
      return IDLE_TIMEOUT;
    }

    void GDEH029A1::set_full_update_every(uint32_t full_update_every)
    {
      this->full_update_every_ = full_update_every;
    }

    void GDEH029A1::full_refresh()
    {
      this->at_update_ = 0;
      this->update();
    }

    void GDEH029A1::initialize()
    {
    }

    void GDEH029A1::display()
    {
      bool full_update = this->at_update_ == 0;
      this->at_update_ = (this->at_update_ + 1) % this->full_update_every_;

      this->init_display_();
      if (full_update)
      {
        this->write_lut_(LUT_DATA_full, sizeof(LUT_DATA_full));
      }
      else
      {
        this->write_lut_(LUT_DATA_part, sizeof(LUT_DATA_part));
      }
      this->command(0x22);
      this->data(0xc0);
      this->command(0x20);
      if (!this->wait_until_idle_())
      {
        this->status_set_warning();
        return;
      }

      this->setPartialRamArea_(0, 0, WIDTH, HEIGHT);
      this->command(0x24);
      this->start_data_();
      this->write_array(this->buffer_, this->get_buffer_length_());
      this->end_data_();

      this->command(0x22);
      this->data(full_update ? 0xC4 : 0x04);
      this->command(0x20);
      this->command(0xff);

      this->status_clear_warning();

      this->deep_sleep();
    }

    void GDEH029A1::init_display_()
    {
      if (hibernating_)
        reset_();

      this->command(0x01); // Panel configuration, Gate selection
      this->data((HEIGHT - 1) % 256);
      this->data((HEIGHT - 1) / 256);
      this->data(0x00);

      this->command(0x0c); // softstart
      this->data(0xd7);
      this->data(0xd6);
      this->data(0x9d);

      this->command(0x2c); // VCOM setting
      this->data(0xa8);    // * different

      this->command(0x3a); // DummyLine
      this->data(0x1a);    // 4 dummy line per gate

      this->command(0x3b); // Gatetime
      this->data(0x08);    // 2us per line
      setPartialRamArea_(0, 0, WIDTH, HEIGHT);
    }

    void GDEH029A1::reset_()
    {
      if (this->reset_pin_ != nullptr)
      {
        this->reset_pin_->digital_write(false);
        delay(10);
        this->reset_pin_->digital_write(true);
        delay(10);
        hibernating_ = false;
      }
      this->wait_until_idle_();
    }

    void GDEH029A1::setPartialRamArea_(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
    {
      this->command(0x11); // set ram entry mode
      this->data(0x03);    // x increase, y increase : normal mode

      this->command(0x44);
      this->data(x / 8);
      this->data((x + w - 1) / 8);

      this->command(0x45);
      this->data(y % 256);
      this->data(y / 256);
      this->data((y + h - 1) % 256);
      this->data((y + h - 1) / 256);

      this->command(0x4e);
      this->data(x / 8);

      this->command(0x4f);
      this->data(y % 256);
      this->data(y / 256);
    }

    void GDEH029A1::write_lut_(const uint8_t *lut, uint8_t n)
    {
      this->command(0x32);
      for (uint8_t i = 0; i < n; i++)
        this->data(lut[i]);
    }

    void GDEH029A1::deep_sleep()
    {
      if (hibernating_)
        return;

      // power off
      this->command(0x22);
      this->data(0xc3);
      this->command(0x20);

      if (this->reset_pin_ != nullptr)
      {
        this->command(0x10); // deep sleep mode
        this->data(0x1);     // enter deep sleep
        hibernating_ = true;
      }
    }

    void GDEH029A1::dump_config()
    {
      LOG_DISPLAY("", "Good Display e-Paper", this)
      ESP_LOGCONFIG(TAG, "  Model: GDEH029A1");
      LOG_PIN("  CS Pin: ", this->cs_)
      LOG_PIN("  Reset Pin: ", this->reset_pin_)
      LOG_PIN("  DC Pin: ", this->dc_pin_)
      LOG_PIN("  Busy Pin: ", this->busy_pin_)
      LOG_UPDATE_INTERVAL(this)
    }

  } // namespace waveshare_epaper
} // namespace esphome