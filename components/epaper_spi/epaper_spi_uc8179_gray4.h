#pragma once

#include "epaper_spi_uc8179_base.h"

namespace esphome::epaper_spi {

/**
 * UC8179 e-paper displays driven with 4 gray levels (e.g. Good Display
 * GDEY075T7, 800x480).
 *
 * The controller only has two 1-bpp planes (0x10 "old" / 0x13 "new"). Gray
 * comes from treating a pixel's (old, new) bit pair as a 2-bit level, with a
 * waveform that drives each combination to a different grayness. On the wire a
 * set bit is ink (DDX = 00 in register 0x50, same as the mono driver's
 * full-refresh path), so the transitions are:
 *
 *   white       old=0 new=0 -> LUTWW (0x21)
 *   light gray  old=0 new=1 -> LUTWK (0x23)
 *   dark gray   old=1 new=0 -> LUTKW (0x22)
 *   black       old=1 new=1 -> LUTKK (0x24)
 *
 * Buffer layout: 1 bit per pixel, two separate planes
 * - first half:  MSB plane of the level
 * - second half: LSB plane of the level
 * - total: width * height / 4 bytes (2 * width * height / 8)
 *
 * The buffer holds the level itself (bit set = brighter), so each plane is
 * inverted as it is written out.
 *
 * There is no partial/differential mode here -- the "old" plane carries gray
 * data rather than the previous frame -- so every update is a full refresh
 * (~2.5s on GDEY075T7) and `full_update_every` has no effect.
 */
class EPaperUC8179Gray4Base : public EPaperUC8179Base {
 public:
  EPaperUC8179Gray4Base(const char *name, uint16_t width, uint16_t height, const uint8_t *init_sequence,
                        size_t init_sequence_length)
      : EPaperUC8179Base(name, width, height, init_sequence, init_sequence_length, DISPLAY_TYPE_GRAYSCALE) {
    this->buffer_length_ = this->row_width_ * height * 2;  // two 1-bpp planes
  }

  // Quantise to the four levels, following the it8951 driver: Rec.601 luma
  // (0.299/0.587/0.114, scaled by 256), then rounded to the nearest level.
  //
  // Rec.601 is the standard for turning SDR images into gray and spreads
  // saturated colours across the mid-range, where Rec.709 would crowd them
  // against white and black -- with only four levels there is no room for that.
  // Rounding to the nearest level rather than splitting the range into four
  // equal bands matters just as much: the levels sit at luma 0 / 85 / 170 / 255
  // and measure 0 / 35 / 65 / 100 on paper, so nearest-level is what actually
  // reproduces a tone.
  //
  // Drawing stays WYSIWYG -- a bright colour ends up as bright paper, so ink
  // must be drawn with a dark colour. No special case is needed for gray
  // colours: with r == g == b the weights sum back to exactly that value.
  static uint8_t color_to_gray4(Color color) {
    const uint8_t luma = (uint8_t) ((77u * color.r + 150u * color.g + 29u * color.b + 128u) >> 8);
    return (uint8_t) ((luma + 42u) / 85u);  // 0..255 -> 0..3, nearest
  }

  void fill(Color color) override;

 protected:
  void setup() override;
  void draw_pixel_at(int x, int y, Color color) override;
  bool transfer_data() override;

  // Which plane goes to which RAM. The two waveforms assign the middle levels
  // to opposite planes, so each subclass fixes this.
  bool swap_planes_{};
};

/**
 * 4 gray levels from the waveform built into OTP, selected by forcing the
 * temperature (0xE5 = 0x5F) as in Good Display's current A32-GDEY075T7-FP4G
 * demo. Only panels made from roughly late 2024 on carry that waveform, but
 * where it exists it spaces the four levels far more evenly than the register
 * LUTs do, so it is the default.
 */
class EPaperUC8179Gray4 : public EPaperUC8179Gray4Base {
 public:
  EPaperUC8179Gray4(const char *name, uint16_t width, uint16_t height, const uint8_t *init_sequence,
                    size_t init_sequence_length)
      : EPaperUC8179Gray4Base(name, width, height, init_sequence, init_sequence_length) {
    this->swap_planes_ = true;
  }

 protected:
  void power_on() override;
};

/**
 * 4 gray levels from waveform LUTs downloaded into registers 0x20..0x25 (they
 * are part of the model's init sequence), as in Good Display's older
 * GDEY075T7_4Gray demo. Works on every panel batch, but on current panels the
 * light gray it produces sits very close to white.
 */
class EPaperUC8179Gray4Lut : public EPaperUC8179Gray4Base {
 public:
  EPaperUC8179Gray4Lut(const char *name, uint16_t width, uint16_t height, const uint8_t *init_sequence,
                       size_t init_sequence_length)
      : EPaperUC8179Gray4Base(name, width, height, init_sequence, init_sequence_length) {
    this->swap_planes_ = false;
  }

 protected:
  void power_on() override;
};

}  // namespace esphome::epaper_spi
