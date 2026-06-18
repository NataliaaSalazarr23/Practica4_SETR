#include "task_manager.hpp"
#include "app_config.hpp"
#include "app_context.hpp"
#include "messages.hpp"
#include "sensor_task.hpp"
#include "button_task.hpp"
#include "servo_task.hpp"
#include "ready_led_task.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

namespace App
{
    static const char *TAG = "MANAGER";

    static SensorTaskConfig sensor_cfg{
        AppConfig::LDR_ADC_UNIT,
        AppConfig::LDR_ADC_CHANNEL,
        AppConfig::SENSOR_PERIOD_MS,
        AppConfig::FILTER_WINDOW_SIZE,
        "SensorTask"
    };

    static ServoTaskConfig servo_cfg{
        AppConfig::SERVO_GPIO,
        AppConfig::SERVO_PWM_CHANNEL,
        AppConfig::SERVO_PWM_TIMER,
        AppConfig::SERVO_PWM_MODE,
        AppConfig::SERVO_PWM_RES_BITS,
        AppConfig::SERVO_PWM_FREQ_HZ,
        AppConfig::SERVO_MIN_US,
        AppConfig::SERVO_MAX_US,
        "ServoTask"
    };

    static ButtonTaskConfig start_btn_cfg{
        AppConfig::START_BUTTON_GPIO,
        "StartButton",
        ButtonEventType::Start,
        AppConfig::BUTTON_POLL_MS,
        true
    };

    static ButtonTaskConfig speed_btn_cfg{
        AppConfig::SPEED_BUTTON_GPIO,
        "SpeedButton",
        ButtonEventType::SpeedState,
        AppConfig::BUTTON_POLL_MS,
        true
    };

    static ReadyLedTaskConfig ready_led_cfg{
        AppConfig::READY_LED_GPIO,
        AppConfig::READY_LED_ON_MS,
        AppConfig::READY_LED_OFF_MS,
        "ReadyLED"
    };

    static ManagerTaskConfig manager_cfg{
        AppConfig::HOLD_TARGET_MS,
        AppConfig::SERVO_TOLERANCE_DEG,
        AppConfig::SERVO_DELAY_SLOW_MS,
        AppConfig::SERVO_DELAY_FAST_MS,
        AppConfig::SERVO_STEP_DEG,
        "TaskManager"
    };

    static void send_servo_cmd(uint8_t target, bool fast, const ManagerTaskConfig *cfg)
    {
        ServoCmd cmd{
            target,
            cfg->tolerance_deg,
            fast ? cfg->fast_delay_ms : cfg->slow_delay_ms,
            cfg->step_deg
        };

        xQueueOverwrite(g_queues.servo_cmd, &cmd);
    }

    void TaskManager::run(void *pvParameters)
    {
        auto *cfg = static_cast<ManagerTaskConfig *>(pvParameters);

        bool fast_mode = false;
        bool operating = false;
        bool reached = false;

        bool target_locked = false;
        TickType_t hold_start_tick = 0;

        uint8_t current_target = 255;



        vTaskSuspend(g_handles.servo);
        vTaskSuspend(g_handles.speed_button);
        vTaskResume(g_handles.ready_led);

        while (true)
        {
            ButtonMsg btn_msg;
            SensorMsg sensor_msg;
            ServoStatusMsg servo_msg;

            if (xQueueReceive(g_queues.buttons, &btn_msg, 0) == pdTRUE)
            {
                if (btn_msg.type == ButtonEventType::Start && btn_msg.pressed && !operating)
                {
                    operating = true;
                    reached = false;
                    target_locked = false;

                    vTaskSuspend(g_handles.ready_led);
                    vTaskResume(g_handles.servo);
                    vTaskResume(g_handles.speed_button);

                    ESP_LOGI(TAG, "SYSTEM STARTED");
                }

                if (btn_msg.type == ButtonEventType::SpeedState)
                {
                    fast_mode = btn_msg.pressed;

                    if (operating)
                    {
                        send_servo_cmd(current_target, fast_mode, cfg);
                    }
            }
        }

            // ================= SENSOR =================
                    if (operating && !target_locked &&
                        xQueueReceive(g_queues.sensor, &sensor_msg, 0) == pdTRUE)
                    {
                        uint8_t new_target = sensor_msg.target_angle;

                        ESP_LOGI(TAG,
                        "Sensor=%u Current=%u Locked=%d",
                        new_target,
                        current_target,
                        target_locked);

                        if (new_target != current_target)
                        {
                            current_target = new_target;
                            target_locked = true;

                            send_servo_cmd(current_target, fast_mode, cfg);

                            ESP_LOGI(TAG, "TARGET LOCKED: %u", current_target);
                        }
                    }



                    // ================= SERVO STATUS ================= 
                    if (xQueueReceive(g_queues.servo_status, &servo_msg, 0) == pdTRUE)
                    {
                        if (servo_msg.status == ServoStatusType::Reached)
                        {
                            ESP_LOGI(TAG, "REACHED");

                            reached = true;
                            hold_start_tick = xTaskGetTickCount();
                        }
                    }



            if (operating && reached)
            {
                if ((xTaskGetTickCount() - hold_start_tick) >= pdMS_TO_TICKS(cfg->hold_target_ms))
                {
                    operating = false;
                    reached = false;
                    target_locked = false;

                    xQueueReset(g_queues.sensor);
                    xQueueReset(g_queues.servo_status);
                    xQueueReset(g_queues.servo_cmd);

                    current_target = 255;
                    fast_mode = false;


                    vTaskSuspend(g_handles.servo);
                    vTaskSuspend(g_handles.speed_button);

                    vTaskResume(g_handles.ready_led);

                    ESP_LOGI(TAG, "SYSTEM IDLE");
                }
            }

            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    void app_tasks_create()
    {
        g_queues.sensor = xQueueCreate(1, sizeof(SensorMsg));
        g_queues.buttons = xQueueCreate(AppConfig::BUTTON_QUEUE_LEN, sizeof(ButtonMsg));
        g_queues.servo_cmd = xQueueCreate(1, sizeof(ServoCmd));
        g_queues.servo_status = xQueueCreate(1, sizeof(ServoStatusMsg));

        xTaskCreate(SensorTask::run, sensor_cfg.name, 4096, &sensor_cfg, 2, &g_handles.sensor);
        xTaskCreate(ServoTask::run, servo_cfg.name, 4096, &servo_cfg, 3, &g_handles.servo);
        xTaskCreate(ButtonTask::run, start_btn_cfg.name, 2048, &start_btn_cfg, 4, &g_handles.start_button);
        xTaskCreate(ButtonTask::run, speed_btn_cfg.name, 2048, &speed_btn_cfg, 4, &g_handles.speed_button);
        xTaskCreate(ReadyLedTask::run, ready_led_cfg.name, 2048, &ready_led_cfg, 1, &g_handles.ready_led);
        xTaskCreate(TaskManager::run, manager_cfg.name, 4096, &manager_cfg, 5, &g_handles.manager);
    }
}
