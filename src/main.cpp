#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Mail_Client.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 587

#define SENDER_EMAIL ""
#define SENDER_PASSWORD ""

#define RECIPIENT_EMAIL ""

SMTPSession smtp;

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
Sends email with information about soil moisture.

What is SMTP? Simple Mail Transfer Protocol
Process for delivering emails to a mail server from an email client

1) TCP connection between client and server
2) Client sends "Hello" command (HELO)
3) Client sends multiple commands with email content
4) Client alerts server when data transmission is complete, and server closes connection

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

  //connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connection successful!");


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