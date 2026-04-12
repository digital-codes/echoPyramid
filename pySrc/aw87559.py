# SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
#
# SPDX-License-Identifier: MIT

"""
aw87559.py - MicroPython port of AW87559.h / AW87559.cpp

Driver for the AW87559 Smart Audio Amplifier (Class-K PA with boost converter).

Key features:
  - Integrated boost converter (6.5 V – 9.5 V)
  - PA gain range: 0 dB – 27 dB (1.5 dB steps)
  - Three-stage AGC (AGC1 / AGC2 / AGC3)
  - Adaptive boost modes
"""

from i2c_bus import I2CBus
import time

AW87559_I2C_ADDR = 0x5B

# Register addresses
AW87559_REG_ID          = 0x00
AW87559_REG_SYSCTRL     = 0x01
AW87559_REG_BATSAFE     = 0x02
AW87559_REG_BSTOVR      = 0x03
AW87559_REG_BSTCPR2     = 0x05
AW87559_REG_PAGR        = 0x06
AW87559_REG_AGC3PR      = 0x07
AW87559_REG_AGC3_TIME   = 0x08
AW87559_REG_AGC2PR      = 0x09
AW87559_REG_AGC2_TIME   = 0x0A
AW87559_REG_AGC1PR      = 0x0B
AW87559_REG_ADPMODE     = 0x0C
AW87559_REG_ADPBST_TIME1 = 0x0D
AW87559_REG_ADPBST_VTH  = 0x0F

# SYSCTRL bit masks
AW87559_SYS_EN_SW_MASK    = (1 << 6)
AW87559_SYS_EN_BOOST_MASK = (1 << 4)
AW87559_SYS_EN_PA_MASK    = (1 << 3)
AW87559_SYS_RCV_MODE_MASK = (1 << 2)
AW87559_SYS_EN_HVBAT_MASK = (1 << 0)

# BATSAFE
AW87559_BAT_EN_SFGD_MASK = (1 << 1)

# Boost voltage / peak current masks
AW87559_BST_VOUT_MASK  = 0x0F
AW87559_BST_IPEAK_MASK = 0x0F

# PA gain
AW87559_PAGR_GAIN_MASK = 0x1F

# AGC3
AW87559_AGC3_OUTPUT_POWER_MASK = 0x0F
AW87559_AGC3_PD_MASK           = 0x10
AW87559_AGC3_PD_SHIFT          = 4
AW87559_AGC3_REL_MASK          = 0xE0
AW87559_AGC3_REL_SHIFT         = 5
AW87559_AGC3_ATT_MASK          = 0x1C
AW87559_AGC3_ATT_SHIFT         = 2

# AGC2
AW87559_AGC2_OUTPUT_MASK  = 0x0F
AW87559_AGC2_ATT_MASK     = 0x1C
AW87559_AGC2_ATT_SHIFT    = 2

# AGC1
AW87559_AGC1_ATT_MASK  = 0x06
AW87559_AGC1_ATT_SHIFT = 1
AW87559_AGC1_PD_MASK   = 0x01

# Adaptive boost
AW87559_ADPBOOST_MODE_MASK  = 0x07
AW87559_AGC1_ATT_TIMEA_MASK = 0x10
AW87559_ADP_BST_TIME2_MASK  = 0xF0
AW87559_ADP_BST_TIME2_SHIFT = 4
AW87559_ADP_BST_TIME1_MASK  = 0x0F

# Adaptive boost VTH
AW87559_ADP_LOW_STEP_MASK   = 0xC0
AW87559_ADP_LOW_STEP_SHIFT  = 6
AW87559_SET_BOOST_VTH2_MASK = 0x38
AW87559_SET_BOOST_VTH2_SHIFT = 3
AW87559_SET_BOOST_VTH1_MASK = 0x07
AW87559_SET_BOOST_VTH1_SHIFT = 0

