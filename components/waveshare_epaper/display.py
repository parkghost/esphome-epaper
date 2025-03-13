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
GDEW029T5D = waveshare_epaper_ns.class_(
    "GDEW029T5D", WaveshareEPaper
)
GDEY029Z95 = waveshare_epaper_ns.class_(
    "GDEY029Z95", WaveshareEPaper
)
DEPG0420 = waveshare_epaper_ns.class_(
    "DEPG0420", WaveshareEPaper
)
GDEW042Z15 = waveshare_epaper_ns.class_(
    "GDEW042Z15", WaveshareEPaper
)
GDEY075T7 = waveshare_epaper_ns.class_(
    "GDEY075T7", WaveshareEPaper
)
P750057MF1A = waveshare_epaper_ns.class_(
    "P750057MF1A", WaveshareEPaper
)


MODELS = {
    "e0213a09": ("c", E0213A09),
    "gdeh029a1": ("c", GDEH029A1),
    "gdem029t94": ("c", GDEM029T94),
    "gdew029t5d": ("c", GDEW029T5D),
    "gdey029z95": ("c", GDEY029Z95),
    "depg0420": ("c", DEPG0420),
    "gdew042z15": ("b", GDEW042Z15),
    "gdey075t7": ("c", GDEY075T7),
    "p750057-mf1-a": ("c", P750057MF1A),
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

CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(WaveshareEPaperBase),
            cv.Required(CONF_DC_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_MODEL): cv.one_of(*MODELS, lower=True),
            cv.Required(CONF_RESET_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_BUSY_PIN): pins.gpio_input_pin_schema,
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
    cv.has_at_most_one_key(CONF_PAGES, CONF_LAMBDA),
)

FINAL_VALIDATE_SCHEMA = spi.final_validate_device_schema(
    "waveshare_epaper", require_miso=False, require_mosi=True
)

async def to_code(config):
    model_type, model = MODELS[config[CONF_MODEL]]
    if model_type in ("b", "c"):
        rhs = model.new()
        var = cg.Pvariable(config[CONF_ID], rhs, model)
    else:
        raise NotImplementedError()

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