# SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
#
# SPDX-License-Identifier: MIT

"""
es8311.py - MicroPython port of ES8311.h / ES8311.cpp

Driver for the ES8311 audio codec (DAC + ADC).
"""

from i2c_bus import I2CBus
import time

ES8311_I2C_ADDR = 0x18

# Register addresses
ES8311_RESET_REG00       = 0x00
ES8311_CLK_MANAGER_REG01 = 0x01
ES8311_CLK_MANAGER_REG02 = 0x02
ES8311_CLK_MANAGER_REG03 = 0x03
ES8311_CLK_MANAGER_REG04 = 0x04
ES8311_CLK_MANAGER_REG05 = 0x05
ES8311_CLK_MANAGER_REG06 = 0x06
ES8311_CLK_MANAGER_REG07 = 0x07
ES8311_CLK_MANAGER_REG08 = 0x08
ES8311_SDPIN_REG09       = 0x09
ES8311_SDPOUT_REG0A      = 0x0A
ES8311_SYSTEM_REG0D      = 0x0D
ES8311_SYSTEM_REG0E      = 0x0E
ES8311_SYSTEM_REG12      = 0x12
ES8311_SYSTEM_REG13      = 0x13
ES8311_ADC_REG16         = 0x16
ES8311_ADC_REG1C         = 0x1C
ES8311_DAC_REG31         = 0x31
ES8311_DAC_REG32         = 0x32
ES8311_DAC_REG34         = 0x34
ES8311_DAC_REG35         = 0x35
ES8311_DAC_REG37         = 0x37

# Microphone gain levels
ES8311_MIC_GAIN_0DB  = 0
ES8311_MIC_GAIN_6DB  = 1
ES8311_MIC_GAIN_12DB = 2
ES8311_MIC_GAIN_18DB = 3
ES8311_MIC_GAIN_24DB = 4
ES8311_MIC_GAIN_30DB = 5
ES8311_MIC_GAIN_36DB = 6
ES8311_MIC_GAIN_42DB = 7

