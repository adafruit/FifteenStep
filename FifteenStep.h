// ---------------------------------------------------------------------------
//
// FifteenStep.h
// A step sequencer library for Arduino.
//
// Author: Todd Treece <todd@uniontownlabs.org>
// Copyright: (c) 2015 Adafruit Industries
// License: GNU GPLv3
//
// ---------------------------------------------------------------------------
#ifndef _FifteenStep_h
#define _FifteenStep_h

#include "Arduino.h"
#include "Timer.h"

class FifteenStep
{
  public:
    FifteenStep();
    void update(void);
    void setTempo(int tempo);
    void setSteps(int steps);
  private:
    int   _tempo;
    int   _steps;
    Timer _timer;
};

#endif
