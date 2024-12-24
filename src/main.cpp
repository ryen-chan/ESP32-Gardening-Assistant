#include <Arduino.h>

TaskHandle_t emailTaskHandle = NULL;
int moistureData[24];

/*
Takes hourly readings from a capacitive soil moisture sensor.
Notifies watering task if below moisture threshold.
Notifies email task every 24 hours.
*/
void readSensorData(void * parameters){
  for(;;){

    //read sensor data every hour for 24 hours
    for(int i = 0; i < 24; i++){

      Serial.println("Reading Soil Moisture Data...");

      vTaskDelay(60 * 1000 / portTICK_PERIOD_MS); //delay for a minute

    }
    
    //notify email task
    Serial.println("Notifying Email Task...");
    xTaskNotifyGive(emailTaskHandle);

  }
}

/*
Send email with information about soil moisture.
*/
void sendStatusEmail(void * parameters){
  for(;;){

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //block indefinitely until notified

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    Serial.println("Hello from Email Task");
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);

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