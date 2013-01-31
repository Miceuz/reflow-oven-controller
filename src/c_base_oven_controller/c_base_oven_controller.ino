#include <Cmd.h>

#define HEATER A3
#define RUN_LED 8
#define BUTTON 6

boolean doRead = false;
const int tempInput = A0;
boolean heaterOn = false;
boolean running = true;
boolean blinkit = true;
void blinkHello() {
  byte i;
  for(i = 0; i < 10; i++) {
    digitalWrite(RUN_LED, !digitalRead(RUN_LED));
    delay(100);
  }
}

void setup() {
  pinMode(RUN_LED, OUTPUT);
  blinkHello();
    
  
  analogReference(EXTERNAL);
  cmdInit(9600);
  
  cmdAdd("readTemp", readTemp);
  cmdAdd("stop", stopit);
  cmdAdd("heaterOn", turnHeaterOn);
  cmdAdd("heaterOff", turnHeaterOff);
  cmdAdd("blink", blinkOn);
  Serial.println("Hello");
  pinMode(HEATER, OUTPUT);

}

void loop() {
  cmdPoll();
  
  if (doRead) {
    int sensorValue = analogRead(tempInput);
    int resistance = (float) map(sensorValue, 0, 1023, 0, 250000) / (float)11 * 1.041 - 597.6;
    Serial.println(resistance);
    delay(300);
  }
  
  if(heaterOn) {
    digitalWrite(HEATER, HIGH);
  } else {
    digitalWrite(HEATER, LOW);
  }
  
  if(blinkit) {
    heaterOn = !heaterOn;
    delay(1000);
  }
}

void readTemp(int argc, char** argv) {
  doRead = true;
}

void stopit(int argc, char** argv) {
  doRead = false;
  heaterOn = false;
  blinkit = false;
}

void turnHeaterOn(int argc, char** argv) {
  heaterOn = true;
}

void turnHeaterOff(int argc, char** argv) {
  heaterOn = false;
}

void blinkOn(int argc, char** argv) {
  blinkit = true;
}

