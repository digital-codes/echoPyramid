# SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
#
# SPDX-License-Identifier: MIT

"""
stm32_ctrl.py - MicroPython port of STM32Ctrl.h / STM32Ctrl.cpp

Driver for the STM32 peripheral controller:
  - Touch buttons (4 keys)
  - Dual RGB LED channels
  - Brightness control
  - Speaker restart
  - USB 5 V voltage reading
  - Firmware & I2C configuration
"""

from i2c_bus import I2CBus
import time

STM32_I2C_ADDR = 0x1A

TOUCH_BUTTON_STATUS_REG_ADDR = 0x00
RGB1_BRIGHTNESS_REG_ADDR     = 0x10
RGB2_BRIGHTNESS_REG_ADDR     = 0x11
RGB1_STATUS_REG_ADDR         = 0x20
RGB2_STATUS_REG_ADDR         = 0x60
SPK_RESTART_REG_ADDR         = 0xA0
READ_USB5V_REG_ADDR          = 0xB0
FLASH_WRITE_BACK_REG_ADDR    = 0xF0
SW_VER_REG_ADDR              = 0xFE
I2C_ADDR_REG_ADDR            = 0xFF

RGB_NUM_MAX = 13


class STM32Ctrl:
    """Driver for the STM32 touch and RGB LED controller."""

    def __init__(self, bus: I2CBus, addr: int = STM32_I2C_ADDR):
        self._bus = bus
        self._addr = addr

    # ------------------------------------------------------------------
    # Initialisation
    # ------------------------------------------------------------------

    def begin(self) -> bool:
        """Initialise the STM32 controller (restarts the speaker)."""
        self.reset_speaker()
        time.sleep_ms(100)
        return True

    # ------------------------------------------------------------------
    # Touch
    # ------------------------------------------------------------------

    def get_touch_raw(self) -> int:
        """Read 4-bit touch button status bitmask (bits 0–3 → keys 1–4)."""
        data = self._bus.read_bytes(self._addr, TOUCH_BUTTON_STATUS_REG_ADDR, 4)
        if data is None:
            return 0
        state = 0
        if data[0]: state |= 0x01
        if data[1]: state |= 0x02
        if data[2]: state |= 0x04
        if data[3]: state |= 0x08
        return state

    def is_pressed(self, index: int) -> bool:
        """Check if a touch button is pressed.

        :param index: Button index (1–4).
        :return: True if pressed.
        """
        if index < 1 or index > 4:
            return False
        return bool((self.get_touch_raw() >> (index - 1)) & 0x01)

    # ------------------------------------------------------------------
    # Brightness
    # ------------------------------------------------------------------

    def set_brightness(self, channel: int, value: int) -> bool:
        """Set brightness for an RGB channel.

        :param channel: 1 or 2.
        :param value:   Brightness 0–100.
        :return: True on success.
        """
        if value > 100:
            value = 100
        reg = RGB1_BRIGHTNESS_REG_ADDR + (channel - 1)
        return self._bus.write_reg(self._addr, reg, value)

    def get_brightness(self, channel: int) -> int:
        """Get brightness for an RGB channel.

        :param channel: 1 or 2.
        :return: Brightness value 0–255.
        """
        if channel < 1 or channel > 2:
            return 0
        reg = RGB1_BRIGHTNESS_REG_ADDR + (channel - 1)
        data = self._bus.read_bytes(self._addr, reg, 1)
        return data[0] if data else 0

    # ------------------------------------------------------------------
    # RGB control
    # ------------------------------------------------------------------

    def _calc_rgb_base_reg(self, channel: int, led_index: int) -> int:
        base = RGB1_STATUS_REG_ADDR if channel == 1 else RGB2_STATUS_REG_ADDR
        return base + led_index * 4

    def set_rgb(self, channel: int, led_index: int,
                r: int, g: int, b: int) -> bool:
        """Set RGB colour for a specific LED.

        :param channel:   1 or 2.
        :param led_index: LED index (0–RGB_NUM_MAX).
        :param r, g, b:   Colour components 0–255.
        :return: True on success.
        """
        if led_index > RGB_NUM_MAX:
            led_index = RGB_NUM_MAX
        reg = self._calc_rgb_base_reg(channel, led_index)
        return self._bus.write_bytes(self._addr, reg, bytes([b, g, r, 0x00]))

    def set_all_rgb(self, channel: int, r: int, g: int, b: int) -> bool:
        """Set all LEDs in a channel to the same colour.

        :param channel: 1 or 2.
        :param r, g, b: Colour components 0–255.
        :return: True on success.
        """
        # 4 pages × 4 LEDs per page; each LED occupies 3 bytes (B, G, R)
        data = bytearray(12)
        for i in range(4):
            data[i * 3 + 0] = b
            data[i * 3 + 1] = g
            data[i * 3 + 2] = r

        base = RGB1_STATUS_REG_ADDR if channel == 1 else RGB2_STATUS_REG_ADDR
        for page in range(4):
            reg = base + page * 0x10
            self._bus.write_bytes(self._addr, reg, bytes(data))
        return True

    def get_rgb(self, channel: int, led_index: int):
        """Read RGB value for a specific LED.

        :param channel:   1 or 2.
        :param led_index: LED index (0–RGB_NUM_MAX).
        :return: (r, g, b) tuple, or None on failure.
        """
        if led_index > RGB_NUM_MAX:
            led_index = RGB_NUM_MAX
        base = RGB1_STATUS_REG_ADDR if channel == 1 else RGB2_STATUS_REG_ADDR
        buf = self._bus.read_bytes(self._addr, base, 56)
        if buf is None:
            return None
        offset = led_index * 4
        return buf[offset + 2], buf[offset + 1], buf[offset]  # r, g, b

    # ------------------------------------------------------------------
    # System control
    # ------------------------------------------------------------------

    def reset_speaker(self) -> bool:
        """Send speaker restart command."""
        return self._bus.write_reg(self._addr, SPK_RESTART_REG_ADDR, 1)

    def get_usb5v(self):
        """Read USB 5 V voltage in millivolts.

        :return: Voltage in mV, or None on failure.
        """
        data = self._bus.read_bytes(self._addr, READ_USB5V_REG_ADDR, 2)
        if data is None:
            return None
        return (data[1] << 8) | data[0]

    def save_brightness(self, channel: int) -> bool:
        """Save current brightness to STM32 flash.

        :param channel: 1 or 2.
        :return: True on success.
        """
        if channel not in (1, 2):
            return False
        return self._bus.write_reg(self._addr, FLASH_WRITE_BACK_REG_ADDR, channel)

    def get_firmware_version(self) -> int:
        """Get STM32 firmware version number."""
        data = self._bus.read_bytes(self._addr, SW_VER_REG_ADDR, 1)
        return data[0] if data else 0

    def set_i2c_address(self, new_addr: int) -> bool:
        """Change the STM32 I2C address.

        :param new_addr: New 7-bit address (0x08–0x77).
        :return: True on success.
        """
        if new_addr < 0x08 or new_addr > 0x77:
            return False
        ok = self._bus.write_reg(self._addr, I2C_ADDR_REG_ADDR, new_addr)
        if ok:
            self._addr = new_addr
        return ok

    def get_i2c_address(self) -> int:
        """Read the current I2C address from the device."""
        data = self._bus.read_bytes(self._addr, I2C_ADDR_REG_ADDR, 1)
        return data[0] if data else 0
