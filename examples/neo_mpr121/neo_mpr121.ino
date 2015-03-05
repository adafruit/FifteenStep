#include "FifteenStep.h"
#include "Adafruit_NeoPixel.h"
#include "Wire.h"
#include "Adafruit_MPR121.h"

#define NEO_PIN  6
#define LEDS     16
#define TEMPO    60
#define BUTTONS  6
#define IRQ_PIN  4

// sequencer, neopixel, & mpr121 init
FifteenStep seq = FifteenStep(1024);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LEDS, NEO_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_MPR121 cap = Adafruit_MPR121();

// keep track of touched buttons
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

// start sequencer in record mode
bool record_mode = true;

// set command states to off by default
bool command_mode = false;
bool pitch_mode = false;
bool velocity_mode = false;
bool channel_mode = false;

// keep pointers for selected buttons to operate
// on when in note and velocity mode
int  mode_position = 0;
bool position_selected = false;

// prime dynamic values
int channel = 0;
int pitch[] = {36, 38, 42, 46, 44, 55};
int vel[] = {80, 80, 80, 80, 40, 20};
int steps = 16;

void setup() {

  // set mpr121 IRQ pin to input
  pinMode(IRQ_PIN, INPUT);

  // bail if the mpr121 init fails
  if (! cap.begin(0x5A))
    while (1);

  // start neopixels
  pixels.begin();
  pixels.setBrightness(80);

  // start sequencer and set callbacks
  seq.begin(TEMPO, 16);
  seq.setMidiHandler(midi);
  seq.setStepHandler(step);

}

void loop() {

  // if mpr121 irq goes low, there have been
  // changes to the button states, so read the values
  if(digitalRead(IRQ_PIN) == LOW)
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
void handle_note() {

  for (uint8_t i=0; i < BUTTONS; i++) {

    // note on check
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i))) {

      // play pressed note
      midi(channel, 0x9, pitch[i], vel[i]);

      // if recording, save note on
      if(record_mode)
        seq.setNote(channel, pitch[i], vel[i]);

    }

    // note off check
    if ( !(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {

      // play note off
      midi(channel, 0x8, pitch[i], 0x0);

      // if recording, save note off
      if(record_mode)
        seq.setNote(channel, pitch[i], 0x0);

    }

  }

}

// allow user to select which command mode to enter
void handle_command() {

  switch(currtouched) {

    case 0x1:
      seq.decreaseTempo();
      break;
    case 0x2:
      seq.increaseTempo();
      break;
    case 0x4:
      seq.decreaseShuffle();
      break;
    case 0x8:
      seq.increaseShuffle();
      break;
    case 0x10:
      // decrease channel number
      channel = channel > 0 ? channel - 1 : 0;
      show_range(step, channel + 1, 128, 128, 128);
      break;
    case 0x20:
      // increase channel number
      channel = channel < 16 ? channel + 1 : 15;
      show_range(channel, channel + 1, 128, 128, 128);
      break;
  }

}

// allow user to modify note pitches
void handle_pitch_change() {
  // call common handler
  change_pitch_or_velocity();
}

void handle_velocity_change() {
  // call common handler
  change_pitch_or_velocity();
}

// common method for changing pitch and velocity values
void change_pitch_or_velocity(int[] &type) {

  // no button index selected, wait until it has
  // been selected before changing values
  if(! position_selected) {
    get_position();
    return;
  }

  // stash last value
  int last = type[mode_position];
  byte[] rgb = {0,0,0};

  // increasing or decreasing value?
  if(currtouched == 0x1)
    type[mode_position] = type[mode_position] > 0 ? type[mode_position] - 1 : 0;
  else if(currtouched == 0x2)
    type[mode_position] = type[mode_position] < 127 ? type[mode_position] + 1 : 127;

  // preview change
  if(last != type[mode_position])
    midi(channel, 0x9, pitch[mode_position], vel[mode_position]);

  if(type == vel)
    rgb = {0,0,64};
  else
    rgb = {0,64,0};

  // update display
  int display = map(type[mode_position], 0, 127, 0, LEDS);
  flash(0,0,0);
  show_range(0, display_velocity, rgb[0], rgb[1], rgb[2]);

}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                         SEQUENCER CALLBACKS                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// called when the step position changes. both the current
// position and last are passed to the callback
void step(int current, int last) {

  // if we are in a command mode, flash command
  if(mode_active()) {
    mode_flash(current);
    return;
  }

  note_flash(current);

}

