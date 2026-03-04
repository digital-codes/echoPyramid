/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file AW87559.h
 * @brief Driver for the AW87559 Smart Audio Amplifier (Class-K PA with Boost Converter).
 *
 * The AW87559 is a high-efficiency mono Class-K audio power amplifier with an integrated
 * adaptive boost converter. It features three-stage AGC (Automatic Gain Control) for
 * speaker protection and adaptive boost mode for power optimization.
 *
 * Key features:
 * - Integrated boost converter with configurable output voltage (6.5V ~ 9.5V)
 * - PA gain range: 0 dB ~ 27 dB (1.5 dB steps)
 * - Three-stage AGC for speaker over-power protection
 * - Adaptive boost modes for battery life optimization
 * - I2C control interface (7-bit address: 0x5B)
 *
 * @note This driver communicates via the I2CBus abstraction layer, which supports
 *       both Arduino Wire and ESP-IDF i2c_master driver backends.
 */

#ifndef _AW87559_H_
#define _AW87559_H_

#include "I2CBus.h"

/* ================= I2C Address ================= */

/** @brief Default I2C 7-bit slave address of the AW87559. */
#define AW87559_I2C_ADDR (0x5B)

/* ================= Register Addresses ================= */

/** @brief Chip ID register (read-only). */
#define AW87559_REG_ID (0x00)

/** @brief System control register (EN_SW, EN_BOOST, EN_PA, RCV_MODE, EN_HVBAT). */
#define AW87559_REG_SYSCTRL (0x01)

/** @brief Battery safety / under-voltage protection register. */
#define AW87559_REG_BATSAFE (0x02)

/** @brief Boost output voltage configuration register. */
#define AW87559_REG_BSTOVR (0x03)

/** @brief Boost converter peak current limit register. */
#define AW87559_REG_BSTCPR2 (0x05)

/** @brief PA gain register. */
#define AW87559_REG_PAGR (0x06)

/** @brief AGC3 output power limit and enable register. */
#define AW87559_REG_AGC3PR (0x07)

/** @brief AGC3 release and attack time configuration register. */
#define AW87559_REG_AGC3_TIME (0x08)

/** @brief AGC2 output power limit register. */
#define AW87559_REG_AGC2PR (0x09)

/** @brief AGC2 attack time configuration register. */
#define AW87559_REG_AGC2_TIME (0x0A)

/** @brief AGC1 attack time and enable register. */
#define AW87559_REG_AGC1PR (0x0B)

/** @brief Adaptive boost mode and AGC1 time-A configuration register. */
#define AW87559_REG_ADPMODE (0x0C)

/** @brief Adaptive boost time1 / time2 configuration register. */
#define AW87559_REG_ADPBST_TIME1 (0x0D)

/** @brief Adaptive boost voltage threshold and low-step configuration register. */
#define AW87559_REG_ADPBST_VTH (0x0F)

/* ================= SYSCTRL 0x01 Bit Definitions ================= */

/** @brief Bit position for software enable (EN_SW) in SYSCTRL register. */
#define AW87559_SYS_EN_SW_BIT 6

/** @brief Bit position for boost converter enable (EN_BOOST) in SYSCTRL register. */
#define AW87559_SYS_EN_BOOST_BIT 4

/** @brief Bit position for PA enable (EN_PA) in SYSCTRL register. */
#define AW87559_SYS_EN_PA_BIT 3

/** @brief Bit position for receiver mode (RCV_MODE) in SYSCTRL register. */
#define AW87559_SYS_RCV_MODE_BIT 2

/** @brief Bit position for high-voltage battery enable (EN_HVBAT) in SYSCTRL register. */
#define AW87559_SYS_EN_HVBAT_BIT 0

/** @brief Bitmask for software enable. */
#define AW87559_SYS_EN_SW_MASK (1 << AW87559_SYS_EN_SW_BIT)

/** @brief Bitmask for boost converter enable. */
#define AW87559_SYS_EN_BOOST_MASK (1 << AW87559_SYS_EN_BOOST_BIT)

/** @brief Bitmask for PA enable. */
#define AW87559_SYS_EN_PA_MASK (1 << AW87559_SYS_EN_PA_BIT)

/** @brief Bitmask for receiver mode. */
#define AW87559_SYS_RCV_MODE_MASK (1 << AW87559_SYS_RCV_MODE_BIT)

/** @brief Bitmask for high-voltage battery enable. */
#define AW87559_SYS_EN_HVBAT_MASK (1 << AW87559_SYS_EN_HVBAT_BIT)

/* ================= BATSAFE 0x02 Bit Definitions ================= */

/** @brief Bit position for battery safeguard enable in BATSAFE register. */
#define AW87559_BAT_EN_SFGD_BIT 1

/** @brief Bitmask for battery safeguard enable. */
#define AW87559_BAT_EN_SFGD_MASK (1 << AW87559_BAT_EN_SFGD_BIT)

/* ================= BOOST VOLTAGE 0x03 Bit Definitions ================= */

/** @brief Bitmask for boost output voltage setting (lower 4 bits). */
#define AW87559_BST_VOUT_MASK 0x0F

/* ================= BOOST PEAK CURRENT 0x05 Bit Definitions ================= */

/** @brief Bitmask for boost converter peak current limit (lower 4 bits). */
#define AW87559_BST_IPEAK_MASK 0x0F

