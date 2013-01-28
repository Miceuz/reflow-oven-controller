#include <Cmd.h>

boolean doRead = false;
const int tempInput = A0;

void setup() {
  analogReference(EXTERNAL);
  cmdInit(57600);
  
  cmdAdd("readTemp", readTemp);
  cmdAdd("stop", stopit);
//  cmdAdd("args", arg_display);
//  cmdAdd("blink", led_blink);
//  cmdAdd("pwm", led_pwm);
  Serial.println("Hello");
}

void loop() {
  cmdPoll();
  
  if (doRead) {
    int sensorValue = analogRead(tempInput);
    int resistance = (float) map(sensorValue, 0, 1023, 0, 250000) / (float)11 * 1.041 - 597.6;
//    Serial.print("resistance:");
    Serial.println(resistance);
    delay(300);
  }
}

void readTemp(int argc, char** argv) {
  doRead = true;
}

void stopit(int argc, char** argv) {
  doRead = false;
}


