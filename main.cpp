#include "task_manager.hpp"

extern "C" void app_main(void)
{
    //crea las colas y tareas del sistema
    App::app_tasks_create();
}
