# SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
#
# SPDX-License-Identifier: MIT

"""
i2c_bus.py - MicroPython port of I2CBus.h / I2CBus.cpp

Provides a thin wrapper around machine.I2C that mirrors the
writeReg / writeBytes / readBytes API used by the other drivers.
"""

from machine import I2C, Pin


class I2CBus:
    """Cross-platform I2C abstraction (MicroPython version).

    Usage::

        bus = I2CBus()
        bus.begin(id=0, sda=38, scl=39)
        bus.write_reg(0x40, 0x00, 0xFF)
        data = bus.read_bytes(0x40, 0x01, 2)
    """

    def __init__(self):
        self._i2c = None

    # ------------------------------------------------------------------
    # Initialisation
    # ------------------------------------------------------------------

    def begin(self, id: int = 0, sda: int = 38, scl: int = 39,
              freq: int = 400_000) -> bool:
        """Initialise the I2C bus.

        :param id:   I2C peripheral id (0 or 1).
        :param sda:  GPIO number for SDA.
        :param scl:  GPIO number for SCL.
        :param freq: Clock frequency in Hz (default 400 kHz).
        :return:     True on success.
        """
        try:
            self._i2c = I2C(id, scl=Pin(scl), sda=Pin(sda), freq=freq)
            return True
        except Exception as e:
            print("[I2CBus] begin failed:", e)
            return False

    # ------------------------------------------------------------------
    # Low-level helpers
    # ------------------------------------------------------------------

    def write_reg(self, addr: int, reg: int, value: int) -> bool:
        """Write a single byte to a device register.

        :param addr:  7-bit I2C device address.
        :param reg:   Register address.
        :param value: Byte value to write.
        :return:      True on success.
        """
        return self.write_bytes(addr, reg, bytes([value]))

    def write_bytes(self, addr: int, reg: int, data: bytes) -> bool:
        """Write multiple bytes starting at a register address.

        :param addr: 7-bit I2C device address.
        :param reg:  Starting register address.
        :param data: Bytes to write.
        :return:     True on success.
        """
        try:
            self._i2c.writeto_mem(addr, reg, data)
            return True
        except Exception as e:
            print("[I2CBus] write_bytes 0x{:02X} reg 0x{:02X} failed: {}".format(
                addr, reg, e))
            return False

    def read_bytes(self, addr: int, reg: int, length: int) -> bytes | None:
        """Read multiple bytes from a device register.

        :param addr:   7-bit I2C device address.
        :param reg:    Starting register address.
        :param length: Number of bytes to read.
        :return:       Bytes read, or None on failure.
        """
        try:
            return self._i2c.readfrom_mem(addr, reg, length)
        except Exception as e:
            print("[I2CBus] read_bytes 0x{:02X} reg 0x{:02X} failed: {}".format(
                addr, reg, e))
            return None
