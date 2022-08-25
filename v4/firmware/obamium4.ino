/*
 * Bro idk what obamium version this is anymore :sob:
 * This is just here for me to write 11.58 => 3.865
 */

// TODO FIND OUT HOW MANY ANALOG TICKS EQUAL 1 VOLT!!!
// We are only GUESSING 64 because of Obamium V2
// THIS IS INACCURATE!
#define VSENSE_VOLT 62

// Input pins from the fob recv
// Expected to be a continuous block - no gaps in between
#define I0 2
#define I1 3
#define I2 4
#define I3 5
#define RECV_INT 6

#define BTN_DELAY 1000

// Harness pins
#define VSENSE A0
#define ACC A1
#define ST 9
#define IG 8
#define KD 7



// RGB LED status lights
#define LR A2
#define LG A3
#define LB A6

volatile int I_STATE = 0;
volatile unsigned long UPDATE_TS = 0;

unsigned long blink_delay = 2000;

enum start_state {
  NONE,
  PRE_START,
  STARTING,
  START_VERIFY,
  ACTIVE,
  PRE_OFF,
  PRE_HOLD
};

enum start_state status;
unsigned long state_change_time;

volatile int pre_start_voltage;
volatile int min_voltage;

void setup() {
  Serial.begin(115200);
  for(int i = I0; i <= RECV_INT; i++) {
    pinMode(i, INPUT);
  }
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LR, OUTPUT);
  pinMode(LG, OUTPUT);
  pinMode(LB, OUTPUT);
  pinMode(VSENSE, INPUT);
  pinMode(ACC, INPUT);
  pinMode(KD, OUTPUT);
  pinMode(IG, OUTPUT);
  pinMode(ST, OUTPUT);
  // IG is a P-channel MOSFET and the RGB LED is wired by cathodes, so they are inverted input
  digitalWrite(IG, LOW);
  digitalWrite(LR, HIGH);
  digitalWrite(LG, HIGH);
  digitalWrite(LB, HIGH);
  attachInterrupt(digitalPinToInterrupt(RECV_INT), btn_clk, CHANGE);
}

void btn_clk() {
  Serial.println("Button state change");
  if(digitalRead(I3) == HIGH) {;
    if(millis() - UPDATE_TS > 1000) {
      UPDATE_TS = millis();
    }
    I_STATE = 1;
  }
  else if(digitalRead(I2) == HIGH) {
    if(millis() - UPDATE_TS > 1000) {
      UPDATE_TS = millis();
    }
    I_STATE = 2;
  }
  else {
    I_STATE = 0;
  }
}