/** @brief Bit shift for boost converter peak current limit field. */
#define AW87559_BST_IPEAK_SHIFT 0

/* ================= PA GAIN 0x06 Bit Definitions ================= */

/** @brief Bitmask for PA gain setting (lower 5 bits, 0 ~ 18 valid). */
#define AW87559_PAGR_GAIN_MASK 0x1F

/* ================= AGC3 POWER 0x07 Bit Definitions ================= */

/** @brief Bitmask for AGC3 output power limit (lower 4 bits). */
#define AW87559_AGC3_OUTPUT_POWER_MASK 0x0F

/** @brief Bitmask for AGC3 power-down control. */
#define AW87559_AGC3_PD_MASK 0x10

/** @brief Bit shift for AGC3 power-down control. */
#define AW87559_AGC3_PD_SHIFT 4

/* ================= AGC3 TIME 0x08 Bit Definitions ================= */

/** @brief Bitmask for AGC3 release time (bits [7:5]). */
#define AW87559_AGC3_REL_MASK 0xE0

/** @brief Bit shift for AGC3 release time field. */
#define AW87559_AGC3_REL_SHIFT 5

/** @brief Bitmask for AGC3 attack time (bits [4:2]). */
#define AW87559_AGC3_ATT_MASK 0x1C

/** @brief Bit shift for AGC3 attack time field. */
#define AW87559_AGC3_ATT_SHIFT 2

/* ================= AGC2 POWER 0x09 Bit Definitions ================= */

/** @brief Bitmask for AGC2 output power limit (lower 4 bits). */
#define AW87559_AGC2_OUTPUT_MASK 0x0F

/* ================= AGC2 TIME 0x0A Bit Definitions ================= */

/** @brief Bitmask for AGC2 attack time (bits [4:2]). */
#define AW87559_AGC2_ATT_MASK 0x1C

/** @brief Bit shift for AGC2 attack time field. */
#define AW87559_AGC2_ATT_SHIFT 2

/* ================= AGC1 0x0B Bit Definitions ================= */

/** @brief Bitmask for AGC1 attack time (bits [2:1]). */
#define AW87559_AGC1_ATT_MASK 0x06

/** @brief Bit shift for AGC1 attack time field. */
#define AW87559_AGC1_ATT_SHIFT 1

/** @brief Bitmask for AGC1 power-down control (bit 0). */
#define AW87559_AGC1_PD_MASK 0x01

/* ================= ADP MODE 0x0C Bit Definitions ================= */

/** @brief Bitmask for adaptive boost mode (bits [2:0]). */
#define AW87559_ADPBOOST_MODE_MASK 0x07

/** @brief Bitmask for AGC1 time-A enable (bit 4). */
#define AW87559_AGC1_ATT_TIMEA_MASK 0x10

/* ================= ADP TIME 0x0D Bit Definitions ================= */

/** @brief Bitmask for adaptive boost time2 (bits [7:4]). */
#define AW87559_ADP_BST_TIME2_MASK 0xF0

/** @brief Bit shift for adaptive boost time2 field. */
#define AW87559_ADP_BST_TIME2_SHIFT 4

/** @brief Bitmask for adaptive boost time1 (bits [3:0]). */
#define AW87559_ADP_BST_TIME1_MASK 0x0F

/* ================= ADP BOOST VTH 0x0F Bit Definitions ================= */

/** @brief Bitmask for adaptive low-step voltage setting (bits [7:6]). */
#define AW87559_ADP_LOW_STEP_MASK 0xC0

/** @brief Bit shift for adaptive low-step voltage field. */
#define AW87559_ADP_LOW_STEP_SHIFT 6

/** @brief Bitmask for boost voltage threshold 2 (bits [5:3]). */
#define AW87559_SET_BOOST_VTH2_MASK 0x38

/** @brief Bit shift for boost voltage threshold 2 field. */
#define AW87559_SET_BOOST_VTH2_SHIFT 3

/** @brief Bitmask for boost voltage threshold 1 (bits [2:0]). */
#define AW87559_SET_BOOST_VTH1_MASK 0x07

/** @brief Bit shift for boost voltage threshold 1 field. */
#define AW87559_SET_BOOST_VTH1_SHIFT 0

/* ================= Enumerations ================= */

/**
 * @brief Boost converter output voltage settings.
 *
 * Configures the DC-DC boost converter output voltage via register 0x03.
 * Range: 6.5V to 9.5V in 0.25V steps. Default is 9.5V (0x0C).
 *
 * @note Higher boost voltage enables louder output but increases power consumption.
 */
typedef enum {
    AW87559_BST_6_5V = 0x00, /**< Boost output 6.50V */
    AW87559_BST_6_75V,       /**< Boost output 6.75V */
    AW87559_BST_7_0V,        /**< Boost output 7.00V */
    AW87559_BST_7_25V,       /**< Boost output 7.25V */
    AW87559_BST_7_5V,        /**< Boost output 7.50V */
    AW87559_BST_7_75V,       /**< Boost output 7.75V */
    AW87559_BST_8_0V,        /**< Boost output 8.00V */
    AW87559_BST_8_25V,       /**< Boost output 8.25V */
    AW87559_BST_8_5V,        /**< Boost output 8.50V */
    AW87559_BST_8_75V,       /**< Boost output 8.75V */
    AW87559_BST_9_0V,        /**< Boost output 9.00V */
    AW87559_BST_9_25V,       /**< Boost output 9.25V */
    AW87559_BST_9_5V,        /**< Boost output 9.50V (default) */
} aw87559_bst_voltage_t;

