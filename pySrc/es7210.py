# SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
#
# SPDX-License-Identifier: MIT

"""
es7210.py - MicroPython port of ES7210.h / ES7210.cpp

Driver for the ES7210 4-channel ADC codec (microphone capture).
"""

from i2c_bus import I2CBus
import time

ES7210_I2C_ADDR = 0x40

# Register map
ES7210_RESET_REG00           = 0x00
ES7210_CLOCK_OFF_REG01       = 0x01
ES7210_MAINCLK_REG02         = 0x02
ES7210_MASTER_CLK_REG03      = 0x03
ES7210_LRCK_DIVH_REG04       = 0x04
ES7210_LRCK_DIVL_REG05       = 0x05
ES7210_POWER_DOWN_REG06      = 0x06
ES7210_OSR_REG07             = 0x07
ES7210_MODE_CONFIG_REG08     = 0x08
ES7210_SDP_INTERFACE1_REG11  = 0x11
ES7210_SDP_INTERFACE2_REG12  = 0x12
ES7210_ADC_AUTOMUTE_REG13    = 0x13
ES7210_ANALOG_REG40          = 0x40
ES7210_MIC12_BIAS_REG41      = 0x41
ES7210_MIC34_BIAS_REG42      = 0x42
ES7210_MIC1_GAIN_REG43       = 0x43
ES7210_MIC2_GAIN_REG44       = 0x44
ES7210_MIC3_GAIN_REG45       = 0x45
ES7210_MIC4_GAIN_REG46       = 0x46
ES7210_MIC1_POWER_REG47      = 0x47
ES7210_MIC2_POWER_REG48      = 0x48
ES7210_MIC3_POWER_REG49      = 0x49
ES7210_MIC4_POWER_REG4A      = 0x4A
ES7210_MIC12_POWER_REG4B     = 0x4B
ES7210_MIC34_POWER_REG4C     = 0x4C

# Gain levels (0 dB ~ 37.5 dB)
GAIN_0DB    = 0
GAIN_3DB    = 1
GAIN_6DB    = 2
GAIN_9DB    = 3
GAIN_12DB   = 4
GAIN_15DB   = 5
GAIN_18DB   = 6
GAIN_21DB   = 7
GAIN_24DB   = 8
GAIN_27DB   = 9
GAIN_30DB   = 10
GAIN_33DB   = 11
GAIN_34_5DB = 12
GAIN_36DB   = 13
GAIN_37_5DB = 14

# Microphone input mask
ES7210_INPUT_MIC1 = 0x01
ES7210_INPUT_MIC2 = 0x02
ES7210_INPUT_MIC3 = 0x04
ES7210_INPUT_MIC4 = 0x08


