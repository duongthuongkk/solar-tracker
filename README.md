# Solar tracker system
## Descriptions
This project shows a system that can track the sun using a light sensor. It calculates an angle that can receive a lot of solar energy and continuously sends the voltage log for user management.

## Features
-  Use STM32 to control 2 servo engines to set the best angle at which to receive the energy and read voltage data through ADC from an IC LM1117.
-  Data is transferred by the UART protocol from STM32 to ESP32
-  Build a FreeRTOS on ESP32 to manage two tasks: receive data task from UART and send data to MQTT server task.
-  Create a queue to communicate between two task. 
