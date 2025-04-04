#include "waveshare_epaper.h"

namespace esphome {
namespace waveshare_epaper {

class E0213A09 : public WaveshareEPaper {
 public:
  static const char *const TAG;

  static const uint16_t WIDTH = 104;

  static const uint16_t HEIGHT = 212;

  static const uint16_t IDLE_TIMEOUT = 2500;

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

  void init_display_();

  void reset_();

  void setPartialRamArea_(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

  static const uint8_t LUT_DATA_FULL[], LUT_DATA_PART[];

#ifdef USE_ESP32
  static uint32_t at_update_;
#else
  uint32_t at_update_{0};
#endif

  uint32_t full_update_every_{30};

  bool initial_{false}, hibernating_{false};
};

}  // namespace waveshare_epaper
}  // namespace esphome