# Clock coefficient table:
# (mclk, rate, pre_div, pre_multi, adc_div, dac_div, fs_mode,
#  lrck_h, lrck_l, bclk_div, adc_osr, dac_osr)
_COEFF_TABLE = (
    # 8k
    (12288000, 8000,  6, 0, 1, 1, 0, 0x00, 0xFF, 4,  0x10, 0x20),
    (18432000, 8000,  6, 0, 1, 1, 0, 0x00, 0xFF, 6,  0x10, 0x20),
    (16384000, 8000,  8, 0, 1, 1, 0, 0x00, 0xFF, 8,  0x10, 0x20),
    (8192000,  8000,  4, 0, 1, 1, 0, 0x00, 0xFF, 4,  0x10, 0x20),
    (6144000,  8000,  3, 0, 1, 1, 0, 0x00, 0xFF, 3,  0x10, 0x20),
    (4096000,  8000,  2, 0, 1, 1, 0, 0x00, 0xFF, 2,  0x10, 0x20),
    (3072000,  8000,  1, 0, 1, 1, 0, 0x00, 0xFF, 4,  0x10, 0x20),
    (2048000,  8000,  1, 0, 1, 1, 0, 0x00, 0xFF, 2,  0x10, 0x20),
    (1536000,  8000,  1, 0, 1, 1, 1, 0x00, 0x7F, 2,  0x10, 0x20),
    (1024000,  8000,  1, 0, 1, 1, 1, 0x00, 0x7F, 1,  0x10, 0x20),
    # 11.025k
    (11289600, 11025, 4, 0, 1, 1, 0, 0x00, 0xFF, 4,  0x10, 0x20),
    (5644800,  11025, 2, 0, 1, 1, 0, 0x00, 0xFF, 2,  0x10, 0x20),
    (2822400,  11025, 1, 0, 1, 1, 0, 0x00, 0xFF, 2,  0x10, 0x20),
    (1411200,  11025, 1, 0, 1, 1, 1, 0x00, 0x7F, 1,  0x10, 0x20),
    # 12k
    (12288000, 12000, 4, 0, 1, 1, 0, 0x00, 0xFF, 4,  0x10, 0x20),
    (6144000,  12000, 2, 0, 1, 1, 0, 0x00, 0xFF, 2,  0x10, 0x20),
    (3072000,  12000, 1, 0, 1, 1, 0, 0x00, 0xFF, 2,  0x10, 0x20),
    (1536000,  12000, 1, 0, 1, 1, 1, 0x00, 0x7F, 1,  0x10, 0x20),
    # 16k
    (12288000, 16000, 3, 0, 1, 1, 0, 0x00, 0xFF, 3,  0x10, 0x20),
    (18432000, 16000, 3, 0, 1, 1, 0, 0x00, 0xFF, 6,  0x10, 0x20),
    (16384000, 16000, 4, 0, 1, 1, 0, 0x00, 0xFF, 4,  0x10, 0x20),
    (8192000,  16000, 2, 0, 1, 1, 0, 0x00, 0xFF, 2,  0x10, 0x20),
    (6144000,  16000, 1, 0, 1, 1, 0, 0x00, 0xFF, 3,  0x10, 0x20),
    (4096000,  16000, 1, 0, 1, 1, 0, 0x00, 0xFF, 2,  0x10, 0x20),
    (3072000,  16000, 1, 0, 1, 1, 1, 0x00, 0x7F, 2,  0x10, 0x20),
    (2048000,  16000, 1, 0, 1, 1, 1, 0x00, 0x7F, 1,  0x10, 0x20),
    # 22.05k
    (11289600, 22050, 2, 0, 1, 1, 0, 0x00, 0xFF, 2,  0x10, 0x20),
    (5644800,  22050, 1, 0, 1, 1, 0, 0x00, 0xFF, 1,  0x10, 0x20),
    (2822400,  22050, 1, 0, 1, 1, 1, 0x00, 0x7F, 1,  0x10, 0x20),
    # 24k
    (12288000, 24000, 2, 0, 1, 1, 0, 0x00, 0xFF, 2,  0x10, 0x20),
    (6144000,  24000, 1, 0, 1, 1, 0, 0x00, 0xFF, 1,  0x10, 0x20),
    (3072000,  24000, 1, 0, 1, 1, 1, 0x00, 0x7F, 1,  0x10, 0x20),
    # 32k
    (12288000, 32000, 1, 0, 1, 1, 0, 0x00, 0xFF, 3,  0x10, 0x20),
    (18432000, 32000, 1, 0, 1, 1, 0, 0x00, 0xFF, 6,  0x10, 0x20),
    (16384000, 32000, 1, 0, 2, 2, 0, 0x00, 0xFF, 4,  0x10, 0x20),
    (8192000,  32000, 1, 0, 1, 1, 0, 0x00, 0xFF, 2,  0x10, 0x20),
    (6144000,  32000, 1, 0, 1, 1, 1, 0x00, 0x7F, 3,  0x10, 0x20),
    (4096000,  32000, 1, 0, 1, 1, 1, 0x00, 0x7F, 2,  0x10, 0x20),
    (2048000,  32000, 1, 0, 1, 1, 1, 0x00, 0x7F, 1,  0x10, 0x20),
    # 44.1k (primary use case)
    (11289600, 44100, 1, 0, 1, 1, 0, 0x00, 0xFF, 1,  0x10, 0x20),
    (5644800,  44100, 1, 0, 1, 1, 1, 0x00, 0x7F, 1,  0x10, 0x20),
    # 48k
    (12288000, 48000, 1, 0, 1, 1, 0, 0x00, 0xFF, 1,  0x10, 0x20),
    (18432000, 48000, 1, 0, 1, 1, 0, 0x00, 0xFF, 3,  0x10, 0x20),
    (16384000, 48000, 1, 0, 2, 2, 0, 0x00, 0xFF, 4,  0x10, 0x20),
    (8192000,  48000, 1, 0, 1, 1, 1, 0x00, 0x7F, 1,  0x10, 0x20),
    (6144000,  48000, 1, 0, 1, 1, 1, 0x00, 0x7F, 3,  0x10, 0x20),
    (4096000,  48000, 1, 0, 1, 1, 1, 0x00, 0x7F, 2,  0x10, 0x20),
)


