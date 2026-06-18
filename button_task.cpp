/*
 * Tarea encargada de monitorear un botón físico.
 * Implementa un algoritmo de debounce para eliminar rebotes mecánicos
 * y genera eventos cuando detecta cambios estables en el estado del botón.
 * Los eventos se envían al Task Manager mediante una cola.
 */

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

    /*
     * Estructura utilizada para almacenar el estado del debounce.
     */
    struct Debounce
    {
        bool last_raw;     // Última lectura instantánea
        bool stable;       // Estado validado
        uint8_t count;     // Contador de estabilidad

        Debounce()
            : last_raw(false),
              stable(false),
              count(0)
        {
        }
    };

    /*
     * Función de debounce.
     * Verifica si el estado del botón se mantiene estable durante
     * varias lecturas consecutivas antes de aceptar el cambio.
     */
    static bool debounce(uint8_t gpio, Debounce &db, bool &level_pressed)
    {
        bool changed = false;

        // Lectura actual del pin
        bool raw = gpio_get_level(static_cast<gpio_num_t>(gpio));

        if (raw != db.last_raw)
        {
            // Reinicia contador si cambia la lectura
            db.last_raw = raw;
            db.count = 0;
        }
        else
        {
            if (db.count < 3)
            {
                db.count++;
            }

            // Cambio validado después de varias muestras iguales
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

        // Configuración del GPIO como entrada
        gpio_reset_pin(static_cast<gpio_num_t>(cfg->gpio));
        gpio_set_direction(static_cast<gpio_num_t>(cfg->gpio), GPIO_MODE_INPUT);
        gpio_set_pull_mode(static_cast<gpio_num_t>(cfg->gpio), GPIO_FLOATING);

        Debounce db;
        bool pressed = false;

        ESP_LOGI(TAG, "%s iniciado", cfg->name);

        while (true)
        {
            // Detecta cambios estables en el botón
            if (debounce(cfg->gpio, db, pressed))
            {
                ButtonMsg msg;

                msg.type = cfg->event_type;
                msg.pressed = pressed;
                msg.tick = xTaskGetTickCount();

                // Envía evento al Task Manager
                xQueueSend(g_queues.buttons, &msg, 0);

                ESP_LOGI(TAG,
                         "%s -> %s",
                         cfg->name,
                         pressed ? "PRESSED" : "RELEASED");
            }

            // Periodo de muestreo
            vTaskDelay(pdMS_TO_TICKS(cfg->poll_ms));
        }
    }
}