/**
 * @brief PA (Power Amplifier) gain settings.
 *
 * Configures the PA gain via register 0x06.
 * Range: 0 dB to 27 dB in 1.5 dB steps.
 *
 * @note Gain values above 24 dB may cause clipping with high-level input signals.
 */
typedef enum {
    AW87559_GAIN_0DB = 0, /**< PA gain  0.0 dB */
    AW87559_GAIN_1_5DB,   /**< PA gain  1.5 dB */
    AW87559_GAIN_3DB,     /**< PA gain  3.0 dB */
    AW87559_GAIN_4_5DB,   /**< PA gain  4.5 dB */
    AW87559_GAIN_6DB,     /**< PA gain  6.0 dB */
    AW87559_GAIN_7_5DB,   /**< PA gain  7.5 dB */
    AW87559_GAIN_9DB,     /**< PA gain  9.0 dB */
    AW87559_GAIN_10_5DB,  /**< PA gain 10.5 dB */
    AW87559_GAIN_12DB,    /**< PA gain 12.0 dB */
    AW87559_GAIN_13_5DB,  /**< PA gain 13.5 dB */
    AW87559_GAIN_15DB,    /**< PA gain 15.0 dB */
    AW87559_GAIN_16_5DB,  /**< PA gain 16.5 dB */
    AW87559_GAIN_18DB,    /**< PA gain 18.0 dB */
    AW87559_GAIN_19_5DB,  /**< PA gain 19.5 dB */
    AW87559_GAIN_21DB,    /**< PA gain 21.0 dB */
    AW87559_GAIN_22_5DB,  /**< PA gain 22.5 dB */
    AW87559_GAIN_24DB,    /**< PA gain 24.0 dB */
    AW87559_GAIN_25_5DB,  /**< PA gain 25.5 dB */
    AW87559_GAIN_27DB,    /**< PA gain 27.0 dB */
} aw87559_gain_t;

/**
 * @brief Boost converter inductor peak current limit settings.
 *
 * Configures the maximum peak current through the boost inductor via register 0x05.
 * Higher peak current allows higher output power but increases EMI and power loss.
 * Default is 3.5A.
 */
typedef enum {
    AW87559_BST_IPEAK_2_0A  = 0x02, /**< Peak current limit 2.00A */
    AW87559_BST_IPEAK_2_25A = 0x03, /**< Peak current limit 2.25A */
    AW87559_BST_IPEAK_2_5A  = 0x04, /**< Peak current limit 2.50A */
    AW87559_BST_IPEAK_2_75A = 0x05, /**< Peak current limit 2.75A */
    AW87559_BST_IPEAK_3_0A  = 0x06, /**< Peak current limit 3.00A */
    AW87559_BST_IPEAK_3_25A = 0x07, /**< Peak current limit 3.25A */
    AW87559_BST_IPEAK_3_5A  = 0x08, /**< Peak current limit 3.50A (default) */
    AW87559_BST_IPEAK_3_75A = 0x09, /**< Peak current limit 3.75A */
    AW87559_BST_IPEAK_4_0A  = 0x0A, /**< Peak current limit 4.00A */
    AW87559_BST_IPEAK_4_25A = 0x0B, /**< Peak current limit 4.25A */
    AW87559_BST_IPEAK_4_5A  = 0x0C, /**< Peak current limit 4.50A */
} aw87559_bst_ipeak_t;

/**
 * @brief AGC3 (stage 3) output power limit settings.
 *
 * AGC3 is the final stage of the three-stage AGC system. It limits the maximum
 * output power delivered to the speaker to prevent damage.
 * Range: 0.5W to 2.0W in 0.1W steps.
 */
typedef enum {
    AW87559_AGC3_0_5W = 0, /**< AGC3 limit 0.5W */
    AW87559_AGC3_0_6W,     /**< AGC3 limit 0.6W */
    AW87559_AGC3_0_7W,     /**< AGC3 limit 0.7W */
    AW87559_AGC3_0_8W,     /**< AGC3 limit 0.8W */
    AW87559_AGC3_0_9W,     /**< AGC3 limit 0.9W */
    AW87559_AGC3_1_0W,     /**< AGC3 limit 1.0W */
    AW87559_AGC3_1_1W,     /**< AGC3 limit 1.1W */
    AW87559_AGC3_1_2W,     /**< AGC3 limit 1.2W */
    AW87559_AGC3_1_3W,     /**< AGC3 limit 1.3W */
    AW87559_AGC3_1_4W,     /**< AGC3 limit 1.4W */
    AW87559_AGC3_1_5W,     /**< AGC3 limit 1.5W */
    AW87559_AGC3_1_6W,     /**< AGC3 limit 1.6W */
    AW87559_AGC3_1_7W,     /**< AGC3 limit 1.7W */
    AW87559_AGC3_1_8W,     /**< AGC3 limit 1.8W */
    AW87559_AGC3_1_9W,     /**< AGC3 limit 1.9W */
    AW87559_AGC3_2_0W,     /**< AGC3 limit 2.0W */
} aw87559_agc3_power_t;