// the callback that will be called by the sequencer when it needs
// to send midi commands. this specific callback is designed to be
// used with an arduino leonardo or micro and the arcore midi
// usb modifications
//
// for more info on arcore:
// https://github.com/rkistner/arcore
void midi(byte channel, byte command, byte arg1, byte arg2) {

  // init combined byte
  byte combined = command;

  // shift if necessary. this is specific to the midi usb spec
  if(command < 16)
    combined = command << 4;

  // add channel. this is specific to the midi usb spec
  combined |= channel;

  // send midi message
  MIDIEvent event = {command, combined, arg1, arg2};
  MIDIUSB.write(event);
  MIDIUSB.flush();

}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                          NEOPIXEL HELPERS                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// handles flashing for the different mode states
void mode_flash(int *current) {

  // bail if we don't need to flash
  if(command_mode && position_selected)
    return;

  byte[] rgb = {0,0,0};

  // set colors for modes
  else if(pitch_mode)
    rgb = {0,64,0};
  else if(velocity_mode)
    rgb = {0,0,64};
  else if(channel_mode)
    rgb = {32,0,32};
  else
    rgb = {64,0,0};

  // LEDs on when it's an even step
  if((current % 2) == 0)
    flash(rgb[0],rgb[1],rgb[2]);
  else
    flash(0,0,0);

}

void note_flash(int *current) {

  byte[] rgb = {0,0,0};

  // make sure we stay within
  current = current % LEDS;

  // all LEDs off
  flash(0, 0, 0);

  // highlight quarter notes
  if(current % 4 == 0) {
    // red quarter notes in record mode
    // bright blue in play mode
    if(record_mode)
      rgb[0] = 255;
    else
      rgb[2] = 255;
  } else {
    // dim blue sixteenths
    b = 64;
  }

  // highlight note
  show_range(current, current + 1, rgb[0], rgb[1], rgb[2]);

}

// sets all pixels to passed RGB values
void flash(byte r, byte g, byte b) {
  show_range(0, LEDS, r, g, b);
}

// sets range of pixels to RGB values
void show_range(int start, int last, byte r, byte g, byte b) {

  for (; start < last; start++)
    pixels.setPixelColor(start, pixels.Color(r,g,b));

  pixels.show();

}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                      GENERAL UTILITY FUNCTIONS                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// read the currently touched buttons and detect any
// pressed patterns that match command states. this
// set of states is set up for 6 pads, but you could
// easily modify the hex values to match the ammount
// pads you are using
void readButtons() {

  // read current values
  currtouched = cap.touched();

  switch(currtouched) {

    case 0x21: // stop all notes. pads 1,6
      seq.panic();
      break;

    case 0x12: // pause. pads 2,4
      seq.pause();
      break;

    case 0x3: // command mode. pads 1,2
      clear_modes();
      command_mode = command_mode ? false : true;
      break;

    case 0x6: // note mode. pads 2,3
      clear_modes();
      pitch_mode = pitch_mode ? false : true;
      break;

    case 0xC: // velocity mode. pads 3,4
      clear_modes();
      velocity_mode = velocity_mode ? false : true;
      break;

    case 0x30: // shuffle mode. pads 4,5
      clear_modes();
      channel_mode = channel_mode ? false : true;
      break;

    default:
      // if it's not a command pattern, call route
      // to send it to the proper handler
      route();
      break;

  }

  // save current values to compare against in the next loop
  lasttouched = currtouched;

}

// route the presses to the proper handlers
void route() {

  if(command_mode)
    handle_command();
  else if(pitch_mode)
    handle_pitch_change();
  else if(velocity_mode)
    handle_velocity_change();
  else
    handle_note();

}

// get the index of the pressed button. this is used by
// the velocity and note modes to tell which button to change
void get_position() {

  for(int i=0; i < BUTTONS; i++) {

    if ( (currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      mode_position = i;
      position_selected = true;
    }

  }

}

// resets mode display to off
void clear_modes() {

  // turn off all modes
  channel_mode = false;
  command_mode = false;
  velocity_mode = false;
  pitch_mode = false;

  // reset mode position
  mode_position = 0;
  position_selected = false;

  // clear pixels
  flash(0,0,0);

}

// check if a command mode is active
bool mode_active() {

  if(command_mode || pitch_mode || velocity_mode || channel_mode)
    return true;

  return false;

}
