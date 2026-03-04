/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _STM32_CTRL_H_
#define _STM32_CTRL_H_

#include "I2CBus.h"

/**
 * @defgroup STM32_RGB_TOUCH_REG STM32 RGB & Touch Register Definitions
 * @brief Register map for STM32 RGB LED and touch controller.
 *
 * This group defines all register addresses used for
 * RGB LED control, touch sensing, speaker control,
 * voltage monitoring and system configuration.
 * @{
 */

/**
 * @def TOUCH_BUTTON_STATUS_REG_ADDR
 * @brief Touch button status register.
 *
 * 4-bit touch state information.
 */
#define TOUCH_BUTTON_STATUS_REG_ADDR (0x00)

/**
 * @def RGB1_BRIGHTNESS_REG_ADDR
 * @brief RGB1 global brightness register.
 */
#define RGB1_BRIGHTNESS_REG_ADDR (0x10)

/**
 * @def RGB2_BRIGHTNESS_REG_ADDR
 * @brief RGB2 global brightness register.
 */
#define RGB2_BRIGHTNESS_REG_ADDR (0x11)

/**
 * @def RGB1_STATUS_REG_ADDR
 * @brief RGB1 LED control base register.
 */
#define RGB1_STATUS_REG_ADDR (0x20)

/**
 * @def RGB2_STATUS_REG_ADDR
 * @brief RGB2 LED control base register.
 */
#define RGB2_STATUS_REG_ADDR (0x60)

/**
 * @def SPK_RESTART_REG_ADDR
 * @brief Speaker restart control register.
 */
#define SPK_RESTART_REG_ADDR (0xA0)

/**
 * @def READ_USB5V_REG_ADDR
 * @brief USB 5V voltage read register.
 *
 * Used to read current USB input voltage level.
 */
#define READ_USB5V_REG_ADDR (0xB0)

/**
 * @def FLASH_WRITE_BACK_REG_ADDR
 * @brief Flash write-back register.
 *
 * Save current brightness settings to flash memory.
 */
#define FLASH_WRITE_BACK_REG_ADDR (0xF0)

/**
 * @def SW_VER_REG_ADDR
 * @brief Firmware version register.
 */
#define SW_VER_REG_ADDR (0xFE)

/**
 * @def I2C_ADDR_REG_ADDR
 * @brief I2C address configuration register.
 */
#define I2C_ADDR_REG_ADDR (0xFF)

/**
 * @def STM32_I2C_ADDR
 * @brief Default STM32 I2C address.
 */
#define STM32_I2C_ADDR (0x1A)

/**
 * @def RGB_NUM_MAX
 * @brief Maximum RGB LED index.
 *
 * Valid range: 0 ~ 13
 */
#define RGB_NUM_MAX (13)

/**
 * @brief STM32 peripheral controller via I2C.
 *
 * This class provides access to:
 *  - Touch buttons
 *  - Dual RGB LED channels
 *  - Brightness control
 *  - Speaker reset
 *  - USB voltage reading
 *  - Firmware & I2C configuration
 */
class STM32Ctrl {
public:
    /**
     * @brief Constructor.
     *
     * @param bus Reference to I2CBus object
     * @param addr I2C device address (default: 0x1A)
     */
    STM32Ctrl(I2CBus& bus, uint8_t addr = STM32_I2C_ADDR);

    /**
     * @brief Initialize the STM32 controller.
     *
     * @return true if device responds correctly
     */
    bool begin();

    /* ================= Touch ================= */

    /**
     * @brief Read raw 4-bit touch button status.
     *
     * Bit0~Bit3 correspond to touch key 1~4.
     *
     * @return 4-bit raw status value
     */
    uint8_t getTouchRaw();

    /**
     * @brief Check if specific touch button is pressed.
     *
     * @param index Touch button index (1~4)
     * @return true if pressed
     */
    bool isPressed(uint8_t index);

    /* ================= Brightness ================= */

    /**
     * @brief Set brightness for a specific RGB channel.
     *
     * @param channel RGB channel (1 or 2)
     * @param value Brightness value (0~255)
     * @return true if successful
     */
    bool setBrightness(uint8_t channel, uint8_t value);

    /**
     * @brief Get current brightness of a channel.
     *
     * @param channel RGB channel (1 or 2)
     * @return Brightness value (0~255)
     */
    uint8_t getBrightness(uint8_t channel);

    /* ================= RGB Control ================= */

    /**
     * @brief Set RGB value for a specific LED.
     *
     * @param channel RGB channel (1 or 2)
     * @param ledIndex LED index (0 ~ RGB_NUM_MAX)
     * @param r Red value (0~255)
     * @param g Green value (0~255)
     * @param b Blue value (0~255)
     * @return true if successful
     */
    bool setRGB(uint8_t channel, uint8_t ledIndex, uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Set all LEDs in a channel to the same color.
     *
     * @param channel RGB channel (1 or 2)
     * @param r Red value (0~255)
     * @param g Green value (0~255)
     * @param b Blue value (0~255)
     * @return true if successful
     */
    bool setAllRGB(uint8_t channel, uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Read RGB value of a specific LED.
     *
     * @param channel RGB channel (1 or 2)
     * @param ledIndex LED index
     * @param r Reference to store red value
     * @param g Reference to store green value
     * @param b Reference to store blue value
     * @return true if successful
     */
    bool getRGB(uint8_t channel, uint8_t ledIndex, uint8_t& r, uint8_t& g, uint8_t& b);

    /* ================= System Control ================= */

    /**
     * @brief Restart the speaker module.
     *
     * @return true if command sent successfully
     */
    bool resetSpeaker();

    /**
     * @brief Read USB 5V voltage.
     *
     * @param mv Output voltage in millivolts
     * @return true if successful
     */
    bool getUSB5V(uint16_t& mv);

    /**
     * @brief Save current brightness to STM32 flash memory.
     *
     * @param channel RGB channel (1 or 2)
     * @return true if successful
     */
    bool saveBrightness(uint8_t channel);

    /**
     * @brief Get firmware version of STM32.
     *
     * @return Firmware version number
     */
    uint8_t getFirmwareVersion();

    /**
     * @brief Set new I2C address for STM32.
     *
     * @param newAddr New I2C address
     * @return true if successful
     */
    bool setI2CAddress(uint8_t newAddr);

    /**
     * @brief Read current I2C address.
     *
     * @return I2C address
     */
    uint8_t getI2CAddress();

private:
    I2CBus& _bus;   ///< Reference to I2C bus
    uint8_t _addr;  ///< Current I2C address

    /**
     * @brief Write raw RGB data page.
     *
     * @param channel RGB channel
     * @param page Page index
     * @param data Pointer to RGB data buffer
     * @return true if successful
     */
    bool writeRawPage(uint8_t channel, uint8_t page, uint8_t* data);

    /**
     * @brief Calculate base register address for specific LED.
     *
     * @param channel RGB channel
     * @param ledIndex LED index
     * @return Base register address
     */
    uint8_t calcRGBBaseReg(uint8_t channel, uint8_t ledIndex);
};

#endif