/**
 * @brief AGC3 release time settings.
 *
 * Defines how quickly the AGC3 gain recovers after the signal drops below
 * the threshold. Longer release times provide smoother gain recovery.
 */
typedef enum {
    AW87559_AGC3_REL_5MS,   /**< AGC3 release time   5 ms */
    AW87559_AGC3_REL_10MS,  /**< AGC3 release time  10 ms */
    AW87559_AGC3_REL_20MS,  /**< AGC3 release time  20 ms */
    AW87559_AGC3_REL_40MS,  /**< AGC3 release time  40 ms */
    AW87559_AGC3_REL_82MS,  /**< AGC3 release time  82 ms */
    AW87559_AGC3_REL_163MS, /**< AGC3 release time 163 ms */
    AW87559_AGC3_REL_327MS, /**< AGC3 release time 327 ms */
    AW87559_AGC3_REL_655MS, /**< AGC3 release time 655 ms */
} aw87559_agc3_release_t;

/**
 * @brief AGC3 attack time settings.
 *
 * Defines how quickly the AGC3 responds to signal peaks exceeding the threshold.
 * Shorter attack times provide faster protection but may cause audible artifacts.
 */
typedef enum {
    AW87559_AGC3_ATT_1_28MS, /**< AGC3 attack time   1.28 ms */
    AW87559_AGC3_ATT_2_56MS, /**< AGC3 attack time   2.56 ms */
    AW87559_AGC3_ATT_10MS,   /**< AGC3 attack time  10.00 ms */
    AW87559_AGC3_ATT_40MS,   /**< AGC3 attack time  40.00 ms */
    AW87559_AGC3_ATT_82MS,   /**< AGC3 attack time  82.00 ms */
    AW87559_AGC3_ATT_164MS,  /**< AGC3 attack time 164.00 ms */
    AW87559_AGC3_ATT_328MS,  /**< AGC3 attack time 328.00 ms */
    AW87559_AGC3_ATT_656MS,  /**< AGC3 attack time 656.00 ms */
} aw87559_agc3_attack_t;

/**
 * @brief AGC2 (stage 2) output power limit settings.
 *
 * AGC2 provides an intermediate power limiting stage between AGC1 and AGC3.
 * Range: 1.0W to 3.0W in 0.2W steps. Can be disabled by setting to AGC2_OFF.
 */
typedef enum {
    AW87559_AGC2_1_0W,      /**< AGC2 limit 1.0W */
    AW87559_AGC2_1_2W,      /**< AGC2 limit 1.2W */
    AW87559_AGC2_1_4W,      /**< AGC2 limit 1.4W */
    AW87559_AGC2_1_6W,      /**< AGC2 limit 1.6W */
    AW87559_AGC2_1_8W,      /**< AGC2 limit 1.8W */
    AW87559_AGC2_2_0W,      /**< AGC2 limit 2.0W */
    AW87559_AGC2_2_2W,      /**< AGC2 limit 2.2W */
    AW87559_AGC2_2_4W,      /**< AGC2 limit 2.4W */
    AW87559_AGC2_2_6W,      /**< AGC2 limit 2.6W */
    AW87559_AGC2_2_8W,      /**< AGC2 limit 2.8W */
    AW87559_AGC2_3_0W,      /**< AGC2 limit 3.0W */
    AW87559_AGC2_OFF = 0x0B /**< AGC2 disabled */
} aw87559_agc2_power_t;

/**
 * @brief AGC2 attack time settings.
 *
 * Defines how quickly AGC2 responds to signal peaks exceeding its threshold.
 */
typedef enum {
    AW87559_AGC2_ATT_0_16MS,  /**< AGC2 attack time   0.16 ms */
    AW87559_AGC2_ATT_0_32MS,  /**< AGC2 attack time   0.32 ms */
    AW87559_AGC2_ATT_0_64MS,  /**< AGC2 attack time   0.64 ms */
    AW87559_AGC2_ATT_2_56MS,  /**< AGC2 attack time   2.56 ms */
    AW87559_AGC2_ATT_10_24MS, /**< AGC2 attack time  10.24 ms */
    AW87559_AGC2_ATT_40_96MS, /**< AGC2 attack time  40.96 ms */
    AW87559_AGC2_ATT_82MS,    /**< AGC2 attack time  82.00 ms */
    AW87559_AGC2_ATT_164MS,   /**< AGC2 attack time 164.00 ms */
} aw87559_agc2_attack_t;

/**
 * @brief AGC1 (stage 1) attack time settings.
 *
 * AGC1 is the first stage of gain control, providing the fastest response
 * to transient peaks. It operates before AGC2 and AGC3 in the signal chain.
 */
typedef enum {
    AW87559_AGC1_ATT_0_04MS, /**< AGC1 attack time 0.04 ms */
    AW87559_AGC1_ATT_0_08MS, /**< AGC1 attack time 0.08 ms */
    AW87559_AGC1_ATT_0_16MS, /**< AGC1 attack time 0.16 ms */
    AW87559_AGC1_ATT_0_32MS, /**< AGC1 attack time 0.32 ms */
} aw87559_agc1_attack_t;

