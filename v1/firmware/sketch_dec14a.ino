/*
 * NOORQUACKER INDUSTRIES SEEECRET CODE
 * OBAMIUM VERSION 0.0.4
 * OBAMA CAR REMOTE START ARDUINO CODE
 */

#define NONE 0
#define STARTING 1
#define ACTIVE 2
#define PIN_ACC A4
#define PIN_VSENSE A7
//#define ACTIVE_THRESHOLD 870           //Obamacar spec says digitalRead(A0) = 64.34 * A0 = (5.003/1023) * 10K/(10K+22K) * Vin, so do whatever
#define A0_FACTOR 64.34               //Only for A0
int startstate = NONE;
float battvolt = 0;
unsigned long first_time = 0;
float minvolt = 0;
float initvolt = 0;
bool a = false;

void setup() {
  Serial.begin(115200);
  /*
  pinMode(A6, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(13, INPUT);
  pinMode(12, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  */
  pinMode(2, INPUT); //am1 but not analog
  pinMode(A7, INPUT); //am1
  pinMode(A0, INPUT); //d
  pinMode(A1, INPUT); //c
  pinMode(A2, INPUT); //b
  pinMode(A3, INPUT); //a
  pinMode(3, OUTPUT); //kd
  pinMode(4, OUTPUT); //ig
  pinMode(5, OUTPUT); //st
  pinMode(6, OUTPUT); //tr
  pinMode(PIN_ACC, INPUT);
  pinMode(PIN_VSENSE, INPUT);
}

void loop() {
  //trunk, the very first completed function in obamamium & obamacar
  /*if(digitalRead(A1) == HIGH) {
    Serial.println("Trunk!");
    digitalWrite(6, HIGH);
  }
  else {
    digitalWrite(6, LOW);
  }*/
  digitalWrite(6, digitalRead(A1));
  //remote start beta
  if(digitalRead(A0) == HIGH && startstate == NONE) {
    delay(500);
    if(digitalRead(A0) == HIGH && digitalRead(PIN_ACC) == LOW) {
      startstate = STARTING;
      Serial.println("MEZAMETAMAE, WAGA ARUJI TACHI YO!");
      minvolt = analogRead(PIN_VSENSE); //get a baseline voltage reading
      initvolt = minvolt;
      digitalWrite(3, HIGH);
      delay(250);
      digitalWrite(4, HIGH);
      first_time = millis();
    }
    else if(digitalRead(PIN_ACC) == HIGH) {
      Serial.println("I AM NOT STARTING THE CAR WHILE IT IS ON");
      startstate = NONE;
    }
  }
  else if(startstate == NONE) {
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
  }
  if(startstate == ACTIVE) {
    if(digitalRead(PIN_ACC) == HIGH) {
      Serial.println("Key detected.");
      delay(1000);
      Serial.println("Revoking all control!");
      startstate = NONE;
    }
    else {
      Serial.println("Waiting for key...");
      digitalWrite(3, HIGH);
      digitalWrite(4, HIGH);
      digitalWrite(5, LOW);
    }
  }
  else if(startstate == STARTING) {
    int volt = analogRead(PIN_VSENSE);
    if(volt >= minvolt + A0_FACTOR*0.5) {
      startstate = ACTIVE;
      delay(500);
      digitalWrite(5, LOW);
    }
    else {
      if(volt < minvolt) {
        minvolt = volt;
      }
      if(millis() - first_time >= 2950) {
        Serial.println("Failed to start. Verifying...");
        digitalWrite(5, LOW); //NO DONT TURN IGNITION OFF BEFORE THE FRIGGIN START ARE YOU STUPID ME //Turn ignition off first, otherwise obamacar gets confused and won't retract the steering wheel
        delay(250);
        if(analogRead(PIN_VSENSE) > A0_FACTOR*0.5 + initvolt) { //usually obamacar will be 14V when on, but will never have a higher voltage after any start because thats how batteries work baka
          Serial.println("FRICK YEAH IT WORKED");
          startstate = ACTIVE;
        }
        else{
          startstate = NONE; //Failed to start
          Serial.println("Failed :(");
          digitalWrite(4, LOW); //Turn ignition off AFTER turning start off, otherwise obamacar won't retract steering wheel
        }
      }
      else {
        digitalWrite(5, HIGH);
      }
    }
  }
  Serial.print((float)analogRead(PIN_VSENSE)/A0_FACTOR);
  Serial.print("V");
  if(digitalRead(PIN_ACC) == HIGH) {
    Serial.println("We have a key");
  }
  else {
    Serial.println("No key yet");
  }
  digitalWrite(LED_BUILTIN, a ? HIGH : LOW);
  a = !a;
  delay(500); //can't freak out the pins
}
