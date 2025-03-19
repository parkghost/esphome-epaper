#include "waveshare_epaper.h"

namespace esphome {
namespace waveshare_epaper {

class GDEY029Z95 : public WaveshareEPaperBWR {
 public:
  static const char *const TAG;

  static const uint16_t WIDTH = 128;

  static const uint16_t HEIGHT = 296;

  static const uint16_t IDLE_TIMEOUT = 16000;

  void initialize() override;

  void dump_config() override;

  void display() override;

  void deep_sleep() override;

  void set_full_update_every(uint32_t full_update_every);

  void full_refresh();

 protected:
  int get_width_internal() override;

  int get_width_controller() override;

  int get_height_internal() override;

  uint32_t idle_timeout_() override;

  void init_display_();

  void reset_();

 private:
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