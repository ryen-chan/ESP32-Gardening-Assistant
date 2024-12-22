#include <Arduino.h>

TaskHandle_t emailTaskHandle = NULL;

/*
Takes hourly/daily readings from a capacitive soil moisture sensor.
Notifies email task (and watering task if below moisture threshold).
*/
void readSensorData(void * parameters){
  for(;;){

    Serial.println("Reading Soil Moisture Data...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    Serial.println("Notify Email Task");
    xTaskNotifyGive(emailTaskHandle);

  }
}

/*
Send email with information about soil moisture.
*/
void sendStatusEmail(void * parameters){
  for(;;){

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //block indefinitely until notified
    Serial.println("Hello from Email Task");
    
  }
}

void setup() {

  Serial.begin(115200);

  xTaskCreate(
    readSensorData,
    "SensorTask",
    1000,
    NULL,
    1,
    NULL
  );

  xTaskCreate(
    sendStatusEmail,
    "EmailTask",
    1000,
    NULL,
    2,
    &emailTaskHandle
  );

} 

void loop() {
  // The loop function can be empty when using FreeRTOS
}