void loop() {
  // button handling
  if(millis() - UPDATE_TS > BTN_DELAY) {
    if(I_STATE == 1) {
      // I don't know why, but this if statement shouldn't return true
      // It does anyways and prints false alarm
      if(millis() - UPDATE_TS < BTN_DELAY) {
        Serial.println("False alarm");
      }
      else {
        I_STATE = 0;
        if(status == NONE) {
          if(digitalRead(ACC) == LOW) {
            Serial.println("MEZAMETAMAE, WAGA ARUJI TACHI YO!");
            pre_start_voltage = analogRead(VSENSE);
            min_voltage = pre_start_voltage;
            status = PRE_START;
            state_change_time = millis() + 250;
            Serial.print("Pre-start voltage is ");
            Serial.println(pre_start_voltage);
          }
          else {
            Serial.println("Attempted to start obamacar while a key is inserted. Are you ok?");
          }
        }
      }
    }
    else if(I_STATE == 2) {
      if(millis() - UPDATE_TS < BTN_DELAY) {
        Serial.println("False alarm");
      }
      else {
        I_STATE = 0;
        Serial.println("Toggle Button 2");
        if(status == ACTIVE) {
          Serial.println("Turning off remotely...");
          status = PRE_OFF;
        }
        else {
          if(digitalRead(ACC) == HIGH) {
            Serial.println("Holding in 5 seconds...");
            status = PRE_HOLD;
            state_change_time = millis() + 5000; // start holding in 5 seconds
          }
          else {
            Serial.println("Attempted to hold key without a key inserted. Are you ok?");
          }
        }
      }
    }
  }

  // hold the key
  if(status == PRE_HOLD && millis() > state_change_time) {
    status = ACTIVE;
  }

  // Pin mapping
  switch(status) {
    case PRE_OFF:
    case PRE_START:
      digitalWrite(KD, HIGH);
      digitalWrite(IG, LOW);
      digitalWrite(ST, LOW);
    break;
    case STARTING:
      digitalWrite(KD, HIGH);
      digitalWrite(IG, HIGH);
      digitalWrite(ST, HIGH);
    break;
    case PRE_HOLD:
    case START_VERIFY:
    case ACTIVE:
      digitalWrite(KD, HIGH);
      digitalWrite(IG, HIGH);
      digitalWrite(ST, LOW);
      blink_delay = 500;
    break;
    default:
      digitalWrite(KD, LOW);
      digitalWrite(IG, LOW);
      digitalWrite(ST, LOW);
      blink_delay = 2000;
  }

  // Transition from key-detect to actual starting
  if(status == PRE_START && millis() > state_change_time) {
    Serial.println("Going from pre-start to start");
    status = STARTING;
    state_change_time = millis() + 3000;
  }

  // Start Protection
  if(status == STARTING) {
    int volt = analogRead(VSENSE);
    if((float)volt >= (float)pre_start_voltage + 0.5*VSENSE_VOLT) {
      Serial.println("Start succeeded!");
      Serial.print("Volt, min_voltage, pre_start_voltage, compared voltage: ");
      Serial.print((float)volt);
      Serial.print((float)min_voltage);
      Serial.print((float)pre_start_voltage);
      Serial.print((float)pre_start_voltage + 0.5*VSENSE_VOLT);
      status = ACTIVE;
    }
    else if(volt < min_voltage) {
      min_voltage = volt;
    }
    if(millis() > state_change_time) {
      Serial.print("Compare: ");
      Serial.print(millis());
      Serial.print(" > ");
      Serial.println(state_change_time);
      Serial.println("Start likely failed. Verifying...");
      status = START_VERIFY;
      state_change_time = millis() + 250;
    }
  }

  Serial.println(analogRead(VSENSE));

  // Did the start fail?
  if(status == START_VERIFY && millis() > state_change_time) {
    Serial.print("Pre Start Voltage: ");
    Serial.println(pre_start_voltage);
    if((float)analogRead(VSENSE) > (float)pre_start_voltage + 0.5 * VSENSE_VOLT) {
      Serial.println("Start verified!");
      status = ACTIVE;
    }
    else {
      Serial.println("***FAILED TO START***");
      status = PRE_OFF;
      state_change_time = millis() + 250;
    }
  }

  
  // Turning obamacar off
  if(status == ACTIVE && digitalRead(ACC) == HIGH) {
    status = PRE_OFF;
    state_change_time = millis() + 500;
  }
  if(status == PRE_OFF && millis() > state_change_time) {
    Serial.println("Retracting key");
    status = NONE;
  }
  
  // telling the user about the current state
  if(status == PRE_OFF) {
    digitalWrite(LG, LOW);
    digitalWrite(LB, LOW);
  }
  else if(status == START_VERIFY || status == PRE_HOLD) {
    digitalWrite(LG, LOW);
    digitalWrite(LB, HIGH);
  }
  else if(status == STARTING) {
    digitalWrite(LG, HIGH);
    digitalWrite(LB, LOW);
  }
  else {
    digitalWrite(LG, HIGH);
    digitalWrite(LB, HIGH);
  }

  // blink
  if((millis() / blink_delay) % 2 == 0) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  delay(100);
}
