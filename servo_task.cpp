/*
 * Tarea encargada de controlar el servomotor.
 * Recibe comandos desde una cola y mueve gradualmente
 * el servo hasta alcanzar el ángulo objetivo.
 * Cuando llega a la posición deseada informa al Task Manager.
 */

#include "servo_task.hpp"
#include "app_context.hpp"
#include "messages.hpp"
#include "app_config.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

namespace App
{
    static const char *TAG = "SERVO";
    /*
     * Convierte un ángulo en grados al duty cycle PWM
     * correspondiente para controlar el servomotor.
     */
    static uint32_t angle_to_duty(
        const ServoTaskConfig *cfg,
        uint8_t angle)
    {
        uint32_t max_duty =
            (1UL << cfg->resolution) - 1UL;

        uint32_t pulse_us =
            cfg->min_us +
            ((uint32_t)angle *
             (cfg->max_us - cfg->min_us)) / 180UL;

        return (pulse_us *
                max_duty *
                cfg->freq_hz) / 1000000UL;
    }

    /*
     * Actualiza la señal PWM del servo.
     */
    static void servo_write_angle(
        const ServoTaskConfig *cfg,
        uint8_t angle)
    {
        uint32_t duty =
            angle_to_duty(cfg, angle);

        ledc_set_duty(
            cfg->mode,
            cfg->channel,
            duty);

        ledc_update_duty(
            cfg->mode,
            cfg->channel);
    }

    void ServoTask::run(void *pvParameters)
    {
        auto *cfg =
            static_cast<ServoTaskConfig *>(pvParameters);

        // Configuración del temporizador PWM
        ledc_timer_config_t timer_cfg = {};

        timer_cfg.speed_mode = cfg->mode;
        timer_cfg.duty_resolution = cfg->resolution;
        timer_cfg.timer_num = cfg->timer;
        timer_cfg.freq_hz = cfg->freq_hz;
        timer_cfg.clk_cfg = LEDC_AUTO_CLK;

        ESP_ERROR_CHECK(
            ledc_timer_config(&timer_cfg));

        // Configuración del canal PWM
        ledc_channel_config_t ch_cfg = {};

        ch_cfg.gpio_num = cfg->gpio;
        ch_cfg.speed_mode = cfg->mode;
        ch_cfg.channel = cfg->channel;
        ch_cfg.intr_type = LEDC_INTR_DISABLE;
        ch_cfg.timer_sel = cfg->timer;
        ch_cfg.duty = 0;
        ch_cfg.hpoint = 0;

        ESP_ERROR_CHECK(
            ledc_channel_config(&ch_cfg));

        uint8_t current_angle =
            AppConfig::SERVO_ANGLE_LIGHT;

        uint8_t target_angle =
            current_angle;

        uint8_t tolerance = 2;
        uint16_t step_delay = 100;
        uint8_t step_deg = 2;

        servo_write_angle(cfg, current_angle);

        ServoCmd cmd;

        TickType_t last_move = 0;
        bool reached_sent = false;

        while (true)
        {
            // Recepción de comandos
            if (xQueueReceive(
                    g_queues.servo_cmd,
                    &cmd,
                    pdMS_TO_TICKS(20)) == pdTRUE)
            {
                target_angle = cmd.target_angle;
                tolerance = cmd.tolerance_deg;
                step_delay = cmd.step_delay_ms;
                step_deg = cmd.step_deg;

                reached_sent = false;
            }

            // Movimiento gradual del servo
            if ((xTaskGetTickCount() - last_move) >=
                pdMS_TO_TICKS(step_delay))
            {
                last_move = xTaskGetTickCount();

                if (current_angle < target_angle)
                {
                    current_angle += step_deg;
                }
                else if (current_angle > target_angle)
                {
                    current_angle -= step_deg;
                }

                servo_write_angle(cfg, current_angle);

                ESP_LOGI(TAG,
                         "Angle: %d Target: %d",
                         current_angle,
                         target_angle);
            }

            // Verifica llegada al objetivo
            if (!reached_sent &&
                abs((int)current_angle -
                    (int)target_angle) <= tolerance)
            {
                ServoStatusMsg msg;

                msg.status =
                    ServoStatusType::Reached;

                msg.current_angle =
                    current_angle;

                msg.target_angle =
                    target_angle;

                msg.tick =
                    xTaskGetTickCount();

                xQueueOverwrite(
                    g_queues.servo_status,
                    &msg);

                reached_sent = true;
            }
        }
    }
}
