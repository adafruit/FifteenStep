#include <math.h>

typedef struct {
  uint8_t x;
  uint8_t y;
} GridCoordinate;

// commands defined as arrays of grid coordinates
const GridCoordinate MIDI_PANIC[]      = {{0, 0}, {0, 7}};
const GridCoordinate PAUSE_TOGGLE[]    = {{0, 0}, {1, 0}};
const GridCoordinate RECORD_TOGGLE[]   = {{0, 0}, {2, 0}};
const GridCoordinate TEMPO_MODE[]      = {{0, 1}, {1, 1}};
const GridCoordinate SHUFFLE_MODE[]    = {{0, 2}, {1, 2}};
const GridCoordinate STEP_MODE[]       = {{0, 3}, {1, 3}};
const GridCoordinate CHANNEL_MODE[]    = {{0, 4}, {1, 4}};
const GridCoordinate PITCH_MODE[]      = {{0, 5}, {1, 5}};
const GridCoordinate VELOCITY_MODE[]   = {{0, 6}, {1, 6}};

// start sequencer in record mode
bool record_mode = true;

// set command states to off by default
bool tempo_mode = false;
bool shuffle_mode = false;
bool pitch_mode = false;
bool velocity_mode = false;
bool channel_mode = false;
bool step_mode = false;

// keep pointers for selected buttons to operate
// on when in note and velocity mode
int  mode_position = 0;
bool position_selected = false;

 // button polling timer
unsigned long prevReadTime = 0L; 

// track the current touched buttons
uint16_t current_touched = 0;
