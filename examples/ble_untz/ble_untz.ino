// ---------------------------------------------------------------------------
//
// ble_untz.ino
//
// A MIDI sequencer example using the Adafruit UNTZ
// and a Bluefruit Feather.
//
// 1x Feather Bluefruit LE 32u4: https://www.adafruit.com/products/2829
// 1x Adafruit UNTZ: https://www.adafruit.com/product/1929
//
// Required dependencies:
// Adafruit Bluefruit Library: https://github.com/adafruit/Adafruit_BluefruitLE_nRF51
// Adafruit UNTZtrument Library: https://github.com/adafruit/Adafruit_UNTZtrument
// Adafruit Trellis Library: https://github.com/adafruit/Adafruit_Trellis_Library
//
// Author: Todd Treece <todd@uniontownlabs.org>
// Copyright: (c) 2015-2016 Adafruit Industries
// License: GNU GPLv3
//
// ---------------------------------------------------------------------------
#include "FifteenStep.h"

#include "bluefruit_config.h"
#include "untz_config.h"
#include "ui_config.h"

// allocate a chunk of sram for the sequencer
#define SEQUENCER_MEMORY 1024
FifteenStep seq = FifteenStep(SEQUENCER_MEMORY);

// set initial state for dynamic values
int tempo = 60;
int channel = 0;
int pitch[] = {72, 71, 69, 67, 65, 64, 62, 60};
int vel[] = {100, 80, 80, 80, 80, 80, 80, 80};
int steps = 16;

