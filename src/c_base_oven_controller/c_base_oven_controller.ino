#include <Cmd.h>

#define HEATER A3
#define RUN_LED 8
#define BUTTON 6

#define PRESSED 1
#define MAYBE_PRESSED 2
#define RELEASED 3
#define MAYBE_RELEASED 4

typedef struct {
  byte temperature;
  byte holdSeconds;
} tProgramPoint;

const int tempInput = A0;
boolean heaterOn = false;
boolean running = false;

unsigned long lastPrint = 0;
unsigned long lastDebounce = 0;
byte buttonState = RELEASED;
byte lastButtonState = RELEASED;
//byte setTemperature = 235;
unsigned int tempMaxHold = 30000;
byte tempReached = false;
unsigned long tempReachedTimestamp = 0;

tProgramPoint program[]={
  {150, 180},
  {205, 15},
  {0, 0}
};

byte programStep = 0;


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
  cmdAdd("run", run);
  cmdAdd("setTemp", setTemp);
  blinkHello();
  Serial.println("Hello");
}

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
    if(temperature < program[programStep].temperature) {
      heaterOn = true;
    } else {
      heaterOn = false;
      if(!tempReached) {
        tempReached = true;
        tempReachedTimestamp = millis();
      }
    }
    if(tempReached) {
      if((millis() - tempReachedTimestamp)/1000 > program[programStep].holdSeconds) {
        programStep ++;
        Serial.print("next temp:");
        Serial.println(program[programStep].temperature);
        if(0 == program[programStep].temperature) {
          Serial.println("Stopping");
          stopit(0, 0);
        } else {
          tempReached = false;
        }
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
  programStep = 0;
  heaterOn = true;
  running = true;
  tempReached = false;  
}

void stopit(int argc, char** argv) {
  heaterOn = false;
  running = false;
}

void setTemp(int argc, char** argv) {
  if(2 == argc) {
    program[programStep].temperature = atol(argv[1]);
    Serial.print("Setting to ");
    Serial.println(program[programStep].temperature);
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

