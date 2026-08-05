"""UC8179 e-paper controller.

Panels:
- GDEY075T7 (Good Display):           800x480 monochrome
- GDEY075T7-GRAY4 (Good Display):     the same panel with 4 gray levels, waveform
                                      from OTP (panels made from ~late 2024 on)
- GDEY075T7-GRAY4-LUT (Good Display): 4 gray levels with the waveform downloaded
                                      into registers; works on any batch, but on
                                      current panels its light gray sits close
                                      to white
- P750057-MF1-A:                      800x480 black/white/red (not a Good Display
                                      panel; equivalent to their GDEY075Z08,
                                      whose demo code the driver was ported from)

UC8179 BUSY is active-LOW; the driver handles that (busy_invert_), so no
`inverted:` is needed on the busy pin in YAML.
"""

from . import RequiredPinsModel


class UC8179Mono(RequiredPinsModel):
    def __init__(self, name, **defaults):
        super().__init__(name, "EPaperUC8179", **defaults)

    def get_init_sequence(self, config):
        width, height = self.get_dimensions(config)
        return (
            (0x00, 0x1F),  # PANEL SETTING: KW (black/white) mode
            (
                0x61,
                (width >> 8) & 0xFF,
                width & 0xFF,
                (height >> 8) & 0xFF,
                height & 0xFF,
            ),  # TRES (resolution)
            (0x15, 0x00),  # DUAL SPI (disabled)
            (0x50, 0x10, 0x07),  # VCOM AND DATA INTERVAL SETTING
            (0x60, 0x22),  # TCON SETTING
        )


class UC8179BWR(RequiredPinsModel):
    def __init__(self, name, **defaults):
        super().__init__(name, "EPaperUC8179BWR", **defaults)

    def get_init_sequence(self, config):
        width, height = self.get_dimensions(config)
        return (
            (0x00, 0x0F),  # PANEL SETTING: KWR (black/white/red) mode
            (
                0x61,
                (width >> 8) & 0xFF,
                width & 0xFF,
                (height >> 8) & 0xFF,
                height & 0xFF,
            ),  # TRES (resolution)
            (0x15, 0x00),  # DUAL SPI (disabled)
            (0x50, 0x11, 0x07),  # VCOM AND DATA INTERVAL SETTING
            (0x60, 0x22),  # TCON SETTING
        )


class UC8179Gray4(RequiredPinsModel):
    """UC8179 panel driven with 4 gray levels from the OTP waveform.

    Forcing the temperature (0xE5 = 0x5F) selects a 4-gray waveform held in OTP,
    so no LUTs are downloaded. Panels made from roughly late 2024 on carry it;
    older ones need the register-LUT model instead.
    """

    def __init__(self, name, **defaults):
        super().__init__(name, "EPaperUC8179Gray4", **defaults)

    def get_init_sequence(self, config):
        width, height = self.get_dimensions(config)
        return (
            (0x00, 0x1F),  # PANEL SETTING: KW mode, LUT from OTP
            (
                0x61,
                (width >> 8) & 0xFF,
                width & 0xFF,
                (height >> 8) & 0xFF,
                height & 0xFF,
            ),  # TRES (resolution)
            (0x15, 0x00),  # DUAL SPI (disabled)
            (0x50, 0x10, 0x07),  # VCOM AND DATA INTERVAL: DDX=00, so 1 = black
            (0x60, 0x22),  # TCON SETTING
            (0xE0, 0x02),  # CASCADE SETTING: TSFIX, use the forced temperature
            (0xE5, 0x5F),  # FORCE TEMPERATURE: 0x5A = 1.5s fast, 0x5F = 4 gray
        )


