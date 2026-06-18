#include "button_task.hpp"
#include "app_context.hpp"
#include "messages.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

namespace App
{
    static const char *TAG = "BUTTON";

    struct Debounce
    {
        bool last_raw;
        bool stable;
        uint8_t count;

        Debounce()
            : last_raw(false),
            stable(false),
            count(0)
        {
        }
    };

    static bool debounce(uint8_t gpio, Debounce &db, bool &level_pressed)
    {
    bool changed = false;

    bool raw = gpio_get_level(static_cast<gpio_num_t>(gpio));

    if (raw != db.last_raw)
    {
        db.last_raw = raw;
        db.count = 0;
    }
    else
    {
        if (db.count < 3)
        {
            db.count++;
        }

        if (db.count >= 3 && raw != db.stable)
        {
            db.stable = raw;
            changed = true;
        }
    }

    level_pressed = db.stable;

        return changed;
    }

    void ButtonTask::run(void *pvParameters)
    {
        auto *cfg = static_cast<ButtonTaskConfig *>(pvParameters);

        gpio_reset_pin(static_cast<gpio_num_t>(cfg->gpio));
        gpio_set_direction(static_cast<gpio_num_t>(cfg->gpio), GPIO_MODE_INPUT);
        gpio_set_pull_mode(static_cast<gpio_num_t>(cfg->gpio), GPIO_FLOATING);

        Debounce db;
        bool pressed = false;

        ESP_LOGI(TAG, "%s iniciado", cfg->name);

        while (true)
        {
            if (debounce(cfg->gpio, db, pressed))
            {
                ButtonMsg msg;

                msg.type = cfg->event_type;
                msg.pressed = pressed;
                msg.tick = xTaskGetTickCount();

                xQueueSend(g_queues.buttons, &msg, 0);

                ESP_LOGI(TAG,
                        "%s -> %s",
                        cfg->name,
                        pressed ? "PRESSED" : "RELEASED");
            }


            vTaskDelay(pdMS_TO_TICKS(cfg->poll_ms));
        }
    }
}
