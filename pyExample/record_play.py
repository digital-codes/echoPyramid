# SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
#
# SPDX-License-Identifier: MIT

"""
record_play.py - MicroPython example for M5EchoPyramid

Equivalent to examples/RecordPlay/RecordPlay.ino

Features:
  - Continuous rainbow RGB animation at startup
  - Touch any button (1–4) to record 5 seconds of audio into RAM
  - Automatic playback after recording completes

Audio format:
  - 44.1 kHz, 16-bit mono
  - Approximate RAM usage: ~430 KB

Target: ESP32 / M5Stack device running MicroPython with
        the M5EchoPyramid accessory attached.

Usage:
    Copy this file and the entire pySrc/ directory to the device,
    then import or run:

        import record_play
"""

import sys
import os
sys.path.insert(0, '/pySrc')   # adjust if pySrc is mounted elsewhere

import time
import struct

from m5echo_pyramid import M5EchoPyramid

# ------------------------------------------------------------------
# Configuration
# ------------------------------------------------------------------

SAMPLE_RATE    = 44100   # Hz
RECORD_SECONDS = 5       # seconds
FRAME_SIZE     = 256     # samples per read/write call
TOTAL_FRAMES   = SAMPLE_RATE * RECORD_SECONDS   # 220 500 samples

# ------------------------------------------------------------------
# Global state
# ------------------------------------------------------------------

ep = M5EchoPyramid()
record_buffer = None   # bytearray allocated after begin()
recording_busy = False
hue_offset = 0.0

# ------------------------------------------------------------------
# Utility
# ------------------------------------------------------------------

def hsv2rgb(h, s, v):
    """Convert HSV (0.0–1.0 each) to (r, g, b) 0–255 integers."""
    i = int(h * 6)
    f = h * 6 - i
    p = v * (1 - s)
    q = v * (1 - f * s)
    t = v * (1 - (1 - f) * s)

    seg = i % 6
    if   seg == 0: r, g, b = v, t, p
    elif seg == 1: r, g, b = q, v, p
    elif seg == 2: r, g, b = p, v, t
    elif seg == 3: r, g, b = p, q, v
    elif seg == 4: r, g, b = t, p, v
    else:          r, g, b = v, p, q

    return int(r * 255), int(g * 255), int(b * 255)


# ------------------------------------------------------------------
# RGB effects
# ------------------------------------------------------------------

def effect_rainbow():
    """Rotating rainbow animation across both LED rings."""
    global hue_offset
    for i in range(14):
        hue = (i / 14.0 + hue_offset) % 1.0
        r, g, b = hsv2rgb(hue, 1.0, 1.0)
        ep.ctrl.set_rgb(1, i, r, g, b)
        ep.ctrl.set_rgb(2, i, r, g, b)
    hue_offset = (hue_offset + 0.003) % 1.0


# ------------------------------------------------------------------
# Audio: record → playback
# ------------------------------------------------------------------

def record_and_play():
    """Record RECORD_SECONDS seconds then play back from RAM buffer."""
    global recording_busy, record_buffer

    if recording_busy:
        return
    recording_busy = True

    print("Start Recording...")

    recorded = 0
    buf_view = memoryview(record_buffer)

    while recorded < TOTAL_FRAMES:
        mic, _ref = ep.read(FRAME_SIZE)
        # Copy mic data (2 bytes per sample) into the record buffer
        end = min(FRAME_SIZE, TOTAL_FRAMES - recorded)
        byte_end = end * 2
        buf_view[recorded * 2 : recorded * 2 + byte_end] = mic[:byte_end]
        recorded += end

    print("Recording Done.")
    time.sleep_ms(200)

    ep.codec.mute(False)
    print("Start Playback...")

    played = 0
    while played < TOTAL_FRAMES:
        chunk_frames = min(FRAME_SIZE, TOTAL_FRAMES - played)
        chunk = buf_view[played * 2 : played * 2 + chunk_frames * 2]
        ep.write(chunk, chunk_frames)
        played += chunk_frames

    print("Playback Done.")
    recording_busy = False


# ------------------------------------------------------------------
# Main
# ------------------------------------------------------------------

def setup():
    """Initialise hardware — call once at startup."""
    global record_buffer

    ep.begin(
        i2c_id=0,
        sda=38, scl=39,
        bclk=6, lrck=8, dout=5, din=7,
        sample_rate=SAMPLE_RATE,
    )

    ep.codec.set_volume(50)
    ep.codec.mute(False)
    ep.ctrl.set_brightness(1, 100)
    ep.ctrl.set_brightness(2, 100)

    # Allocate recording buffer: TOTAL_FRAMES × 2 bytes (16-bit mono)
    try:
        record_buffer = bytearray(TOTAL_FRAMES * 2)
    except MemoryError:
        print("Memory allocation failed! Reduce RECORD_SECONDS.")
        raise

    print("System Ready.")


def loop():
    """Call repeatedly from the main loop."""
    effect_rainbow()

    for i in range(1, 5):
        if ep.ctrl.is_pressed(i):
            record_and_play()
            break

    time.sleep_ms(30)


if __name__ == "__main__":
    setup()
    while True:
        loop()
