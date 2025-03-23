#include "waveshare_epaper.h"

namespace esphome {
namespace waveshare_epaper {

class GDEY075T7 : public WaveshareEPaper {
 public:
  static const char *const TAG;

  static const uint16_t WIDTH = 800;

  static const uint16_t HEIGHT = 480;

  static const uint16_t IDLE_TIMEOUT = 6000;

  void initialize() override;

  void dump_config() override;

  void display() override;

  void deep_sleep() override;

  void set_full_update_every(uint32_t full_update_every);

  void full_refresh();

 protected:
  int get_width_internal() override;

  int get_height_internal() override;

  uint32_t idle_timeout_() override;

  bool is_busy_pin_inverted_() override { return true; }

  void init_display_();

  void write_buffer_(RefreshMode mode);

  void clear_();

  void reset_();

  uint8_t oldData_[WIDTH * HEIGHT / 8];

  uint32_t at_update_{0}, full_update_every_{30};
};

}  // namespace waveshare_epaper
}  // namespace esphome