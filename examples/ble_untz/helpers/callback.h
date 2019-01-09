///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                     SEQUENCER CALLBACK FUNCTIONS                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef CALLBACK_HELPER_H
#define CALLBACK_HELPER_H

// forward definitions
bool commandMode();
void noteDisplay(int current);
void modeDisplay(int current);
void setPlayhead(uint8_t c, bool set);

// called when the step position changes. both the current
// position and last are passed to the callback
void step(int current, int last) {

  if(commandMode()) {
    untztrument.clear();
    modeDisplay(current);
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

#endif
