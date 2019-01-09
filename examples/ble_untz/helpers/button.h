///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                        BUTTON PRESS FUNCTIONS                             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef BUTTON_HELPER_H
#define BUTTON_HELPER_H

#include "callback.h"

// forward definitions 
void handleCommand();
void handleNote();

bool commandPressed(GridCoordinate buttons[]) {

  int length = sizeof(buttons) / sizeof(GridCoordinate);
  uint8_t button = 0;

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

  if((t - prevReadTime) < 20L)
    return;

  prevReadTime = t;

  if(! untztrument.readSwitches())
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

      // if recording, save note on
      if(record_mode) {
        seq.setNote(channel, pitch[y], vel[y], x + start);
        seq.setNote(channel, pitch[y], 0, x + start + 1);
      } else {
        midi(channel, 0x9, pitch[y], vel[y]);
      }

    } else if(untztrument.justReleased(i)) {

      if(! record_mode)
        midi(channel, 0x8, pitch[y], 0x0);

    }

  }

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

#endif