/**
 * @brief Adaptive boost converter operating mode.
 *
 * Controls how the boost converter adapts its output voltage based on
 * the audio signal level to optimize power efficiency.
 *
 * - Pass-through: Boost is bypassed, VBAT is fed directly to the PA.
 * - Force boost:  Boost is always active at the configured voltage.
 * - Mode 1/2:     Boost voltage adapts dynamically based on signal level.
 */
typedef enum {
    AW87559_ADP_PASS_THROUGH = 0, /**< Pass-through mode (boost bypassed) */
    AW87559_ADP_FORCE_BOOST  = 1, /**< Force boost mode (always boosted) */
    AW87559_ADP_BOOST_MD1    = 2, /**< Adaptive boost mode 1 */
    AW87559_ADP_BOOST_MD2    = 3, /**< Adaptive boost mode 2 */
} aw87559_adp_mode_t;

/**
 * @brief Adaptive boost transition time settings.
 *
 * Configures how quickly the boost voltage transitions between levels
 * in adaptive boost mode. Applies to both time1 (ramp-up) and time2 (ramp-down).
 */
typedef enum {
    AW87559_ADP_TIME_20MS  = 1,  /**< Transition time  20 ms */
    AW87559_ADP_TIME_30MS  = 3,  /**< Transition time  30 ms */
    AW87559_ADP_TIME_40MS  = 4,  /**< Transition time  40 ms */
    AW87559_ADP_TIME_65MS  = 6,  /**< Transition time  65 ms */
    AW87559_ADP_TIME_80MS  = 8,  /**< Transition time  80 ms */
    AW87559_ADP_TIME_100MS = 10, /**< Transition time 100 ms */
    AW87559_ADP_TIME_120MS = 11, /**< Transition time 120 ms */
    AW87559_ADP_TIME_140MS = 12, /**< Transition time 140 ms */
    AW87559_ADP_TIME_160MS = 13, /**< Transition time 160 ms */
    AW87559_ADP_TIME_320MS = 14, /**< Transition time 320 ms */
    AW87559_ADP_TIME_480MS = 15, /**< Transition time 480 ms */
} aw87559_adp_time_t;

/**
 * @brief Adaptive boost low-step voltage settings.
 *
 * In adaptive mode, when the signal level is low, the boost converter
 * reduces its output to this voltage to save power.
 */
typedef enum {
    AW87559_ADP_LOW_STEP_6_5V = 0, /**< Low-step voltage 6.50V */
    AW87559_ADP_LOW_STEP_6_75V,    /**< Low-step voltage 6.75V */
    AW87559_ADP_LOW_STEP_7_0V,     /**< Low-step voltage 7.00V */
    AW87559_ADP_LOW_STEP_7_25V,    /**< Low-step voltage 7.25V */
} aw87559_adp_low_step_t;

/**
 * @brief Boost voltage threshold 1 settings for adaptive mode.
 *
 * Defines the lower power threshold at which the adaptive boost begins
 * increasing the boost voltage from the low-step level.
 */
typedef enum {
    AW87559_BOOST_VTH1_0_1W = 0, /**< Threshold 1 at 0.1W */
    AW87559_BOOST_VTH1_0_2W,     /**< Threshold 1 at 0.2W */
    AW87559_BOOST_VTH1_0_3W,     /**< Threshold 1 at 0.3W */
    AW87559_BOOST_VTH1_0_4W,     /**< Threshold 1 at 0.4W */
} aw87559_boost_vth1_t;

/**
 * @brief Boost voltage threshold 2 settings for adaptive mode.
 *
 * Defines the upper power threshold at which the adaptive boost reaches
 * the full configured boost voltage.
 */
typedef enum {
    AW87559_BOOST_VTH2_1_2W = 0, /**< Threshold 2 at 1.2W */
    AW87559_BOOST_VTH2_1_4W,     /**< Threshold 2 at 1.4W */
    AW87559_BOOST_VTH2_1_6W,     /**< Threshold 2 at 1.6W */
    AW87559_BOOST_VTH2_1_8W,     /**< Threshold 2 at 1.8W */
    AW87559_BOOST_VTH2_2_0W,     /**< Threshold 2 at 2.0W */
    AW87559_BOOST_VTH2_2_2W,     /**< Threshold 2 at 2.2W */
} aw87559_boost_vth2_t;

/**
 * @class AW87559
 * @brief Driver class for the AW87559 Smart Audio Amplifier.
 *
 * Provides a complete interface to configure the AW87559 Class-K PA including:
 * - System enable/disable control
 * - Boost converter voltage and peak current configuration
 * - PA gain adjustment (0 dB ~ 27 dB)
 * - Three-stage AGC (AGC1/AGC2/AGC3) configuration
 * - Adaptive boost mode for power optimization
 * - Battery under-voltage protection
 *
 * @par Typical Usage
 * @code
 * I2CBus bus;
 * bus.begin(I2C_NUM_0, SDA_PIN, SCL_PIN);
 *
 * AW87559 pa(bus);
 * pa.begin();                             // Initialize with default settings
 * pa.setPAGain(AW87559_GAIN_6DB);         // Set PA gain to 6 dB
 * pa.setBoostVoltage(AW87559_BST_8_0V);   // Set boost output to 8.0V
 * pa.enableAGC3(true);                    // Enable AGC3 speaker protection
 * pa.setAGC3Power(AW87559_AGC3_1_5W);    // Limit output to 1.5W
 * @endcode
 *
 * @par Signal Chain
 * @verbatim
 *  Audio Input → [AGC1] → [AGC2] → [AGC3] → [PA (Class-K)] → Speaker
 *                                                ↑
 *                                     [Adaptive Boost Converter]
 * @endverbatim
 */
