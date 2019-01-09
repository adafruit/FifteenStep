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
    trellis.fill(0);
    modeDisplay(current);
    return;
  }

  trellis.fill(0);
  noteDisplay(current);
  setPlayhead(current % WIDTH, true);

}

// the callback that will be called by the sequencer
// when it needs to send commands over BLE.
void midi(byte channel, byte command, byte arg1, byte arg2) {

  byte c = channel > 0 ? channel - 1 : 0;

  trellis.setUSBMIDIchannel(c);
  trellis.setUARTMIDIchannel(c);

  if(command == 0x9)
    trellis.noteOn(arg1, arg2);
  else if(command == 0x8)
    trellis.noteOff(arg1, arg2);
  else
    trellis.controlChange(command, arg1);

}

#endif