# Gain constants (index × 1.5 dB)
AW87559_GAIN_0DB    = 0
AW87559_GAIN_1_5DB  = 1
AW87559_GAIN_3DB    = 2
AW87559_GAIN_4_5DB  = 3
AW87559_GAIN_6DB    = 4
AW87559_GAIN_7_5DB  = 5
AW87559_GAIN_9DB    = 6
AW87559_GAIN_10_5DB = 7
AW87559_GAIN_12DB   = 8
AW87559_GAIN_13_5DB = 9
AW87559_GAIN_15DB   = 10
AW87559_GAIN_16_5DB = 11
AW87559_GAIN_18DB   = 12
AW87559_GAIN_19_5DB = 13
AW87559_GAIN_21DB   = 14
AW87559_GAIN_22_5DB = 15
AW87559_GAIN_24DB   = 16
AW87559_GAIN_25_5DB = 17
AW87559_GAIN_27DB   = 18

# Boost voltage constants
AW87559_BST_6_5V  = 0x00
AW87559_BST_6_75V = 0x01
AW87559_BST_7_0V  = 0x02
AW87559_BST_7_25V = 0x03
AW87559_BST_7_5V  = 0x04
AW87559_BST_7_75V = 0x05
AW87559_BST_8_0V  = 0x06
AW87559_BST_8_25V = 0x07
AW87559_BST_8_5V  = 0x08
AW87559_BST_8_75V = 0x09
AW87559_BST_9_0V  = 0x0A
AW87559_BST_9_25V = 0x0B
AW87559_BST_9_5V  = 0x0C

# AGC3 power constants (W)
AW87559_AGC3_0_5W = 0
AW87559_AGC3_1_0W = 5
AW87559_AGC3_1_5W = 10
AW87559_AGC3_2_0W = 15

# Adaptive boost mode constants
AW87559_ADP_PASS_THROUGH = 0
AW87559_ADP_FORCE_BOOST  = 1
AW87559_ADP_BOOST_MD1    = 2
AW87559_ADP_BOOST_MD2    = 3


