#pragma once
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"

namespace AppConfig
{
    constexpr adc_unit_t LDR_ADC_UNIT = ADC_UNIT_1;
    constexpr adc_channel_t LDR_ADC_CHANNEL = ADC_CHANNEL_6; // GPIO34

    constexpr int SERVO_GPIO = 33;
    constexpr int SPEED_BUTTON_GPIO = 39;
    constexpr int START_BUTTON_GPIO = 36;
    constexpr int READY_LED_GPIO = 32;

    constexpr unsigned SENSOR_PERIOD_MS = 500;
    constexpr unsigned FILTER_WINDOW_SIZE = 5;
    constexpr unsigned LDR_THRESHOLD_LOW = 1400;
    constexpr unsigned LDR_THRESHOLD_HIGH = 2600;

    constexpr unsigned SERVO_MIN_US = 500;
    constexpr unsigned SERVO_MAX_US = 2400;
    constexpr unsigned SERVO_PWM_FREQ_HZ = 50;
    constexpr ledc_timer_bit_t SERVO_PWM_RES_BITS = LEDC_TIMER_16_BIT;
    constexpr ledc_timer_t SERVO_PWM_TIMER = LEDC_TIMER_0;
    constexpr ledc_channel_t SERVO_PWM_CHANNEL = LEDC_CHANNEL_0;
    constexpr ledc_mode_t SERVO_PWM_MODE = LEDC_LOW_SPEED_MODE;

    constexpr unsigned SERVO_ANGLE_DARK = 0;
    constexpr unsigned SERVO_ANGLE_LIGHT = 180;
    constexpr unsigned SERVO_TOLERANCE_DEG = 3;

    constexpr unsigned SERVO_STEP_DEG = 2;
    constexpr unsigned SERVO_DELAY_SLOW_MS = 112;
    constexpr unsigned SERVO_DELAY_FAST_MS = 56;

    constexpr unsigned BUTTON_POLL_MS = 30;
    constexpr unsigned READY_LED_ON_MS = 250;
    constexpr unsigned READY_LED_OFF_MS = 100;
    constexpr unsigned HOLD_TARGET_MS = 8000;

    constexpr unsigned BUTTON_QUEUE_LEN = 8;

    constexpr int SERVO_OFFSET_DEG = 10;
}
