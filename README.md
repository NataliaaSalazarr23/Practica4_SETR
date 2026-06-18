# Practica4_SETR

Emilio Hernández Santana - 10095
Natalia Marian Salazar Domínguez - 10073


## Preguntas  

¿Cuál es la diferencia entre usar una variable global y una cola para comunicar datos?  
Una variable global puede ser leída o modificada por varias tareas al mismo tiempo, lo que puede causar errores de sincronización. Una cola permite enviar datos de forma ordenada y segura entre tareas, evitando conflictos. 

¿Qué tarea queda bloqueada cuando espera datos de una cola?  
Queda bloqueada la tarea que intenta leer la cola. Esta tarea permanece esperando hasta que llegue un dato o hasta que se cumpla el tiempo máximo de espera.  

¿Por qué TaskManager debe concentrar las decisiones del sistema?  
Porque funciona como el control central del programa. Recibe la información de las demás tareas y decide qué acciones tomar, evitando que cada tarea tome decisiones por separado.  

¿Por qué pvParameters es más flexible que crear una función distinta por tarea?  
Porque permite usar una misma función para varias tareas, cambiando solo los parámetros que recibe. Así el código es más reutilizable y fácil de modificar.  

¿Qué diferencia existe entre suspender una tarea y bloquearla esperando una cola?  
Suspender una tarea la detiene hasta que otra parte del programa la reactive. En cambio, bloquearla por una cola significa que la tarea espera automáticamente hasta recibir un dato.  

¿Qué efecto tiene aumentar el tamaño de la ventana del filtro de mediana?  
Hace que la señal sea más estable y reduzca más ruido, pero también vuelve la respuesta más lenta ante cambios rápidos.  

¿Por qué un filtro de mediana rechaza picos mejor que un promedio simple?  
Porque la mediana toma el valor central de un conjunto de datos y no se ve tan afectada por valores extremos. En cambio, el promedio sí cambia mucho si aparece un pico alto o bajo.  

¿Qué ocurre si se presiona Start-operation mientras el servo está en movimiento?  
El sistema no debe iniciar una nueva operación inmediatamente. Normalmente ignora el botón o espera a que el servo termine su movimiento para evitar conflictos.  

¿Cómo se garantiza que el botón de velocidad solo funcione durante la operación?  
El TaskManager valida el estado del sistema. Solo acepta el cambio de velocidad cuando la operación está activa; si el sistema está en reposo, ignora ese botón.  

¿Por qué un constexpr en C++ es preferible a #define para constantes tipadas?  
Porque constexpr tiene tipo de dato, respeta el alcance del código y permite detectar errores en compilación. #define solo reemplaza texto y puede causar errores más difíciles de encontrar.


## Conclusiones  

En esta práctica se aplicaron conceptos fundamentales de sistemas embebidos en tiempo real utilizando FreeRTOS, como la comunicación entre tareas mediante colas, la sincronización de eventos y la organización del sistema mediante un administrador central de tareas. Además, se implementó el procesamiento de señales utilizando un filtro de mediana para mejorar la calidad de las mediciones y se controló el comportamiento de un servomotor a través de diferentes estados de operación. Gracias a esta práctica se comprendió la importancia de diseñar sistemas modulares, seguros y eficientes, donde cada tarea cumple una función específica y la comunicación se realiza de forma ordenada y confiable.
