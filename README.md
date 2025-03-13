# ESPHome E-Paper Display Integration

This ESPHome component provides support for various e-paper displays.

## Supported Displays

The following e-paper display models are supported by this component:

| Model         | Size  | Colors | Resolution | Partial Refresh | Fast Refresh | Tested                                        | Useful for                 | Driver IC         |
|---------------|-------|--------|------------|-----------------|--------------|-----------------------------------------------|----------------------------|-------------------|
| e0213a09      | 2.13" | B/W    | 212x104    | Y               | N            | E213A09N(HINK-E0213A07-A1)                    |                            | SSD1675A(IL3897)  |
| gdeh029a1     | 2.9"  | B/W    | 296x128    | Y               | N            | E029A01(E029A01-FPCA-V2.0) / (EO29A01-FPC-A1) | Good Display GDEH029A1     | SSD1608(IL3820)   |
| gdem029t94    | 2.9"  | B/W    | 296x128    | Y               | N            | Waveshare 2.9" SKU-12563 (FPC-7519rev.b)      | Good Display GDEM029T94    | SSD1680           |
| gdew029t5d    | 2.9"  | B/W    | 296x128    | Y               | N            | WF0290T5(WFT0290CZ10 LW) / (WFT0290CZ10 LP)   | Good Display GDEW029T5D    | UC8151D           |
| gdey029z95    | 2.9"  | B/W/R  | 296x128    | N               | Y            | (FPC-A005 20.06.15 TRX)                       | Good Display GDEY029Z95    | SSD1680           |
| depg0420      | 4.2"  | B/W/R  | 400x300    | N               | Y            | DEPG0420(FPC-190)                             | Good Display GDEY042Z98    | SSD1683           |
| gdew042z15    | 4.2"  | B/W/R  | 400x300    | N               | N            | WF0420T80CZ15(WFT0420CZ15 LW)                 | Good Display GDEW042Z15    | UC8176(IL0398)    |
| gdey075t7     | 7.5"  | B/W    | 800x480    | Y               | Y            | GDEY075T7(FPC-C001 21.08.30 HB)               | Good Display GDEY075T7     | UC8179            |
| p750057-mf1-a | 7.5"  | B/W/R  | 800x480    | N               | Y            | (P750057-MF1-A)                               | Good Display GDEY075Z08    | UC8179            |

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