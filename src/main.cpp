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

#define SOIL_MOISTURE_PIN 35

SMTPSession smtp;
Session_Config config;
SMTP_Message message;

int moistureData[24];

TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t emailTaskHandle = NULL;


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

      moistureData[i] = analogRead(SOIL_MOISTURE_PIN);

      vTaskDelay(5 * 1000 / portTICK_PERIOD_MS); //delay for five seconds

    }
    
    Serial.println("Notifying Email Task...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    xTaskNotifyGive(emailTaskHandle); //notify email task
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //block indefinitely until email task done

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

    Serial.println("Hello from Email Task!");
    
    //set message content
    String dataStr = "";
    for(int i = 0; i < 23; i++){ //get sensor data from the past 24 hours
      dataStr += String(moistureData[i]);
      dataStr += " => ";
    }
    dataStr += moistureData[23];

    String htmlMsg = "<h1>Soil Moisture Data</h1>"; //generate HTML message
    htmlMsg += "<p>Hourly readings for the past day:<br>";
    htmlMsg += dataStr;
    htmlMsg += "</p>";

    message.html.content = htmlMsg.c_str();
    
    //message.text.content = "Testing";

    //connect to server
    Serial.println("Connecting to server...");
    if(!smtp.connect(&config)){
      Serial.println("Failed to connect");
    }

    //send email and close session
    Serial.println("Sending email...");
    if(!MailClient.sendMail(&smtp, &message)){
      Serial.println("Failed to send email");
    }

    vTaskDelay(15*1000 / portTICK_PERIOD_MS);

    xTaskNotifyGive(sensorTaskHandle); //notify sensor task when done

  }
}

void setup() {

  Serial.begin(115200);

  //set pin modes
  pinMode(SOIL_MOISTURE_PIN, INPUT);

  //connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connection successful!");

  //configure session
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;

  config.login.email = SENDER_EMAIL;
  config.login.password = SENDER_PASSWORD;
  config.login.user_domain = "";

  //set message headers
  message.sender.name = "Gardening Assistant";
  message.sender.email = SENDER_EMAIL;
  message.subject = "Daily Gardening Report";
  message.addRecipient("Ryan", RECIPIENT_EMAIL);

  smtp.debug(1);

  xTaskCreate(
    readSensorData,
    "SensorTask",
    1000,
    NULL,
    2,
    &sensorTaskHandle
  );

  xTaskCreate(
    sendStatusEmail,
    "EmailTask",
    5000,
    NULL,
    1,
    &emailTaskHandle
  );

} 

void loop() {
  // The loop function can be empty when using FreeRTOS
}