void setup() {

  Serial.print(F("Initialising the Bluefruit LE module: "));

  if(! ble.begin(VERBOSE_MODE)) {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if(FACTORYRESET_ENABLE) {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if(! ble.factoryReset()) {
      error(F("Couldn't factory reset"));
    }
  }

  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  ble.info();

  /* Set BLE callbacks */
  ble.setConnectCallback(connected);
  ble.setDisconnectCallback(disconnected);

  Serial.println(F("Enable MIDI: "));
  if(! blemidi.begin(true)) {
    error(F("Could not enable MIDI"));
  }

  ble.verbose(false);
  Serial.print(F("Waiting for a connection..."));
  while(!isConnected) {
    ble.update(500);
  }
#ifndef HELLA
  untztrument.begin(addr[0], addr[1], addr[2], addr[3]);
#else
  untztrument.begin(addr[0], addr[1], addr[2], addr[3],
                    addr[4], addr[5], addr[6], addr[7]);
#endif // HELLA
  // Default Arduino I2C speed is 100 KHz, but the HT16K33 supports
  // 400 KHz.  We can force this for faster read & refresh, but may
  // break compatibility with other I2C devices...so be prepared to
  // comment this out, or save & restore value as needed.
#ifdef ARDUINO_ARCH_SAMD
    Wire.setClock(400000L);
#endif
#ifdef __AVR__
      TWBR = 12; // 400 KHz I2C on 16 MHz AVR
#endif
  untztrument.clear();
  untztrument.writeDisplay();
  
  // start sequencer and set callbacks
  seq.begin(tempo, steps);
  seq.setMidiHandler(midi);
  seq.setStepHandler(step);

}

void loop() {
  
  ble.update(1);
 
  if(! isConnected) {
    untztrument.clear();
    untztrument.writeDisplay();
    return;
  }
  
  readButtons();

  // this is needed to keep the sequencer
  // running. there are other methods for
  // start, stop, and pausing the steps
  seq.run();

}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                       BUTTON PRESS HANDLERS                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// deal with note on and off presses
void handleNote() {
  
  float current = (float) seq.getPosition() / (float) WIDTH;
  int end = ceil(current) * WIDTH;
  
  if(end < WIDTH)
    end = WIDTH;
    
  int start = end - WIDTH;

  for(uint8_t i=0; i < N_BUTTONS; i++) {

    uint8_t x, y;
    untztrument.i2xy(i, &x, &y);

    // note on check
    if(untztrument.justPressed(i)) {
      Serial.println("ON");
      // if recording, save note on
      if(record_mode)
        seq.setNote(channel, pitch[y], vel[y], x + start);
      else
        midi(channel, 0x9, pitch[y], vel[y]);

    }

    if(untztrument.justReleased(i)) {
      Serial.println("OFF");

      // if recording, save note off
      if(record_mode)
        seq.setNote(channel, pitch[y], 0x0, x + start);
      else
        midi(channel, 0x8, pitch[y], 0x0);

    }

  }

}

// allow user to select which command mode to enter
void handleCommand() {

   if(shuffle_mode)
     shuffleChange();
   else if(tempo_mode)
     tempoChange();
   else if(pitch_mode)
     pitchChange();
   else if(velocity_mode)
     velocityChange();
   else if(step_mode)
     stepChange();
   else if(channel_mode)
     channelChange();

}

void tempoChange() {

  if(current_touched == 0x20)
    seq.increaseTempo();
  else if(current_touched == 0x10)
    seq.decreaseTempo();

}

void shuffleChange() {

  if(current_touched == 0x20)
    seq.increaseShuffle();
  else if(current_touched == 0x10)
    seq.decreaseShuffle();

}

void pitchChange() {

   if(! position_selected) {
     setModePosition();
     return;
   }

  int last = pitch[mode_position];
  changeValue(pitch[mode_position], 127);

  if(last != pitch[mode_position])
    midi(channel, 0x9, pitch[mode_position], vel[mode_position]);

}

void velocityChange() {

   if(! position_selected) {
     setModePosition();
     return;
   }

  int last = vel[mode_position];
  changeValue(vel[mode_position], 127);

  if(last != vel[mode_position])
    midi(channel, 0x9, pitch[mode_position], vel[mode_position]);

}

void channelChange() {
  changeValue(channel, 15);
}

void stepChange() {
  changeValue(steps, FS_MAX_STEPS);
  seq.setSteps(steps);
}

// common method for increasing or decreasing values
int changeValue(int &current, int max_val) {

  uint8_t increase = untztrument.xy2i(1,0);
  uint8_t decrease = untztrument.xy2i(0,0);

  // increasing or decreasing value?
  if(untztrument.justPressed(increase))
    current = current > 0 ? current - 1 : 0;
  else if(untztrument.justPressed(decrease))
    current = current < max_val ? current + 1 : max_val;

  return current;

}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                         SEQUENCER CALLBACKS                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// called when the step position changes. both the current
// position and last are passed to the callback
void step(int current, int last) {

  if(commandMode()) {
    untztrument.clear();
    untztrument.writeDisplay();
    return;
  }
  untztrument.clear();
  noteDisplay(current);
  setPlayhead(current % WIDTH, true);

  untztrument.writeDisplay();

}

// the callback that will be called by the sequencer
// when it needs to send commands over BLE.
void midi(byte channel, byte command, byte arg1, byte arg2) {

  // init combined byte
  byte combined = command;

  // shift if necessary and add MIDI channel
  if(combined < 128) {
    combined <<= 4;
    combined |= channel;
  }

  blemidi.send(combined, arg1, arg2);

}
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                          DISPLAY FUNCTIONS                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

void noteDisplay(int current) {

  int length = SEQUENCER_MEMORY / sizeof(FifteenStepNote);
  float c = (float) current / (float) WIDTH;
  int end = ceil(c) * WIDTH;

  if(end < WIDTH)
    end = WIDTH;
  
  int start = end - WIDTH;

  FifteenStepNote* _sequence = seq.getSequence();

  for(int i=0; i < length; ++i) {

    // default state, skip.
    if(_sequence[i].pitch == 0 && _sequence[i].velocity == 0 && _sequence[i].step == 0)
      continue;

    // if the current step isn't in the display range, skip
    if(_sequence[i].step < start || _sequence[i].step >= end)
      continue;

    // if we are on a different channel, skip
    if(channel != _sequence[i].channel)
      continue;

    uint8_t x = _sequence[i].step % WIDTH;
    uint8_t y = pitchToCol(_sequence[i].pitch);

    // pitch isn't currently set
    if(y == 255)
      continue;

    uint8_t led = untztrument.xy2i(x, y);

    if(_sequence[i].velocity != 0)
      untztrument.setLED(led);
    else
      untztrument.clrLED(led);

  }

}

// turn on (or off) one column of the display
void setPlayhead(uint8_t x, boolean set) {

  for(uint8_t y=0; y<8; y++) {
    uint8_t i = untztrument.xy2i(x, y);
    if(set)
      untztrument.setLED(i);
    else
      untztrument.clrLED(i);
  }

}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                      GENERAL UTILITY FUNCTIONS                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

uint8_t pitchToCol(uint8_t p) {

  for(uint8_t i=0; i < 8; i++) {
    if(pitch[i] == p) {
      return i;
    }
  }

  return 255;

}

bool commandPressed(GridCoordinate buttons[]) {

  int length = sizeof(buttons) / sizeof(GridCoordinate);
  uint8_t button = 0;

  Serial.print("command length: ");
  Serial.println(length);

  for(int i=0; i < 2; i++) {

    button = untztrument.xy2i(buttons[i].x, buttons[i].y);

    if(! untztrument.isKeyPressed(button))
      return false;

  }

  return true;

}

// read the currently touched buttons and detect any
// pressed patterns that match command states.
void readButtons() {
  
  unsigned long t = millis();
  
  if((t - prevReadTime) < 20L) // 20ms = min Trellis poll time
    return;

  if(! untztrument.readSwitches()) // Button state change?
    return;

  if(commandPressed(MIDI_PANIC)) {
    seq.panic();
  } else if(commandPressed(TEMPO_MODE)) {
    tempo_mode = !tempo_mode;
  } else if(commandPressed(SHUFFLE_MODE)) {
    shuffle_mode = !shuffle_mode;
  } else if(commandPressed(STEP_MODE)) {
    position_selected = false;
    step_mode = !step_mode;
  } else if(commandPressed(CHANNEL_MODE)) {
    position_selected = false;
    channel_mode = !channel_mode;
  } else if(commandPressed(PITCH_MODE)) {
    pitch_mode = !pitch_mode;
  } else if(commandPressed(VELOCITY_MODE)) {
    velocity_mode = !velocity_mode;
  } else if(commandPressed(RECORD_TOGGLE)) {
    record_mode = !record_mode;
  } else if(commandPressed(PAUSE_TOGGLE)) {
    seq.pause();
  } else {
    if(commandMode()) {
      handleCommand();
    } else {
      handleNote();
    }
  }

}

// get the index of the pressed button. this is used by
// the velocity and note modes to tell which button to change
void setModePosition() {

  for(uint8_t i=0; i < N_BUTTONS; i++) {
    if(untztrument.justPressed(i)) {
      mode_position = i;
      position_selected = true;
    }
  }

}

// resets mode display to off
void clearModes() {

  // turn off all modes
  channel_mode = false;
  velocity_mode = false;
  pitch_mode = false;
  step_mode = false;
  tempo_mode = false;
  shuffle_mode = false;

  // reset mode position
  mode_position = 0;
  position_selected = false;

  // clear pixels
  untztrument.clear();
  untztrument.writeDisplay();

}

// check if we have selected a command mode
bool commandMode() {

  if(channel_mode || velocity_mode || pitch_mode || step_mode || tempo_mode || shuffle_mode)
    return true;

  return false;

}
