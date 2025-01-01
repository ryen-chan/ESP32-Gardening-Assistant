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
#define RELAY_PIN 16

//based on testing, moisture values vary greatly depending on sensor insertion depth and soil compactness
//ideal moisture conditions (inserting such that "or" remains visible) -> [1700-2000]
//after watering -> [<1575]
#define MOISTURE_THRESHOLD 1600 //smaller the value, higher the moisture

SMTPSession smtp;
Session_Config config;
SMTP_Message message;

int hour;
int moistureData[24];

TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t wateringTaskHandle = NULL;
TaskHandle_t emailTaskHandle = NULL;

/*

Takes hourly readings from a capacitive soil moisture sensor.
Notifies watering task every hour.
Notifies email task every 24 hours.

*/
void readSensorData(void * parameters){
  for(;;){

    //read sensor data every hour for 24 hours
    for(hour = 0; hour < 24; hour++){

      Serial.println("Reading Soil Moisture Data...");
      moistureData[hour] = analogRead(SOIL_MOISTURE_PIN);

      Serial.println("Notifying Watering Task...");
      xTaskNotifyGive(wateringTaskHandle); //notify watering task
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //block indefinitely until watering task done

      vTaskDelay(60 * 60 * 1000 / portTICK_PERIOD_MS); //delay for an hour

    }
    
    Serial.println("Notifying Email Task...");
    xTaskNotifyGive(emailTaskHandle); //notify email task
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //block indefinitely until email task done

  }
}

/*

Regulates moisture based on latest reading.
If too wet (moisture value below threshold), turn off timed watering system.
Otherwise, turn watering system on.

*/
void regulateMoisture(void * parameters){
  for(;;){

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //block indefinitely until notified
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.println("Hello from Watering Task!");

    Serial.println(moistureData[hour]);

    //closed -> watering OFF, open -> watering ON
    //relay module -> normally open
    if(moistureData[hour] <= MOISTURE_THRESHOLD){ //if below threshold

      //turn off watering system
      digitalWrite(RELAY_PIN, HIGH);

    }else{ //if above threshold

      //turn on watering system
      digitalWrite(RELAY_PIN, LOW);

    }

    xTaskNotifyGive(sensorTaskHandle); //notify sensor task when done

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
  pinMode(RELAY_PIN, OUTPUT);

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
    regulateMoisture,
    "WateringTask",
    1000,
    NULL,
    1,
    &wateringTaskHandle
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