class AW87559 {
public:
    /**
     * @brief Construct a new AW87559 driver instance.
     *
     * @param[in] bus  Reference to an initialized I2CBus object.
     * @param[in] addr I2C 7-bit slave address (default: 0x5B).
     */
    AW87559(I2CBus &bus, uint8_t addr = AW87559_I2C_ADDR);

    /**
     * @brief Initialize the AW87559 device.
     *
     * Verifies communication by reading the chip ID register and applies
     * default power-on configuration:
     * - Software enable ON
     * - Boost enable ON
     * - PA enable ON
     *
     * @return true  Initialization successful, chip ID verified.
     * @return false Communication failed or unexpected chip ID.
     */
    bool begin();

    /* ========= SYSCTRL (0x01) ========= */

    /**
     * @brief Enable or disable the software master switch.
     *
     * When disabled, the entire device enters a low-power standby state.
     *
     * @param[in] en  true = enable, false = disable (standby).
     * @return true on successful I2C write, false on failure.
     */
    bool setSoftwareEnable(bool en);

    /**
     * @brief Enable or disable the boost DC-DC converter.
     *
     * @param[in] en  true = enable boost, false = disable boost.
     * @return true on successful I2C write, false on failure.
     */
    bool setBoostEnable(bool en);

    /**
     * @brief Enable or disable the power amplifier output stage.
     *
     * @param[in] en  true = enable PA, false = disable PA (mute).
     * @return true on successful I2C write, false on failure.
     */
    bool setPAEnable(bool en);

    /**
     * @brief Enable or disable receiver (earpiece) mode.
     *
     * In receiver mode, the PA operates at reduced power suitable for
     * driving an earpiece speaker close to the ear.
     *
     * @param[in] en  true = receiver mode, false = speaker mode.
     * @return true on successful I2C write, false on failure.
     */
    bool setReceiverMode(bool en);

    /**
     * @brief Enable or disable high-voltage battery detection.
     *
     * @param[in] en  true = enable HVBAT, false = disable HVBAT.
     * @return true on successful I2C write, false on failure.
     */
    bool setHVBATEnable(bool en);

    /* ========= Battery Protection (0x02) ========= */

    /**
     * @brief Enable or disable battery under-voltage safeguard.
     *
     * When enabled, the device automatically reduces output power or shuts
     * down when the battery voltage drops below a safe threshold.
     *
     * @param[in] en  true = enable protection, false = disable.
     * @return true on successful I2C write, false on failure.
     */
    bool enableBatteryProtection(bool en);

    /* ========= Boost Voltage (0x03) ========= */

    /**
     * @brief Set the boost converter output voltage.
     *
     * @param[in] voltage  Target voltage from @ref aw87559_bst_voltage_t (6.5V ~ 9.5V).
     * @return true on successful I2C write, false on failure.
     *
     * @see getBoostVoltage()
     */
    bool setBoostVoltage(aw87559_bst_voltage_t voltage);

    /**
     * @brief Get the current boost converter output voltage setting.
     *
     * @param[out] voltage  Current voltage setting read from the device.
     * @return true on successful I2C read, false on failure.
     *
     * @see setBoostVoltage()
     */
    bool getBoostVoltage(aw87559_bst_voltage_t &voltage);

    /* ========= Boost Peak Current (0x05) ========= */

    /**
     * @brief Set the boost converter inductor peak current limit.
     *
     * @param[in] ipeak  Peak current limit from @ref aw87559_bst_ipeak_t (2.0A ~ 4.5A).
     * @return true on successful I2C write, false on failure.
     *
     * @see getBoostIPeak()
     */
    bool setBoostIPeak(aw87559_bst_ipeak_t ipeak);

    /**
     * @brief Get the current boost converter peak current limit setting.
     *
     * @param[out] ipeak  Current peak current setting read from the device.
     * @return true on successful I2C read, false on failure.
     *
     * @see setBoostIPeak()
     */
    bool getBoostIPeak(aw87559_bst_ipeak_t &ipeak);

    /* ========= PA Gain (0x06) ========= */

    /**
     * @brief Set the PA gain.
     *
     * Adjusts the power amplifier gain in 1.5 dB steps from 0 dB to 27 dB.
     *
     * @param[in] gain  Target gain from @ref aw87559_gain_t.
     * @return true on successful I2C write, false on failure.
     *
     * @see getPAGain()
     */
    bool setPAGain(aw87559_gain_t gain);

    /**
     * @brief Get the current PA gain setting in dB.
     *
     * @return Current PA gain in dB (0.0 ~ 27.0), or -1.0 on read failure.
     *
     * @see setPAGain()
     */
    float getPAGain();

    /* ========= AGC3 Output Power (0x07) ========= */

