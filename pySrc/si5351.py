# SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
#
# SPDX-License-Identifier: MIT

"""
si5351.py - MicroPython port of SI5351.h / SI5351.cpp

Driver for the SI5351 programmable clock generator.
Generates the MCLK required by the ES7210 and ES8311 codecs.

Supported MCLK outputs:
    2,048,000 Hz  → 8 kHz audio
    4,096,000 Hz  → 16 kHz audio
   11,289,600 Hz  → 44.1 kHz audio
   12,288,000 Hz  → 48 kHz audio


Chip: SI5351A-B-GTR
    Crytal only, Rev B, GT = 10-MSOP*
Crytal: 27 MHz

Output clock CLK1 is used for MCLK. CLK0 and CLK2 are powered down.
   
2.1. PLL Selection
If spread spectrum is not enabled, either of the two PLLs may be used as the source for any outputs of the
Si5351A. If both XTAL and CLKIN input options are used simultaneously (Si5351C only), then one PLL must be
reserved for use with CLKIN and one for use with the XTAL.
Note: PLLA must be used for any spread spectrum-enabled outputs. PLLB must be used for any VCXO outputs.

2.1.1. Selecting the Proper VCO Frequencies and Divide Ratios
The general criteria below may be used to set the VCO frequencies. This is a general model, and individual
applications may require some modification.
    1. Valid Multisynth divider ratios are 4, 6, 8, and any fractional value between 8 + 1/1,048,575 and 2048. This
    means that if any output is greater than 112.5 MHz (900 MHz/8), then this output frequency sets one of the
    VCO frequencies.
    2. For the frequencies where jitter is a concern make the output Multisynth divide ratio an integer. If possible,
    make both output and feedback Multisynth ratios integers.
    3. Once criteria 1 and 2 are satisfied, try to select as many integer output Multisynth ratios as possible.

2.2. Output Clock Pin Assignment (Optional)
Some customers may wish to have full control of output clock pin assignment. The guidelines below may be used
to assign clocks to pins on the device.
    1. Equal output frequencies should share a VDDO bank (e.g., CLK0&1, CLK2&3, etc.) whenever possible in
    order to ensure minimal jitter due to PCB crosstalk.
    2. Isolate unique output frequencies on individual VDDO bank if possible. For example, if the four outputs are
    25 MHz, 25 MHz, 27 MHz, and 74.25 MHz, then place them on CLK0, CLK1, CLK2, and CLK4,
    respectively.
    3. Otherwise, use all necessary CLK channels based on the required frequency plan.

"""

from i2c_bus import I2CBus
import time

SI5351_I2C_ADDR = 0x60
_XTAL_FREQ = 27_000_000


class SI5351:
    """Driver for SI5351 programmable clock generator."""

    def __init__(self, bus: I2CBus, addr: int = SI5351_I2C_ADDR):
        self._bus = bus
        self._addr = addr

    # ------------------------------------------------------------------
    # Private helpers
    # ------------------------------------------------------------------

    def _write_reg(self, reg: int, val: int) -> bool:
        return self._bus.write_bytes(self._addr, reg, bytes([val]))

    def _write_bulk(self, reg: int, data: bytes) -> bool:
        return self._bus.write_bytes(self._addr, reg, data)

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    def begin(self) -> bool:
        """Initialise SI5351: disable all outputs and power off CLKs."""
        self._write_reg(3, 0xFF)    # Disable all outputs
        time.sleep_ms(10)
        self._write_reg(16, 0x80)   # CLK0 power-down
        self._write_reg(17, 0x80)   # CLK1 power-down
        self._write_reg(18, 0x80)   # CLK2 power-down
        self._write_reg(183, 0xC0)  # Crystal load capacitance 10 pF
        print("[SI5351] begin OK")
        return True

    def set_mclk(self, freq: int) -> bool:
        """Set master-clock output frequency.

        :param freq: MCLK frequency in Hz (2_048_000 / 4_096_000 / 11_289_600 / 12_288_000).
        :return: True on success.
        """
        if freq == 11_289_600:   # 44100 × 256
            return self._set_pll(903_168_000, 80)
        if freq == 12_288_000:   # 48000 × 256
            return self._set_pll(884_736_000, 72)
        if freq == 4_096_000:    # 16000 × 256
            return self._set_pll(884_736_000, 216)
        if freq == 2_048_000:    # 8000 × 256
            return self._set_pll(884_736_000, 432)  # 2*216
        print("[SI5351] Unsupported MCLK: {} Hz".format(freq))
        return False

    def set_sample_rate(self, sample_rate: int) -> bool:
        """Configure MCLK for a given audio sample rate.

        :param sample_rate: 8000, 16000, 44100, or 48000 Hz.
        :return: True on success.
        """
        mapping = {8000: 2_048_000, 16000: 4_096_000, 44100: 11_289_600, 48000: 12_288_000}
        if sample_rate not in mapping:
            return False
        return self.set_mclk(mapping[sample_rate])

    # ------------------------------------------------------------------
    # Private PLL helpers
    # ------------------------------------------------------------------

    def _reset_pll(self):
        self._write_reg(177, 0xA0)
        time.sleep_ms(10)

    def _set_pll(self, pll_freq: int, ms_div: int) -> bool:
        """Configure PLLA and Multisynth1 (CLK1) for the desired frequency."""
        a = pll_freq // _XTAL_FREQ
        rest = pll_freq % _XTAL_FREQ
        c = 1_000_000
        b = (rest * c) // _XTAL_FREQ

        P1 = 128 * a + (128 * b) // c - 512
        P2 = 128 * b - c * ((128 * b) // c)
        P3 = c

        # Disable output while reconfiguring
        self._write_reg(3, 0xFF)

        # Write PLLA registers 26–33
        pll_buf = bytearray(8)
        pll_buf[0] = (P3 >> 8) & 0xFF
        pll_buf[1] = P3 & 0xFF
        pll_buf[2] = (P1 >> 16) & 0x03
        pll_buf[3] = (P1 >> 8) & 0xFF
        pll_buf[4] = P1 & 0xFF
        pll_buf[5] = ((P3 >> 12) & 0xF0) | ((P2 >> 16) & 0x0F)
        pll_buf[6] = (P2 >> 8) & 0xFF
        pll_buf[7] = P2 & 0xFF
        self._bus.write_bytes(self._addr, 26, bytes(pll_buf))

        # Multisynth1 (integer mode) → CLK1
        # Setting start at register 50.
        # R52 is output divider
        MS_P1 = 128 * ms_div - 512
        ms_buf = bytearray(8)
        ms_buf[0] = 0x00
        ms_buf[1] = 0x01   # P3 = 1
        ms_buf[2] = (MS_P1 >> 16) & 0x03
        ms_buf[3] = (MS_P1 >> 8) & 0xFF
        ms_buf[4] = MS_P1 & 0xFF
        ms_buf[5] = 0x00
        ms_buf[6] = 0x00
        ms_buf[7] = 0x00
        self._bus.write_bytes(self._addr, 50, bytes(ms_buf))

        # CLK1 control: use PLLA, 8 mA drive
        self._write_reg(17, 0x4F)
        # CLK0 / CLK2 powered down
        self._write_reg(16, 0x80)
        self._write_reg(18, 0x80)

        # Reset PLL
        self._write_reg(177, 0xA0)
        time.sleep_ms(10)

        # Enable only CLK1 (bit1=0 means CLK1 enabled; 0xFD = 1111 1101)
        self._write_reg(3, 0xFD)

        print("[SI5351] CLK1 = {} Hz (PLL={}, div={})".format(
            pll_freq // ms_div, pll_freq, ms_div))
        return True
