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
    void            setTempo(int tempo);
    void            setSteps(int steps);
    void            run();
    void            trigger();
  private:
    int             _tempo;
    int             _steps;
    unsigned long   _sixteenth;
    Timer           _timer;
    static void     _tick(void *s);
    void            _noteOn();
    void            _noteOff();
};

#endif