    /**
     * @brief Set the AGC3 output power limit.
     *
     * AGC3 is the final AGC stage that directly limits the power delivered
     * to the speaker. This should be set according to the speaker's
     * maximum rated power to prevent damage.
     *
     * @param[in] power  Power limit from @ref aw87559_agc3_power_t (0.5W ~ 2.0W).
     * @return true on successful I2C write, false on failure.
     *
     * @see getAGC3Power(), enableAGC3()
     */
    bool setAGC3Power(aw87559_agc3_power_t power);

    /**
     * @brief Get the current AGC3 output power limit setting.
     *
     * @param[out] power  Current AGC3 power limit read from the device.
     * @return true on successful I2C read, false on failure.
     *
     * @see setAGC3Power()
     */
    bool getAGC3Power(aw87559_agc3_power_t &power);

    /**
     * @brief Enable or disable AGC3.
     *
     * @param[in] en  true = enable AGC3, false = disable (bypass).
     * @return true on successful I2C write, false on failure.
     *
     * @note Disabling AGC3 removes the output power limit protection.
     *       Use with caution to avoid speaker damage.
     */
    bool enableAGC3(bool en);

    /* ========= AGC3 Timing (0x08) ========= */

    /**
     * @brief Set the AGC3 release (recovery) time.
     *
     * Controls how quickly the gain recovers after the signal level drops
     * below the AGC3 threshold.
     *
     * @param[in] rel  Release time from @ref aw87559_agc3_release_t (5ms ~ 655ms).
     * @return true on successful I2C write, false on failure.
     */
    bool setAGC3ReleaseTime(aw87559_agc3_release_t rel);

    /**
     * @brief Set the AGC3 attack (response) time.
     *
     * Controls how quickly AGC3 reduces gain when the signal exceeds
     * the power threshold.
     *
     * @param[in] att  Attack time from @ref aw87559_agc3_attack_t (1.28ms ~ 656ms).
     * @return true on successful I2C write, false on failure.
     */
    bool setAGC3AttackTime(aw87559_agc3_attack_t att);

    /* ========= AGC2 Configuration (0x09) ========= */

    /**
     * @brief Set the AGC2 output power limit.
     *
     * AGC2 is the intermediate AGC stage. Setting to @ref AW87559_AGC2_OFF
     * disables this stage.
     *
     * @param[in] power  Power limit from @ref aw87559_agc2_power_t (1.0W ~ 3.0W or OFF).
     * @return true on successful I2C write, false on failure.
     */
    bool setAGC2Power(aw87559_agc2_power_t power);

    /* ========= AGC2 Timing (0x0A) ========= */

    /**
     * @brief Set the AGC2 attack time.
     *
     * @param[in] att  Attack time from @ref aw87559_agc2_attack_t (0.16ms ~ 164ms).
     * @return true on successful I2C write, false on failure.
     *
     * @see getAGC2AttackTime()
     */
    bool setAGC2AttackTime(aw87559_agc2_attack_t att);

    /**
     * @brief Get the current AGC2 attack time setting.
     *
     * @param[out] att  Current AGC2 attack time read from the device.
     * @return true on successful I2C read, false on failure.
     *
     * @see setAGC2AttackTime()
     */
    bool getAGC2AttackTime(aw87559_agc2_attack_t &att);

    /* ========= AGC1 Configuration (0x0B) ========= */

    /**
     * @brief Set the AGC1 attack time.
     *
     * AGC1 provides the fastest transient peak limiting. It is the first
     * stage in the AGC chain.
     *
     * @param[in] att  Attack time from @ref aw87559_agc1_attack_t (0.04ms ~ 0.32ms).
     * @return true on successful I2C write, false on failure.
     *
     * @see getAGC1AttackTime()
     */
    bool setAGC1AttackTime(aw87559_agc1_attack_t att);

    /**
     * @brief Get the current AGC1 attack time setting.
     *
     * @param[out] att  Current AGC1 attack time read from the device.
     * @return true on successful I2C read, false on failure.
     *
     * @see setAGC1AttackTime()
     */
    bool getAGC1AttackTime(aw87559_agc1_attack_t &att);

    /**
     * @brief Enable or disable AGC1.
     *
     * @param[in] en  true = enable AGC1, false = disable (bypass).
     * @return true on successful I2C write, false on failure.
     */
    bool enableAGC1(bool en);

    /* ========= Adaptive Boost Mode (0x0C) ========= */

    /**
     * @brief Set the adaptive boost operating mode.
     *
     * In adaptive mode, the boost voltage dynamically adjusts based on the
     * audio signal level, reducing power consumption during quiet passages.
     *
     * @param[in] mode  Operating mode from @ref aw87559_adp_mode_t.
     * @return true on successful I2C write, false on failure.
     *
     * @see getADPMode()
     */
    bool setADPMode(aw87559_adp_mode_t mode);

    /**
     * @brief Get the current adaptive boost operating mode.
     *
     * @param[out] mode  Current mode read from the device.
     * @return true on successful I2C read, false on failure.
     *
     * @see setADPMode()
     */
    bool getADPMode(aw87559_adp_mode_t &mode);

    /**
     * @brief Enable or disable AGC1 time-A alternate timing mode.
     *
     * When enabled, AGC1 uses an alternate attack time profile that may
     * be better suited for certain audio content.
     *
     * @param[in] en  true = enable time-A, false = use default timing.
     * @return true on successful I2C write, false on failure.
     *
     * @see getAGC1TimeAEnabled()
     */
    bool setAGC1TimeAEnabled(bool en);

