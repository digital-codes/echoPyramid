# SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
#
# SPDX-License-Identifier: MIT

"""
m5echo_pyramid.py - MicroPython port of M5EchoPyramid.h / M5EchoPyramid.cpp

High-level wrapper that integrates:
  - SI5351  clock generator
  - ES7210  ADC (microphone input)
  - ES8311  audio codec (DAC + ADC)
  - STM32Ctrl  touch + RGB controller
  - AW87559  power amplifier
  - machine.I2S  audio I/O

Supported sample rates: 16000, 44100, 48000 Hz
"""

import struct
import time
from machine import I2S, Pin

from i2c_bus import I2CBus
from si5351 import SI5351
from es7210 import ES7210, ES7210_INPUT_MIC1, ES7210_INPUT_MIC3
from es8311 import ES8311
from stm32_ctrl import STM32Ctrl
from aw87559 import AW87559

# Supported sample rates
_SUPPORTED_RATES = (16000, 44100, 48000)


class M5EchoPyramid:
    """High-level audio system wrapper for M5EchoPyramid.

    Typical usage (MicroPython)::

        from m5echo_pyramid import M5EchoPyramid

        ep = M5EchoPyramid()
        ep.begin(i2c_id=0, sda=38, scl=39, bclk=6, lrck=8, dout=5, din=7)

        # Read 256 microphone samples
        mic, ref = ep.read(256)

        # Play 256 samples
        ep.write(samples)
    """

    def __init__(self):
        self._bus = I2CBus()
        self._si5351 = None
        self._es7210 = None
        self._es8311 = None
        self._stm32 = None
        self._pa = None

        self._i2s_rx = None
        self._i2s_tx = None
        self._sample_rate = 44100

    # ------------------------------------------------------------------
    # Initialisation
    # ------------------------------------------------------------------

    def begin(self, i2c_id: int = 0, sda: int = 38, scl: int = 39,
              bclk: int = 6, lrck: int = 8, dout: int = 5, din: int = 7,
              sample_rate: int = 44100) -> bool:
        """Initialise the entire M5EchoPyramid system.

        :param i2c_id:     I2C peripheral id (0 or 1).
        :param sda:        SDA GPIO.
        :param scl:        SCL GPIO.
        :param bclk:       I2S bit-clock GPIO.
        :param lrck:       I2S word-select (WS/LRCK) GPIO.
        :param dout:       I2S data-output GPIO (DAC path).
        :param din:        I2S data-input GPIO (ADC path).
        :param sample_rate: Audio sample rate (16000 / 44100 / 48000).
        :return: True on success.
        """
        if sample_rate not in _SUPPORTED_RATES:
            print("[M5EchoPyramid] Unsupported sample rate: {}".format(sample_rate))
            return False

        self._sample_rate = sample_rate

        # Initialise I2C bus
        if not self._bus.begin(i2c_id, sda, scl):
            return False

        # Instantiate sub-drivers
        self._si5351 = SI5351(self._bus)
        self._es7210 = ES7210(self._bus)
        self._es8311 = ES8311(self._bus)
        self._stm32 = STM32Ctrl(self._bus)
        self._pa = AW87559(self._bus)

        # Start clock generator
        self._si5351.begin()

        # Configure I2S
        self._init_i2s(bclk, lrck, dout, din, sample_rate)

        mclk_hz = sample_rate * 256
        self._es7210.begin(mclk_hz, sample_rate,
                           ES7210_INPUT_MIC1 | ES7210_INPUT_MIC3)
        self._es8311.begin(mclk_hz, sample_rate)
        self._stm32.begin()
        self._pa.begin()

        return True

    # ------------------------------------------------------------------
    # I2S setup
    # ------------------------------------------------------------------

    def _init_i2s(self, bclk: int, lrck: int, dout: int, din: int,
                  sample_rate: int):
        """Configure the I2S peripheral for both TX and RX."""
        mclk = sample_rate * 256
        self._si5351.set_mclk(mclk)
        time.sleep_ms(10)

        # Close existing channels if reinitialising
        if self._i2s_tx:
            self._i2s_tx.deinit()
            self._i2s_tx = None
        if self._i2s_rx:
            self._i2s_rx.deinit()
            self._i2s_rx = None

        # TX channel (playback)
        self._i2s_tx = I2S(
            0,
            sck=Pin(bclk),
            ws=Pin(lrck),
            sd=Pin(dout),
            mode=I2S.TX,
            bits=16,
            format=I2S.STEREO,
            rate=sample_rate,
            ibuf=4096,
        )

        # RX channel (capture) – uses a second I2S id when available;
        # on ESP32 both TX and RX share the same peripheral so we use id=1.
        self._i2s_rx = I2S(
            1,
            sck=Pin(bclk),
            ws=Pin(lrck),
            sd=Pin(din),
            mode=I2S.RX,
            bits=16,
            format=I2S.STEREO,
            rate=sample_rate,
            ibuf=4096,
        )

        print("[M5EchoPyramid] I2S init OK - {} Hz".format(sample_rate))

    # ------------------------------------------------------------------
    # Audio I/O
    # ------------------------------------------------------------------

    def read(self, frames: int = 256):
        """Read audio frames from the microphone.

        Reads interleaved stereo 16-bit PCM from I2S RX (left = mic,
        right = reference) and de-interleaves into two separate arrays.

        :param frames: Number of mono frames to read.
        :return: (mic, ref) — two ``bytearray`` objects of *frames* × 2 bytes
                 each containing 16-bit little-endian PCM samples.
        """
        byte_count = frames * 2 * 2  # frames × 2 channels × 2 bytes/sample
        raw = bytearray(byte_count)
        self._i2s_rx.readinto(raw)

        mic = bytearray(frames * 2)
        ref = bytearray(frames * 2)

        for i in range(frames):
            offset = i * 4
            # Left channel (MIC) at offset+0, Right channel (REF) at offset+2
            mic[i * 2]     = raw[offset]
            mic[i * 2 + 1] = raw[offset + 1]
            ref[i * 2]     = raw[offset + 2]
            ref[i * 2 + 1] = raw[offset + 3]

        return mic, ref

    def write(self, data: bytes | bytearray, frames: int | None = None):
        """Write mono audio frames to the speaker.

        Each mono sample is duplicated to both stereo channels before
        being sent over I2S TX.

        :param data:   16-bit little-endian PCM mono samples.
        :param frames: Number of frames; inferred from *data* length if None.
        """
        if frames is None:
            frames = len(data) // 2

        # Build interleaved stereo buffer
        out = bytearray(frames * 4)
        for i in range(frames):
            s0 = data[i * 2]
            s1 = data[i * 2 + 1]
            out[i * 4]     = s0  # L
            out[i * 4 + 1] = s1
            out[i * 4 + 2] = s0  # R (duplicate)
            out[i * 4 + 3] = s1

        self._i2s_tx.write(out)

    # ------------------------------------------------------------------
    # Sub-driver accessors
    # ------------------------------------------------------------------

    @property
    def codec(self) -> ES8311:
        """ES8311 codec reference."""
        return self._es8311

    @property
    def adc(self) -> ES7210:
        """ES7210 ADC reference."""
        return self._es7210

    @property
    def ctrl(self) -> STM32Ctrl:
        """STM32 controller reference."""
        return self._stm32

    @property
    def pa(self) -> AW87559:
        """AW87559 power amplifier reference."""
        return self._pa
