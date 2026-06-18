#include "sensor_task.hpp"
#include "app_config.hpp"
#include "app_context.hpp"
#include "messages.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

namespace App
{
    static const char *TAG = "SENSOR";

    static uint16_t median_u16(uint16_t *v, uint8_t n)
    {
        for (uint8_t i = 0; i < n - 1; i++)
        {
            for (uint8_t j = i + 1; j < n; j++)
            {
                if (v[j] < v[i])
                {
                    uint16_t tmp = v[i];
                    v[i] = v[j];
                    v[j] = tmp;
                }
            }
        }
        return v[n / 2];
    }

    static uint8_t target_from_ldr(uint16_t filtered)
    {
        if (filtered >= AppConfig::LDR_THRESHOLD_HIGH)
            return AppConfig::SERVO_ANGLE_LIGHT;

        if (filtered <= AppConfig::LDR_THRESHOLD_LOW)
            return AppConfig::SERVO_ANGLE_DARK;

        return (filtered > ((AppConfig::LDR_THRESHOLD_LOW + AppConfig::LDR_THRESHOLD_HIGH) / 2))
               ? AppConfig::SERVO_ANGLE_LIGHT
               : AppConfig::SERVO_ANGLE_DARK;
    }

    void SensorTask::run(void *pvParameters)
    {
        auto *cfg = static_cast<SensorTaskConfig *>(pvParameters);

        adc_oneshot_unit_handle_t adc_handle;

        adc_oneshot_unit_init_cfg_t unit_cfg = {};
        unit_cfg.unit_id = cfg->unit_id;
        unit_cfg.ulp_mode = ADC_ULP_MODE_DISABLE;
        unit_cfg.clk_src = ADC_RTC_CLK_SRC_DEFAULT;
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

        adc_oneshot_chan_cfg_t chan_cfg = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_12
        };
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, cfg->channel, &chan_cfg));

        ESP_LOGI(TAG, "%s iniciado", cfg->name);

        uint8_t last_state = 255;

        while (true)
        {
            uint16_t samples[AppConfig::FILTER_WINDOW_SIZE];

            for (uint8_t i = 0; i < cfg->filter_window; i++)
            {
                int raw_adc = 0;
                adc_oneshot_read(adc_handle, cfg->channel, &raw_adc);
                samples[i] = (uint16_t)raw_adc;
                vTaskDelay(pdMS_TO_TICKS(5));
            }

            uint16_t filtered = median_u16(samples, cfg->filter_window);
            uint8_t state = target_from_ldr(filtered);

            // SOLO evento cuando cambia realmente el estado
            if (state != last_state)
            {
                last_state = state;

                SensorMsg msg;
                msg.raw = samples[cfg->filter_window / 2];
                msg.filtered = filtered;
                msg.target_angle = state;
                msg.tick = xTaskGetTickCount();

                xQueueOverwrite(g_queues.sensor, &msg);

                ESP_LOGI(TAG, "EVENTO SENSOR: %u", state);
            }

            vTaskDelay(pdMS_TO_TICKS(cfg->period_ms));
        }
    }
}
