from esphome import core, pins
import esphome.codegen as cg
from esphome.components import display, spi
import esphome.config_validation as cv
from esphome.const import (
    CONF_BUSY_PIN,
    CONF_DC_PIN,
    CONF_FULL_UPDATE_EVERY,
    CONF_ID,
    CONF_LAMBDA,
    CONF_MODEL,
    CONF_PAGES,
    CONF_RESET_DURATION,
    CONF_RESET_PIN,
)

DEPENDENCIES = ["spi"]

waveshare_epaper_ns = cg.esphome_ns.namespace("waveshare_epaper")
WaveshareEPaperBase = waveshare_epaper_ns.class_(
    "WaveshareEPaperBase", cg.PollingComponent, spi.SPIDevice, display.DisplayBuffer
)
WaveshareEPaper = waveshare_epaper_ns.class_(
    "WaveshareEPaper", WaveshareEPaperBase
)
WaveshareEPaperBWR = waveshare_epaper_ns.class_(
    "WaveshareEPaperBWR", WaveshareEPaperBase
)
E0213A09 = waveshare_epaper_ns.class_(
    "E0213A09", WaveshareEPaper
)
GDEH029A1 = waveshare_epaper_ns.class_(
    "GDEH029A1", WaveshareEPaper
)
GDEM029T94 = waveshare_epaper_ns.class_(
    "GDEM029T94", WaveshareEPaper
)

MODELS = {
    "e0213a09": ("c", E0213A09),  # HINK-E0213A09  104x212, SSD1675A (IL3897)
    "gdeh029a1": ("c", GDEH029A1), # GDEH029A1  128x296, SSD1608 (IL3820), (E029A01-FPC-A1 SYX1553)
    "gdem029t94": ("c", GDEM029T94), # GDEM029T94  128x296, SSD1680, (FPC-7519 rev.b), Waveshare 2.9" V2 variant
}

def validate_full_update_every_only_types_ac(value):
    if CONF_FULL_UPDATE_EVERY not in value:
        return value
    if MODELS[value[CONF_MODEL]][0] == "b":
        full_models = []
        for key, val in sorted(MODELS.items()):
            if val[0] != "b":
                full_models.append(key)
        raise cv.Invalid(
            "The 'full_update_every' option is only available for models "
            + ", ".join(full_models)
        )
    return value


def validate_reset_pin_required(config):
    if CONF_RESET_PIN not in config:
        raise cv.Invalid(
            f"'{CONF_RESET_PIN}' is required for model {config[CONF_MODEL]}"
        )
    return config


CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(WaveshareEPaperBase),
            cv.Required(CONF_DC_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_MODEL): cv.one_of(*MODELS, lower=True),
            cv.Optional(CONF_RESET_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_BUSY_PIN): pins.gpio_input_pin_schema,
            cv.Optional(CONF_FULL_UPDATE_EVERY): cv.int_range(min=1, max=4294967295),
            cv.Optional(CONF_RESET_DURATION): cv.All(
                cv.positive_time_period_milliseconds,
                cv.Range(max=core.TimePeriod(milliseconds=500)),
            ),
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(spi.spi_device_schema()),
    validate_full_update_every_only_types_ac,
    validate_reset_pin_required,
    cv.has_at_most_one_key(CONF_PAGES, CONF_LAMBDA),
)

FINAL_VALIDATE_SCHEMA = spi.final_validate_device_schema(
    "waveshare_epaper", require_miso=False, require_mosi=True
)

async def to_code(config):
    _, model = MODELS[config[CONF_MODEL]]
    rhs = model.new()
    var = cg.Pvariable(config[CONF_ID], rhs, model)

    await display.register_display(var, config)
    await spi.register_spi_device(var, config)

    dc = await cg.gpio_pin_expression(config[CONF_DC_PIN])
    cg.add(var.set_dc_pin(dc))

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(display.DisplayRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))
    if CONF_RESET_PIN in config:
        reset = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
        cg.add(var.set_reset_pin(reset))
    if CONF_BUSY_PIN in config:
        reset = await cg.gpio_pin_expression(config[CONF_BUSY_PIN])
        cg.add(var.set_busy_pin(reset))
    if CONF_FULL_UPDATE_EVERY in config:
        cg.add(var.set_full_update_every(config[CONF_FULL_UPDATE_EVERY]))
    if CONF_RESET_DURATION in config:
        cg.add(var.set_reset_duration(config[CONF_RESET_DURATION]))
