#pragma once
#include <cstdint>
#include "messages.hpp"

namespace App
{
    struct ButtonTaskConfig
    {
        uint8_t gpio;
        const char *name;
        ButtonEventType event_type;
        uint32_t poll_ms;
        bool send_on_change_only;
    };

    class ButtonTask
    {
        public:
            static void run(void *pvParameters);
    };
}