class AW87559:
    """Driver for AW87559 smart audio amplifier."""

    def __init__(self, bus: I2CBus, addr: int = AW87559_I2C_ADDR):
        self._bus = bus
        self._addr = addr

    # ------------------------------------------------------------------
    # Private helpers
    # ------------------------------------------------------------------

    def _write_reg(self, reg: int, val: int) -> bool:
        return self._bus.write_bytes(self._addr, reg, bytes([val & 0xFF]))

    def _read_reg(self, reg: int):
        data = self._bus.read_bytes(self._addr, reg, 1)
        return data[0] if data else None

    def _update_bits(self, reg: int, mask: int, value: int) -> bool:
        """Read-modify-write: clear mask bits then set (value & mask)."""
        val = self._read_reg(reg)
        if val is None:
            return False
        val = (val & ~(mask & 0xFF)) & 0xFF
        val = (val | (value & mask)) & 0xFF
        return self._write_reg(reg, val)

    # ------------------------------------------------------------------
    # Initialisation
    # ------------------------------------------------------------------

    def begin(self) -> bool:
        """Initialise AW87559 with default power-on settings."""
        time.sleep_ms(10)
        chip_id = self._read_reg(AW87559_REG_ID)
        if chip_id is None or chip_id != 0x5A:
            print("AW87559 ID error: 0x{:02X}".format(
                chip_id if chip_id is not None else 0xFF))
            return False
        print("AW87559 Ready")
        self.set_software_enable(True)
        self.set_boost_enable(True)
        self.set_pa_enable(True)
        self.set_pa_gain(AW87559_GAIN_3DB) # was 16_5
        return True

    # ------------------------------------------------------------------
    # SYSCTRL (0x01)
    # ------------------------------------------------------------------

    def set_software_enable(self, en: bool) -> bool:
        return self._update_bits(AW87559_REG_SYSCTRL, AW87559_SYS_EN_SW_MASK,
                                 AW87559_SYS_EN_SW_MASK if en else 0)

    def set_boost_enable(self, en: bool) -> bool:
        return self._update_bits(AW87559_REG_SYSCTRL, AW87559_SYS_EN_BOOST_MASK,
                                 AW87559_SYS_EN_BOOST_MASK if en else 0)

    def set_pa_enable(self, en: bool) -> bool:
        return self._update_bits(AW87559_REG_SYSCTRL, AW87559_SYS_EN_PA_MASK,
                                 AW87559_SYS_EN_PA_MASK if en else 0)

    def set_receiver_mode(self, en: bool) -> bool:
        return self._update_bits(AW87559_REG_SYSCTRL, AW87559_SYS_RCV_MODE_MASK,
                                 AW87559_SYS_RCV_MODE_MASK if en else 0)

    def set_hvbat_enable(self, en: bool) -> bool:
        return self._update_bits(AW87559_REG_SYSCTRL, AW87559_SYS_EN_HVBAT_MASK,
                                 AW87559_SYS_EN_HVBAT_MASK if en else 0)

    # ------------------------------------------------------------------
    # Boost voltage / peak current
    # ------------------------------------------------------------------

    def set_boost_voltage(self, voltage: int) -> bool:
        """Set boost converter output voltage (AW87559_BST_* constant)."""
        if voltage > AW87559_BST_9_5V:
            voltage = AW87559_BST_9_5V
        return self._update_bits(AW87559_REG_BSTOVR, AW87559_BST_VOUT_MASK, voltage)

    def get_boost_voltage(self):
        """Return boost voltage setting, or None on failure."""
        val = self._read_reg(AW87559_REG_BSTOVR)
        return (val & AW87559_BST_VOUT_MASK) if val is not None else None

    def enable_battery_protection(self, en: bool) -> bool:
        return self._update_bits(AW87559_REG_BATSAFE, AW87559_BAT_EN_SFGD_MASK,
                                 AW87559_BAT_EN_SFGD_MASK if en else 0)

    def set_boost_ipeak(self, ipeak: int) -> bool:
        return self._update_bits(AW87559_REG_BSTCPR2, AW87559_BST_IPEAK_MASK, ipeak)

    def get_boost_ipeak(self):
        val = self._read_reg(AW87559_REG_BSTCPR2)
        return (val & AW87559_BST_IPEAK_MASK) if val is not None else None

    # ------------------------------------------------------------------
    # PA gain
    # ------------------------------------------------------------------

    def set_pa_gain(self, gain: int) -> bool:
        """Set PA gain (AW87559_GAIN_* constant; 0–18 → 0–27 dB)."""
        if gain > AW87559_GAIN_27DB:
            gain = AW87559_GAIN_27DB
        return self._update_bits(AW87559_REG_PAGR, AW87559_PAGR_GAIN_MASK, gain)

    def get_pa_gain(self) -> float:
        """Return PA gain in dB (as float), or 0.0 on failure."""
        val = self._read_reg(AW87559_REG_PAGR)
        return 0.0 if val is None else (val & AW87559_PAGR_GAIN_MASK) * 1.5

    # ------------------------------------------------------------------
    # AGC3
    # ------------------------------------------------------------------

    def enable_agc3(self, en: bool) -> bool:
        val = 0 if en else AW87559_AGC3_PD_MASK
        return self._update_bits(AW87559_REG_AGC3PR, AW87559_AGC3_PD_MASK, val)

    def set_agc3_power(self, power: int) -> bool:
        if power > AW87559_AGC3_2_0W:
            power = AW87559_AGC3_2_0W
        return self._update_bits(AW87559_REG_AGC3PR, AW87559_AGC3_OUTPUT_POWER_MASK, power)

    def get_agc3_power(self):
        val = self._read_reg(AW87559_REG_AGC3PR)
        return (val & AW87559_AGC3_OUTPUT_POWER_MASK) if val is not None else None

    def set_agc3_release_time(self, rel: int) -> bool:
        return self._update_bits(AW87559_REG_AGC3_TIME, AW87559_AGC3_REL_MASK,
                                 rel << AW87559_AGC3_REL_SHIFT)

    def set_agc3_attack_time(self, att: int) -> bool:
        return self._update_bits(AW87559_REG_AGC3_TIME, AW87559_AGC3_ATT_MASK,
                                 att << AW87559_AGC3_ATT_SHIFT)

    # ------------------------------------------------------------------
    # AGC2
    # ------------------------------------------------------------------

    def set_agc2_power(self, power: int) -> bool:
        return self._update_bits(AW87559_REG_AGC2PR, AW87559_AGC2_OUTPUT_MASK, power)

    def set_agc2_attack_time(self, att: int) -> bool:
        return self._update_bits(AW87559_REG_AGC2_TIME, AW87559_AGC2_ATT_MASK,
                                 att << AW87559_AGC2_ATT_SHIFT)

    def get_agc2_attack_time(self):
        val = self._read_reg(AW87559_REG_AGC2_TIME)
        return ((val & AW87559_AGC2_ATT_MASK) >> AW87559_AGC2_ATT_SHIFT) if val is not None else None

    # ------------------------------------------------------------------
    # AGC1
    # ------------------------------------------------------------------

    def set_agc1_attack_time(self, att: int) -> bool:
        return self._update_bits(AW87559_REG_AGC1PR, AW87559_AGC1_ATT_MASK,
                                 att << AW87559_AGC1_ATT_SHIFT)

    def get_agc1_attack_time(self):
        val = self._read_reg(AW87559_REG_AGC1PR)
        return ((val & AW87559_AGC1_ATT_MASK) >> AW87559_AGC1_ATT_SHIFT) if val is not None else None

    def enable_agc1(self, en: bool) -> bool:
        val = 0 if en else AW87559_AGC1_PD_MASK
        return self._update_bits(AW87559_REG_AGC1PR, AW87559_AGC1_PD_MASK, val)

    # ------------------------------------------------------------------
    # Adaptive boost
    # ------------------------------------------------------------------

    def set_adp_mode(self, mode: int) -> bool:
        return self._update_bits(AW87559_REG_ADPMODE, AW87559_ADPBOOST_MODE_MASK, mode)

    def get_adp_mode(self):
        val = self._read_reg(AW87559_REG_ADPMODE)
        return (val & AW87559_ADPBOOST_MODE_MASK) if val is not None else None

    def set_agc1_timea_enabled(self, en: bool) -> bool:
        val = AW87559_AGC1_ATT_TIMEA_MASK if en else 0
        return self._update_bits(AW87559_REG_ADPMODE, AW87559_AGC1_ATT_TIMEA_MASK, val)

    def set_adp_time1(self, t: int) -> bool:
        return self._update_bits(AW87559_REG_ADPBST_TIME1, AW87559_ADP_BST_TIME1_MASK, t)

    def set_adp_time2(self, t: int) -> bool:
        return self._update_bits(AW87559_REG_ADPBST_TIME1, AW87559_ADP_BST_TIME2_MASK,
                                 t << AW87559_ADP_BST_TIME2_SHIFT)

    def set_adp_low_step(self, step: int) -> bool:
        return self._update_bits(AW87559_REG_ADPBST_VTH, AW87559_ADP_LOW_STEP_MASK,
                                 step << AW87559_ADP_LOW_STEP_SHIFT)

    def set_boost_vth1(self, vth1: int) -> bool:
        return self._update_bits(AW87559_REG_ADPBST_VTH, AW87559_SET_BOOST_VTH1_MASK,
                                 vth1 << AW87559_SET_BOOST_VTH1_SHIFT)

    def set_boost_vth2(self, vth2: int) -> bool:
        return self._update_bits(AW87559_REG_ADPBST_VTH, AW87559_SET_BOOST_VTH2_MASK,
                                 vth2 << AW87559_SET_BOOST_VTH2_SHIFT)
