#include <Cmd.h>

#define HEATER A3
#define RUN_LED 8
#define BUTTON 6

#define PRESSED 1
#define MAYBE_PRESSED 2
#define RELEASED 3
#define MAYBE_RELEASED 4

const int tempInput = A0;
boolean heaterOn = false;
boolean running = false;

void blinkHello() {
  byte i;
  for(i = 0; i < 10; i++) {
    digitalWrite(RUN_LED, !digitalRead(RUN_LED));
    delay(100);
  }
}

void setup() {
  pinMode(RUN_LED, OUTPUT);
  pinMode(HEATER, OUTPUT);
  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, HIGH);//button pullup on
      
  analogReference(EXTERNAL);
  cmdInit(9600);
  
  cmdAdd("stop", stopit);
  cmdAdd("heaterOn", turnHeaterOn);
  cmdAdd("heaterOff", turnHeaterOff);
  cmdAdd("run", run);
  cmdAdd("setTemp", setTemp);
  blinkHello();
  Serial.println("Hello");
}


unsigned long lastPrint = 0;
unsigned long lastDebounce = 0;
byte buttonState = RELEASED;
byte lastButtonState = RELEASED;
byte setTemperature = 235;
unsigned int tempMaxHold = 15000;
byte tempReached = false;
unsigned long tempReachedTimestamp = 0;

void loop() {
  cmdPoll();

  if(buttonPress()) {
    if(running) {
      stopit(0, 0);
    } else {
      run(0, 0);
    }
  }

  int sensorValue = analogRead(tempInput);
  int resistance = (float) map(sensorValue, 0, 1023, 0, 250000) / (float)11 * 1.007 + 60.296;// * 1.041 - 597.6;
  float temperature = (resistance - 10000)/38.5;
    
  if(heaterOn) {
    digitalWrite(HEATER, HIGH);
  } else {
    digitalWrite(HEATER, LOW);
  }

  if(running) {
    digitalWrite(RUN_LED, HIGH);
    if(temperature < setTemperature) {
      heaterOn = true;
    } else {
      heaterOn = false;
      if(!tempReached) {
        tempReached = true;
        tempReachedTimestamp = millis();
      }
    }
    if(tempReached) {
      if((millis() - tempReachedTimestamp) > tempMaxHold) {
        Serial.println("Stopping");
        stopit(0, 0);
      }
    }
  } else {
    digitalWrite(RUN_LED, LOW);
  }
  
  if(millis() - lastPrint > 1000) {
    Serial.println(temperature);
    lastPrint = millis();
  }
}

void run(int argc, char** argv) {
  heaterOn = true;
  running = true;
  tempReached = false;  
}

void stopit(int argc, char** argv) {
  heaterOn = false;
  running = false;
}

void turnHeaterOn(int argc, char** argv) {
  heaterOn = true;
}

void turnHeaterOff(int argc, char** argv) {
  heaterOn = false;
}

void setTemp(int argc, char** argv) {
  if(2 == argc) {
    setTemperature = atol(argv[1]);
    Serial.print("Setting to ");
    Serial.println(setTemperature);
  } else {
    Serial.println("Usage: setTemp <temperature>");
  }
}

boolean buttonPress() {
  byte button = digitalRead(BUTTON);
  switch(buttonState) {
    case MAYBE_PRESSED:
      delay(100);
      if(LOW == button) {
        buttonState = PRESSED;
      } else {
        buttonState = RELEASED;
      }
      break;
    case PRESSED:
      if(HIGH == button) {
        buttonState = MAYBE_RELEASED;
      }
      break;
    case MAYBE_RELEASED:
      delay(100);
      if(HIGH == button) {
        buttonState = RELEASED;
      } else {
        buttonState = PRESSED;
      }
      break;
    case RELEASED:
      if(LOW == button) {
        buttonState = MAYBE_PRESSED;
      }
      break;
  }
  boolean ret = lastButtonState != buttonState && PRESSED == buttonState;
  lastButtonState = buttonState;

  return ret;
}

