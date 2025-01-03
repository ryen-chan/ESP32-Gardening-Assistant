# ESP32-Gardening-Assistant

An ESP32-based system that monitors and regulates soil moisture to ensure proper conditions for plant growth

## Required Materials

* ESP32
* Breadboard
* Soil Moisture Sensor
* Relay Module

## Project Components

* Hourly soil moisture readings using a capacitive soil moisture sensor
* Daily email notifications through SMTP
* Toggling on/off existing front yard sprinkler system based on current moisture levels

## Challenges

* Soil moisture readings can vary depending on sensor insertion depth and soil compactness (had to standardize insertion method)

## Future Improvements

* Plant Identification / Plant-Specific Soil Moisture Regulation

## References

* <https://www.freertos.org/Documentation/02-Kernel/04-API-references/05-Direct-to-task-notifications/00-RTOS-task-notifications>
* <https://github.com/mobizt/ESP-Mail-Client>
