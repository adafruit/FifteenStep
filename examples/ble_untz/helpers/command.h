///////////////////////////////////////////////////////////////////////////////
//                                                                           // //                        COMMAND MODE FUNCTIONS                             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef COMMAND_HELPER_H
#define COMMAND_HELPER_H

#include "callback.h"
#include "button.h"

// set the index of the pressed button. this is used by
// the velocity and note modes to tell which button to change
void setModePosition() {

  for(uint8_t i=0; i < N_BUTTONS; i++) {
    if(untztrument.justPressed(i)) {
      mode_position = i;
      position_selected = true;
    }
  }

}

void tempoChange() {

  uint8_t row = TEMPO_MODE[0].y;
  uint8_t increase = untztrument.xy2i(1, row);
  uint8_t decrease = untztrument.xy2i(0, row);

  if(untztrument.justPressed(increase))
    seq.increaseTempo();
  else if(untztrument.justPressed(decrease))
    seq.decreaseTempo();

}

void shuffleChange() {

  uint8_t row = SHUFFLE_MODE[0].y;
  uint8_t increase = untztrument.xy2i(1, row);
  uint8_t decrease = untztrument.xy2i(0, row);

  if(untztrument.justPressed(increase))
    seq.increaseShuffle();
  else if(untztrument.justPressed(decrease))
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


#endif
