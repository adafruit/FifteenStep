///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                          DISPLAY FUNCTIONS                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef DISPLAY_HELPER_H
#define DISPLAY_HELPER_H

uint8_t pitchToCol(uint8_t p) {

  for(uint8_t i=0; i < 8; i++) {
    if(pitch[i] == p) {
      return i;
    }
  }

  return 255;

}

void rowDisplay(uint8_t y, uint8_t end) {

  for(uint8_t x=0; x<end; x++) {
    uint8_t i = untztrument.xy2i(x, y);
    untztrument.setLED(i);
  }

}

void shuffleDisplay(int position) {

  uint8_t row = SHUFFLE_MODE[0].y;
  uint8_t i;

  if((position % 2) == 0)
    i = untztrument.xy2i(0, row);
  else
    i = untztrument.xy2i(1, row);

  untztrument.setLED(i);

}

void tempoDisplay(int position) {

  uint8_t row = TEMPO_MODE[0].y;

  if((position % 2) == 0)
    rowDisplay(row, WIDTH);

}

void modeDisplay(int current) {

   if(shuffle_mode)
     shuffleDisplay(current);
   else if(tempo_mode)
     tempoDisplay(current);
   /*
   else if(pitch_mode)
     pitchChange();
   else if(velocity_mode)
     velocityChange();
   else if(step_mode)
     stepChange();
   else if(channel_mode)
     channelChange();
     */

}

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

#endif
