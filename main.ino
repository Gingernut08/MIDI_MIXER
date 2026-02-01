#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

/* ---------------- CONFIG ---------------- */

const int NUM_SLIDER_INPUTS  = 4;
const int NUM_SLIDER_OUTPUTS = 3;
const int NUM_SLIDERS        = 10;

const int NUM_ROWS = 5;
const int NUM_COLS = 4;
const int NUM_BUTTONS = NUM_ROWS * NUM_COLS;

/* ----------- PIN DEFINITIONS ------------ */

const int analogInputs[NUM_SLIDER_INPUTS] = {A0, A1, A2, A3};
const int digitalOutputs[NUM_SLIDER_OUTPUTS] = {2, 3, 4};

const int rowPins[NUM_ROWS] = {5, 6, 7, 8, 9};
const int colPins[NUM_COLS] = {10, 11, 12, 13};

/* ------------- STATE ------------------- */

int analogSliderValues[NUM_SLIDERS];
int sliderValues[NUM_SLIDERS];
int lastSliderValues[NUM_SLIDERS];

int buttonValues[NUM_BUTTONS];
int prevButtonValues[NUM_BUTTONS];
int changeButtons[NUM_BUTTONS];

int mutedSlides[NUM_SLIDERS] = {1,1,1,1,1,1,1,1,1,1};

/* -------- APP / CHANNEL HANDLING -------- */

const int NUM_APPS = 10;
int currentApps[NUM_SLIDERS - 2] = {0,1,2,3,4,5,6,7};

/* Buttons that change apps */
const int appChange[NUM_SLIDERS - 2] = {0,2,4,6,8,10,12,14};
/* Buttons that mute sliders */
const int muteChange[NUM_SLIDERS] = {1,3,5,7,9,11,13,15,16,17};

/* ---------------- SETUP ---------------- */

void setup() {
  for (int i = 0; i < NUM_SLIDER_OUTPUTS; i++) {
    pinMode(digitalOutputs[i], OUTPUT);
    digitalWrite(digitalOutputs[i], LOW);
  }

  for (int i = 0; i < NUM_COLS; i++) {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], LOW);
  }

  for (int i = 0; i < NUM_ROWS; i++) {
    pinMode(rowPins[i], INPUT_PULLDOWN);
  }

  MIDI.begin(MIDI_CHANNEL_OMNI);
}

/* ---------------- LOOP ----------------- */

void loop() {
  getSliders();
  getButtons();
  changeMutes();
  changeApps();
  sendMidi();
  delay(5);
}

/* ------------- SLIDERS ----------------- */

void getSliders() {
  for (int out = 0; out < NUM_SLIDER_OUTPUTS; out++) {
    digitalWrite(digitalOutputs[out], HIGH);

    for (int in = 0; in < NUM_SLIDER_INPUTS; in++) {
      int index = out * NUM_SLIDER_INPUTS + in;
      if (index < NUM_SLIDERS) {
        analogSliderValues[index] = analogRead(analogInputs[in]) >> 3; // 0–127
      }
    }

    digitalWrite(digitalOutputs[out], LOW);
  }
}

/* ------------- BUTTON MATRIX ----------- */

void getButtons() {
  for (int c = 0; c < NUM_COLS; c++) {
    digitalWrite(colPins[c], HIGH);

    for (int r = 0; r < NUM_ROWS; r++) {
      int index = c * NUM_ROWS + r;
      buttonValues[index] = digitalRead(rowPins[r]);
    }

    digitalWrite(colPins[c], LOW);
  }

  for (int i = 0; i < NUM_BUTTONS; i++) {
    changeButtons[i] =
      (buttonValues[i] == HIGH && prevButtonValues[i] == LOW);
    prevButtonValues[i] = buttonValues[i];
  }
}

/* ------------- MUTES ------------------- */

void changeMutes() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    if (changeButtons[muteChange[i]]) {
      mutedSlides[i] = !mutedSlides[i];
    }
  }
}

/* ------------- APPS / CHANNELS -------- */

void changeApps() {
  for (int i = 0; i < NUM_SLIDERS - 2; i++) {
    if (changeButtons[appChange[i]]) {
      currentApps[i]++;
      currentApps[i] %= NUM_APPS;
    }
  }
}

/* ------------- MIDI OUTPUT ------------- */

void sendMidi() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    int value = mutedSlides[i] ? analogSliderValues[i] : 0;

    if (value != lastSliderValues[i]) {
      int channel = (i < NUM_SLIDERS - 2)
                      ? currentApps[i] + 1
                      : 1;

      MIDI.sendControlChange(i, value, channel);
      lastSliderValues[i] = value;
    }
  }
}