class ES8311:
    """Driver for ES8311 audio codec."""

    def __init__(self, bus: I2CBus, addr: int = ES8311_I2C_ADDR):
        self._bus = bus
        self._addr = addr

    # ------------------------------------------------------------------
    # Private helpers
    # ------------------------------------------------------------------

    def _write_reg(self, reg: int, val: int) -> bool:
        return self._bus.write_bytes(self._addr, reg, bytes([val]))

    def _read_reg(self, reg: int):
        data = self._bus.read_bytes(self._addr, reg, 1)
        return data[0] if data else None

    def _get_coeff(self, mclk: int, rate: int):
        for row in _COEFF_TABLE:
            if row[0] == mclk and row[1] == rate:
                return row
        return None

    def _sample_frequency_config(self, mclk_hz: int, sample_rate: int) -> bool:
        coeff = self._get_coeff(mclk_hz, sample_rate)
        if coeff is None:
            print("[ES8311] Cannot find coefficients: MCLK={} Rate={}".format(
                mclk_hz, sample_rate))
            return False

        (_, _, pre_div, pre_multi, adc_div, dac_div, fs_mode,
         lrck_h, lrck_l, bclk_div, adc_osr, dac_osr) = coeff

        # REG02: pre_div & pre_multi
        regv = self._read_reg(ES8311_CLK_MANAGER_REG02)
        if regv is None:
            regv = 0
        regv &= 0x07
        regv |= (pre_div - 1) << 5
        regv |= pre_multi << 3
        self._write_reg(ES8311_CLK_MANAGER_REG02, regv)

        # REG03: fs_mode & adc_osr
        self._write_reg(ES8311_CLK_MANAGER_REG03, (fs_mode << 6) | adc_osr)

        # REG04: dac_osr
        self._write_reg(ES8311_CLK_MANAGER_REG04, dac_osr)

        # REG05: adc_div & dac_div
        self._write_reg(ES8311_CLK_MANAGER_REG05,
                        ((adc_div - 1) << 4) | (dac_div - 1))

        # REG06: bclk_div (preserve polarity bit)
        regv = self._read_reg(ES8311_CLK_MANAGER_REG06)
        if regv is None:
            regv = 0
        regv &= 0xE0
        regv |= (bclk_div - 1) if bclk_div < 19 else bclk_div
        self._write_reg(ES8311_CLK_MANAGER_REG06, regv)

        # REG07: lrck_h (preserve upper 2 bits)
        regv = self._read_reg(ES8311_CLK_MANAGER_REG07)
        if regv is None:
            regv = 0
        regv &= 0xC0
        regv |= lrck_h
        self._write_reg(ES8311_CLK_MANAGER_REG07, regv)

        # REG08: lrck_l
        self._write_reg(ES8311_CLK_MANAGER_REG08, lrck_l)

        print("[ES8311] Frequency config OK: MCLK={} Rate={}".format(
            mclk_hz, sample_rate))
        return True

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    def begin(self, mclk_hz: int, sample_rate: int = 44100) -> bool:
        """Initialise ES8311 codec.

        :param mclk_hz:     External MCLK frequency in Hz.
        :param sample_rate: Target audio sample rate in Hz.
        :return: True on success.
        """
        # Reset
        self._write_reg(ES8311_RESET_REG00, 0x1F)
        time.sleep_ms(20)
        self._write_reg(ES8311_RESET_REG00, 0x00)
        self._write_reg(ES8311_RESET_REG00, 0x80)

        # Clock source: use MCLK pin, enable all clocks
        self._write_reg(ES8311_CLK_MANAGER_REG01, 0x3F)

        if not self._sample_frequency_config(mclk_hz, sample_rate):
            return False

        # Slave mode, 16-bit I2S
        reg00 = self._read_reg(ES8311_RESET_REG00)
        if reg00 is None:
            reg00 = 0
        reg00 &= ~(1 << 6)  # bit6=0 → Slave
        self._write_reg(ES8311_RESET_REG00, reg00)
        self._write_reg(ES8311_SDPIN_REG09, 0x0C)   # 16-bit I2S
        self._write_reg(ES8311_SDPOUT_REG0A, 0x0C)  # 16-bit I2S

        # Analogue power
        self._write_reg(ES8311_SYSTEM_REG0D, 0x01)
        self._write_reg(ES8311_SYSTEM_REG0E, 0x02)
        self._write_reg(ES8311_SYSTEM_REG12, 0x00)
        self._write_reg(ES8311_SYSTEM_REG13, 0x10)

        self._write_reg(ES8311_ADC_REG1C, 0x6A)
        self._write_reg(ES8311_DAC_REG37, 0x08)

        self.set_volume(40)

        print("[ES8311] Init OK")
        return True

    def set_volume(self, volume: int) -> bool:
        """Set DAC output volume (0–100).

        :param volume: Volume level 0–100.
        :return: True on success.
        """
        if volume > 100:
            volume = 100
        reg = 0 if volume == 0 else ((volume * 256) // 100) - 1
        self._write_reg(ES8311_DAC_REG34, 0x80)
        self._write_reg(ES8311_DAC_REG35, 0x90)
        return self._write_reg(ES8311_DAC_REG32, reg)

    def get_volume(self) -> int | None:
        """Get current DAC volume (0–100).

        :return: Volume value, or None on I2C failure.
        """
        reg = self._read_reg(ES8311_DAC_REG32)
        if reg is None:
            return None
        return 0 if reg == 0 else (reg * 100) // 256 + 1

    def mute(self, enable: bool) -> bool:
        """Enable or disable DAC mute.

        :param enable: True = mute, False = unmute.
        :return: True on success.
        """
        reg = self._read_reg(ES8311_DAC_REG31)
        if reg is None:
            return False
        if enable:
            reg |= (1 << 6) | (1 << 5)
        else:
            reg &= ~((1 << 6) | (1 << 5))
        return self._write_reg(ES8311_DAC_REG31, reg)

    def set_mic_gain(self, gain: int) -> bool:
        """Set microphone PGA gain.

        :param gain: Gain constant (ES8311_MIC_GAIN_0DB … ES8311_MIC_GAIN_42DB).
        :return: True on success.
        """
        return self._write_reg(ES8311_ADC_REG16, gain & 0xFF)
