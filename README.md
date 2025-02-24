# ESPHome E-Paper Display Integration

This ESPHome component provides support for various e-paper displays.

## Supported Displays

The following e-paper display models are supported by this component:

| Model         | Size  | Colors | Resolution | Partial Refresh | Fast Refresh | Tested                            | Useful for                 |
|---------------|-------|--------|------------|-----------------|--------------|-----------------------------------|----------------------------|
| e0213a09      | 2.13" | B/W    | 104x212    | Y               | N            | HINK-E0213A07-A1                  |                            |
| gdeh029a1     | 2.9"  | B/W    | 128x296    | Y               | N            | E029A01-FPCA-V2.0                 | Good Display GDEH029A1     |
| gdem029t94    | 2.9"  | B/W    | 128x296    | Y               | N            | Waveshare 2.9" V2 (FPC-7519rev.b) | Good Display GDEM029T94    |
| gdew029t5d    | 2.9"  | B/W    | 128x296    | Y               | N            | WFT0290CZ10 LW                    | Good Display GDEW029T5D    |
| gdew042z15    | 4.2"  | B/W/R  | 400x300    | N               | N            | WFT0420CZ15 LW                    | Good Display GDEW042Z15    |
| p750057-mf1-a | 7.5"  | B/W/R  | 800x480    | N               | Y            | P750057-MF1-A                     | Good Display GDEY075Z08    |

## Usage
```yaml
external_components:
  - source: github://parkghost/epd
    components: [ waveshare_epaper ]

font:
  - file: "gfonts://Roboto"
    id: roboto_36
    size: 36

spi:
  clk_pin: GPIO4
  mosi_pin: GPIO6

display:
  - platform: waveshare_epaper
    cs_pin: GPIO0
    dc_pin:  GPIO1
    busy_pin: GPIO3
    reset_pin: GPIO2
    model: e0213a09
    rotation: 270
    update_interval: 24h
    full_update_every: 30
    id: my_display
    lambda: |-
      int width = it.get_width();
      int height = it.get_height();
      it.printf(width/2, height/2, id(roboto_36), TextAlign::CENTER, "Hello World!");
```

## Acknowledgements

- **[ESPHome Waveshare E-Paper Display](https://esphome.io/components/display/waveshare_epaper.html)**: The ESPHome waveshare_epaper component provides the base for integrating e-paper displays with ESPHome, enabling easy configuration and control of supported displays.

- **[GxEPD2](https://github.com/ZinggJM/GxEPD2)**: This library has been instrumental in driving e-paper displays. Its open-source implementation allows for smooth handling of various e-paper models.