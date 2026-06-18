/*
 * Tarea encargada del LED de estado (Ready LED).
 * Mientras el sistema se encuentra en reposo, el LED parpadea indicando
 * que está listo para iniciar una operación.
 */

#include "ready_led_task.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

namespace App
{
    void ReadyLedTask::run(void *pvParameters)
    {
        // Obtiene la configuración de la tarea
        auto *cfg = static_cast<ReadyLedTaskConfig *>(pvParameters);

        // Configura el pin del LED como salida
        gpio_reset_pin(static_cast<gpio_num_t>(cfg->gpio));
        gpio_set_direction(static_cast<gpio_num_t>(cfg->gpio), GPIO_MODE_OUTPUT);

        while (true)
        {
            // Enciende el LED
            gpio_set_level(static_cast<gpio_num_t>(cfg->gpio), 1);

            // Mantiene el LED encendido
            vTaskDelay(pdMS_TO_TICKS(cfg->on_ms));

            // Apaga el LED
            gpio_set_level(static_cast<gpio_num_t>(cfg->gpio), 0);

            // Mantiene el LED apagado
            vTaskDelay(pdMS_TO_TICKS(cfg->off_ms));
        }
    }
}