class ES7210:
    """Driver for ES7210 4-channel ADC codec."""

    def __init__(self, bus: I2CBus, addr: int = ES7210_I2C_ADDR):
        self._bus = bus
        self._addr = addr

    # ------------------------------------------------------------------
    # Private helpers
    # ------------------------------------------------------------------

    def _write_reg(self, reg: int, val: int) -> bool:
        ok = self._bus.write_bytes(self._addr, reg, bytes([val]))
        if not ok:
            print("[ES7210] Write FAILED: REG[0x{:02X}] = 0x{:02X}".format(reg, val))
        return ok

    def _read_reg(self, reg: int):
        data = self._bus.read_bytes(self._addr, reg, 1)
        return data[0] if data else None

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    def begin(self, mclk_hz: int, sample_rate: int = 44100,
              mic_mask: int = ES7210_INPUT_MIC1 | ES7210_INPUT_MIC3) -> bool:
        """Initialise ES7210.

        :param mclk_hz:    External MCLK frequency in Hz.
        :param sample_rate: Target sample rate in Hz.
        :param mic_mask:   Bitmask of enabled microphone channels.
        :return: True on success.
        """
        print("=== ES7210 Init Start ===")

        chip_id = self._read_reg(0x3D)
        print("[ES7210] Chip ID reg(0x3D) = 0x{:02X}".format(
            chip_id if chip_id is not None else 0xFF))

        # 1. Software reset
        self._write_reg(ES7210_RESET_REG00, 0xFF)
        time.sleep_ms(20)
        self._write_reg(ES7210_RESET_REG00, 0x32)
        time.sleep_ms(20)

        # 2. Clock configuration
        self._write_reg(ES7210_CLOCK_OFF_REG01, 0x3F)   # All clocks off
        self._write_reg(ES7210_MASTER_CLK_REG03, 0x04)  # MCLK from pin

        lrck_div = mclk_hz // sample_rate
        self._write_reg(ES7210_LRCK_DIVH_REG04, (lrck_div >> 8) & 0xFF)
        self._write_reg(ES7210_LRCK_DIVL_REG05, lrck_div & 0xFF)
        self._write_reg(ES7210_MAINCLK_REG02, 0x01)
        self._write_reg(ES7210_OSR_REG07, 0x20)

        print("[ES7210] MCLK={} Hz  FS={} Hz  LRCK_DIV={}".format(
            mclk_hz, sample_rate, lrck_div))

        # 3. Slave mode + I2S format
        self._write_reg(ES7210_MODE_CONFIG_REG08, 0x20)
        self._write_reg(ES7210_SDP_INTERFACE1_REG11, 0x60)  # I2S, 16-bit
        self._write_reg(ES7210_SDP_INTERFACE2_REG12, 0x00)

        # 4. Analogue power on
        self._write_reg(ES7210_ANALOG_REG40, 0x42)
        self._write_reg(ES7210_MIC12_BIAS_REG41, 0x70)
        self._write_reg(ES7210_MIC34_BIAS_REG42, 0x70)
        print("[ES7210] Analog power ON")

        # 5. Microphone gain (~24 dB)
        self._write_reg(ES7210_MIC1_GAIN_REG43, 0x1C)
        self._write_reg(ES7210_MIC2_GAIN_REG44, 0x1C)
        self._write_reg(ES7210_MIC3_GAIN_REG45, 0x1C)
        self._write_reg(ES7210_MIC4_GAIN_REG46, 0x1C)
        print("[ES7210] MIC gain set to ~24dB")

        # 6. MIC channel power on
        self._write_reg(ES7210_MIC1_POWER_REG47, 0x08)
        self._write_reg(ES7210_MIC2_POWER_REG48, 0x08)
        self._write_reg(ES7210_MIC3_POWER_REG49, 0x08)
        self._write_reg(ES7210_MIC4_POWER_REG4A, 0x08)
        self._write_reg(ES7210_MIC12_POWER_REG4B, 0x0F)
        self._write_reg(ES7210_MIC34_POWER_REG4C, 0x0F)
        print("[ES7210] MIC channel power ON")

        # 7. Disable automute
        self._write_reg(ES7210_ADC_AUTOMUTE_REG13, 0x00)

        # 8. Ensure no power-down
        self._write_reg(ES7210_POWER_DOWN_REG06, 0x00)

        # 9. Enable clocks
        self._write_reg(ES7210_CLOCK_OFF_REG01, 0x00)

        # 10. Start state machine
        self._write_reg(ES7210_RESET_REG00, 0x71)
        time.sleep_ms(100)
        self._write_reg(ES7210_RESET_REG00, 0x41)
        time.sleep_ms(50)

        print("[ES7210] Init Complete")
        return True

    def set_mic_gain(self, mic: int, gain: int) -> bool:
        """Set gain for a microphone channel.

        :param mic:  Channel index (1–4).
        :param gain: Gain constant (GAIN_0DB … GAIN_37_5DB).
        :return: True on success.
        """
        reg_map = {
            1: ES7210_MIC1_GAIN_REG43,
            2: ES7210_MIC2_GAIN_REG44,
            3: ES7210_MIC3_GAIN_REG45,
            4: ES7210_MIC4_GAIN_REG46,
        }
        reg = reg_map.get(mic)
        if reg is None:
            return False
        self._write_reg(reg, 0x10)
        return self._write_reg(reg, 0x10 | (gain & 0x0F))

    def mute(self, enable: bool) -> bool:
        """Enable or disable ADC mute.

        :param enable: True = mute, False = unmute.
        :return: True on success.
        """
        return self._write_reg(ES7210_ADC_AUTOMUTE_REG13, 0xFF if enable else 0x00)

    def power_mic(self, mic: int, enable: bool) -> bool:
        """Power a microphone channel on or off.

        :param mic:    Channel index (1–4).
        :param enable: True = power on, False = power off.
        :return: True on success.
        """
        reg_map = {
            1: ES7210_MIC1_POWER_REG47,
            2: ES7210_MIC2_POWER_REG48,
            3: ES7210_MIC3_POWER_REG49,
            4: ES7210_MIC4_POWER_REG4A,
        }
        reg = reg_map.get(mic)
        if reg is None:
            return False
        return self._write_reg(reg, 0x08 if enable else 0x00)