    /**
     * @brief Get the AGC1 time-A alternate timing mode state.
     *
     * @param[out] en  true if time-A is enabled, false otherwise.
     * @return true on successful I2C read, false on failure.
     *
     * @see setAGC1TimeAEnabled()
     */
    bool getAGC1TimeAEnabled(bool &en);

    /* ========= Adaptive Boost Timing (0x0D) ========= */

    /**
     * @brief Set adaptive boost transition time 1 (ramp-up time).
     *
     * Controls how quickly the boost voltage ramps up when the audio
     * signal level increases.
     *
     * @param[in] time  Transition time from @ref aw87559_adp_time_t.
     * @return true on successful I2C write, false on failure.
     *
     * @see getADPTime1()
     */
    bool setADPTime1(aw87559_adp_time_t time);

    /**
     * @brief Set adaptive boost transition time 2 (ramp-down time).
     *
     * Controls how quickly the boost voltage ramps down when the audio
     * signal level decreases.
     *
     * @param[in] time  Transition time from @ref aw87559_adp_time_t.
     * @return true on successful I2C write, false on failure.
     *
     * @see getADPTime2()
     */
    bool setADPTime2(aw87559_adp_time_t time);

    /**
     * @brief Get the current adaptive boost time 1 setting.
     *
     * @param[out] time  Current time1 value read from the device.
     * @return true on successful I2C read, false on failure.
     *
     * @see setADPTime1()
     */
    bool getADPTime1(aw87559_adp_time_t &time);

    /**
     * @brief Get the current adaptive boost time 2 setting.
     *
     * @param[out] time  Current time2 value read from the device.
     * @return true on successful I2C read, false on failure.
     *
     * @see setADPTime2()
     */
    bool getADPTime2(aw87559_adp_time_t &time);

    /* ========= Adaptive Boost Thresholds (0x0F) ========= */

    /**
     * @brief Set the adaptive boost low-step voltage.
     *
     * The boost output voltage in low-power mode (when audio signal is
     * below VTH1 threshold).
     *
     * @param[in] step  Low-step voltage from @ref aw87559_adp_low_step_t.
     * @return true on successful I2C write, false on failure.
     *
     * @see getADPLowStep()
     */
    bool setADPLowStep(aw87559_adp_low_step_t step);

    /**
     * @brief Set boost voltage threshold 1 (lower threshold).
     *
     * When the output power exceeds this threshold, the boost converter
     * begins ramping up from the low-step voltage toward full boost.
     *
     * @param[in] vth1  Threshold from @ref aw87559_boost_vth1_t (0.1W ~ 0.4W).
     * @return true on successful I2C write, false on failure.
     *
     * @see getBoostVTH1()
     */
    bool setBoostVTH1(aw87559_boost_vth1_t vth1);

    /**
     * @brief Set boost voltage threshold 2 (upper threshold).
     *
     * When the output power exceeds this threshold, the boost converter
     * operates at full configured voltage.
     *
     * @param[in] vth2  Threshold from @ref aw87559_boost_vth2_t (1.2W ~ 2.2W).
     * @return true on successful I2C write, false on failure.
     *
     * @see getBoostVTH2()
     */
    bool setBoostVTH2(aw87559_boost_vth2_t vth2);

    /**
     * @brief Get the current adaptive boost low-step voltage setting.
     *
     * @param[out] step  Current low-step voltage read from the device.
     * @return true on successful I2C read, false on failure.
     *
     * @see setADPLowStep()
     */
    bool getADPLowStep(aw87559_adp_low_step_t &step);

    /**
     * @brief Get the current boost voltage threshold 1 setting.
     *
     * @param[out] vth1  Current VTH1 value read from the device.
     * @return true on successful I2C read, false on failure.
     *
     * @see setBoostVTH1()
     */
    bool getBoostVTH1(aw87559_boost_vth1_t &vth1);

    /**
     * @brief Get the current boost voltage threshold 2 setting.
     *
     * @param[out] vth2  Current VTH2 value read from the device.
     * @return true on successful I2C read, false on failure.
     *
     * @see setBoostVTH2()
     */
    bool getBoostVTH2(aw87559_boost_vth2_t &vth2);

private:
    I2CBus &_bus;  /**< Reference to the I2C bus instance. */
    uint8_t _addr; /**< I2C slave address of this device. */

    /**
     * @brief Write a single byte to a register.
     *
     * @param[in] reg  Register address.
     * @param[in] val  Value to write.
     * @return true on success, false on I2C error.
     */
    bool writeReg(uint8_t reg, uint8_t val);

    /**
     * @brief Read a single byte from a register.
     *
     * @param[in]  reg  Register address.
     * @param[out] val  Value read from the register.
     * @return true on success, false on I2C error.
     */
    bool readReg(uint8_t reg, uint8_t &val);

    /**
     * @brief Read-modify-write specific bits in a register.
     *
     * Performs an atomic read-modify-write operation:
     * `new_val = (old_val & ~mask) | (value & mask)`
     *
     * @param[in] reg    Register address.
     * @param[in] mask   Bitmask indicating which bits to modify.
     * @param[in] value  New value for the masked bits (pre-shifted).
     * @return true on success, false on I2C error.
     */
    bool updateBits(uint8_t reg, uint8_t mask, uint8_t value);
};

#endif