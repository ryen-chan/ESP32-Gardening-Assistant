#include <Arduino.h>

void setup() {

  Serial.begin(115200);

}

void loop() {

  // The loop function can be empty when using FreeRTOS
  Serial.println("Hello World!");
  delay(1000);
  
}