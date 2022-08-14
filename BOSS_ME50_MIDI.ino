//
// BOSS_ME50_MIDI.ino
//
// Quentin Bolsee
//
// This work may be reproduced, modified, distributed,
// performed, and displayed for any purpose, but must
// acknowledge this project. Copyright is retained and
// must be preserved. The work is provided as is; no
// warranty is provided, and users accept all liability.
//

#include "MIDIUSB.h"

#define PIN_POT 3
#define APPROX_TOL 6
#define OVERSAMPLE 64
#define N_MODES 3

//#define DEBUG

int pot_val = 0;
int mode = 0;

int pin_btn[] = {18, 17, 16};
int pin_led[] = {23, 22, 10};


bool approx(int a, int b) {
  return a > b ? (a-b) <= APPROX_TOL : (b-a) <= APPROX_TOL;
}


int map_clamp(int v, int fromLow, int fromHigh, int toLow, int toHigh) {
  int v2 = map(v, fromLow, fromHigh, toLow, toHigh);

  return min(max(v2, toLow), toHigh);
}


int read_adc(int pin) {
  int val = 0;

  for (int i = 0; i < OVERSAMPLE; i++) {
    val += analogRead(pin);
  }

  #ifdef DEBUG
  SerialUSB.println(val);
  delay(200);
  return 0;
  #endif

  return map_clamp(val, 7400, 61200, 0, 127);
}

void setup() {
  SerialUSB.begin(0);

  for (int i = 0; i < N_MODES; i++) {
    pinMode(pin_btn[i], INPUT_PULLUP);
    pinMode(pin_led[i], OUTPUT);
  }

  // default mode
  digitalWrite(pin_led[mode], HIGH);

  pot_val = read_adc(PIN_POT);
}

// event type, event type + channel, control number (0-119), control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  // 0x0B = control change
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void loop() {
  int pot_val_new = read_adc(PIN_POT);

  for (int i = 0; i < N_MODES; i++) {
    if (digitalRead(pin_btn[i]) == LOW) {
      digitalWrite(pin_led[mode], LOW);
      mode = i;
      digitalWrite(pin_led[mode], HIGH);
      break;
    }
  }

  if (pot_val != pot_val_new) {
    SerialUSB.println(pot_val_new);

    controlChange(mode, 1, pot_val_new);

    pot_val = pot_val_new;
  }
  delay(2);
}
