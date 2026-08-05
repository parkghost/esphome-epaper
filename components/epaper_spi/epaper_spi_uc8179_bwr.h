#pragma once

#include "epaper_spi_uc8179_base.h"

namespace esphome::epaper_spi {

/**
 * UC8179 3-color (black/white/red) e-paper displays (e.g. P750057-MF1-A,
 * 800x480 -- not a Good Display panel, though it matches their GDEY075Z08).
 *
 * Buffer layout: 1 bit per pixel, two separate planes
 * - first half:  Black/White plane (bit 1 = black, 0 = white)
 * - second half: Red plane         (bit 1 = red,   0 = white)
 * - total: width * height / 4 bytes (2 * width * height / 8)
 *
 * On the wire the B/W plane goes to DTM1 (0x10) inverted (panel wants 0=black,
 * 1=white), the red plane goes to DTM2 (0x13) as-is (1=red).
 *
 * UC8179 3-color panels have no differential partial mode: `full_update_every`
 * selects between a full refresh and a faster whole-screen redraw (both re-send
 * the whole frame), the latter via the fast waveform (0xE5).
 */
class EPaperUC8179BWR : public EPaperUC8179Base {
 public:
  EPaperUC8179BWR(const char *name, uint16_t width, uint16_t height, const uint8_t *init_sequence,
                  size_t init_sequence_length)
      : EPaperUC8179Base(name, width, height, init_sequence, init_sequence_length, DISPLAY_TYPE_COLOR) {
    this->buffer_length_ = this->row_width_ * height * 2;  // two 1-bpp planes
  }

  void fill(Color color) override;
  // COLOR_ON maps to white here (3-color true color), unlike the mono base.
  void clear() override { this->fill(COLOR_ON); }

 protected:
  void draw_pixel_at(int x, int y, Color color) override;
  bool initialise(bool fast) override;
  bool transfer_data() override;
};

}  // namespace esphome::epaper_spi