class UC8179Gray4Lut(RequiredPinsModel):
    """UC8179 panel driven with 4 gray levels from register LUTs.

    The waveform is downloaded into registers 0x20..0x25 instead of coming from
    OTP, which works on every panel batch. The LUTs are a property of the panel,
    so each registration passes its own tables.
    """

    def __init__(self, name, lut_vcom, lut_ww, lut_bw, lut_wb, lut_bb, **defaults):
        super().__init__(name, "EPaperUC8179Gray4Lut", **defaults)
        self.lut_vcom = lut_vcom
        self.lut_ww = lut_ww
        self.lut_bw = lut_bw
        self.lut_wb = lut_wb
        self.lut_bb = lut_bb

    def get_init_sequence(self, config):
        width, height = self.get_dimensions(config)
        return (
            # PANEL SETTING: REG=1 (LUT from registers) + KW mode. D7 is unused
            # per the datasheet, so the vendor's 0xBF and GxEPD2's 0x3F are the
            # same setting.
            (0x00, 0xBF),
            (0x30, 0x06),  # PLL: FRS=0110 = 50Hz -- LUT phase lengths are in frames
            (
                0x61,
                (width >> 8) & 0xFF,
                width & 0xFF,
                (height >> 8) & 0xFF,
                height & 0xFF,
            ),  # TRES (resolution)
            (0x15, 0x00),  # DUAL SPI (disabled)
            (0x60, 0x22),  # TCON SETTING
            (0x82, 0x12),  # VCOM_DC SETTING: VDCS = -1.00V
            # VCOM AND DATA INTERVAL: BDV=01 (border from LUTKW), N2OCP=0,
            # DDX=00. DDX=00 is what makes a set data bit black, hence the
            # inversion in the driver's transfer_data().
            (0x50, 0x10, 0x07),
            # Waveform LUTs. In KW mode every LUT uses 7 groups of 6 bytes = 42;
            # LUTC (0x20) has room for 10 groups, and the vendor demo writes 44
            # bytes, so the table below carries two trailing zeroes that land in
            # the unused 8th group. The border LUT (0x25) reuses the W2W table,
            # per the vendor demo -- inert here, since BDV=01 selects LUTKW.
            (0x20, *self.lut_vcom),
            (0x21, *self.lut_ww),
            (0x22, *self.lut_bw),
            (0x23, *self.lut_wb),
            (0x24, *self.lut_bb),
            (0x25, *self.lut_ww),
        )


UC8179Mono(
    "GDEY075T7", width=800, height=480, data_rate="4MHz", minimum_update_interval="10s"
)

# Same floor as the mono GDEY075T7: it is the same panel doing a full refresh,
# ~2.5s of which the datasheet's 3.5s full update time is the upper bound.
UC8179Gray4(
    "GDEY075T7-GRAY4",
    width=800,
    height=480,
    data_rate="4MHz",
    minimum_update_interval="10s",
)

# GDEY075T7 4-gray waveform LUTs, from Good Display's GDEY075T7_4Gray demo.
# Each group is 6 bytes: a level-selection byte (four 2-bit phases, 01 = VDH,
# 10 = VDL), four frame counts, and a repeat count.
#
# This is the older of the vendor's two 4-gray approaches, kept for panel
# batches whose OTP has no 4-gray waveform (see UC8179Gray4 for that one).
# fmt: off
_GDEY075T7_GRAY4_LUT_VCOM = [
    0x00, 0x0A, 0x00, 0x00, 0x00, 0x01,
    0x60, 0x14, 0x14, 0x00, 0x00, 0x01,
    0x00, 0x14, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x13, 0x0A, 0x01, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00,
]
# white: old=0 new=0
_GDEY075T7_GRAY4_LUT_WW = [
    0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
    0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
    0x10, 0x14, 0x0A, 0x00, 0x00, 0x01,
    0xA0, 0x13, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
]
# dark gray: old=1 new=0
_GDEY075T7_GRAY4_LUT_BW = [
    0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
    0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
    0x00, 0x14, 0x0A, 0x00, 0x00, 0x01,
    0x99, 0x0C, 0x01, 0x03, 0x04, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
]
# light gray: old=0 new=1
_GDEY075T7_GRAY4_LUT_WB = [
    0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
    0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
    0x00, 0x14, 0x0A, 0x00, 0x00, 0x01,
    0x99, 0x0B, 0x04, 0x04, 0x01, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
]
# black: old=1 new=1
_GDEY075T7_GRAY4_LUT_BB = [
    0x80, 0x0A, 0x00, 0x00, 0x00, 0x01,
    0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
    0x20, 0x14, 0x0A, 0x00, 0x00, 0x01,
    0x50, 0x13, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
]
# fmt: on

UC8179Gray4Lut(
    "GDEY075T7-GRAY4-LUT",
    lut_vcom=_GDEY075T7_GRAY4_LUT_VCOM,
    lut_ww=_GDEY075T7_GRAY4_LUT_WW,
    lut_bw=_GDEY075T7_GRAY4_LUT_BW,
    lut_wb=_GDEY075T7_GRAY4_LUT_WB,
    lut_bb=_GDEY075T7_GRAY4_LUT_BB,
    width=800,
    height=480,
    data_rate="4MHz",
    minimum_update_interval="10s",
)

UC8179BWR(
    "P750057-MF1-A",
    width=800,
    height=480,
    data_rate="4MHz",
    minimum_update_interval="30s",
)
