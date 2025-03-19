#include "waveshare_epaper.h"

namespace esphome {
namespace waveshare_epaper {

class GDEW042Z15 : public WaveshareEPaperBWR {
 public:
  static const char *const TAG;

  static const uint16_t WIDTH = 400;

  static const uint16_t HEIGHT = 300;

  static const uint16_t IDLE_TIMEOUT = 16000;

  void initialize() override;

  void dump_config() override;

  void display() override;

  void deep_sleep() override;

  void full_refresh();

 protected:
  int get_width_internal() override;

  int get_width_controller() override;

  int get_height_internal() override;

  uint32_t idle_timeout_() override;

  bool is_busy_pin_inverted_() override { return true; }

  void init_display_();

  void reset_();

  uint32_t full_update_every_{30};

  bool initial_{false}, hibernating_{false};
};

}  // namespace waveshare_epaper
}  // namespace esphome