#include "FifteenStep.h"
#include "Adafruit_NeoPixel.h"
#include "Wire.h"
#include "Adafruit_MPR121.h"

#define PIN      6
#define STEPS    16
#define TEMPO    60
#define BUTTONS 6

FifteenStep seq = FifteenStep();
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(STEPS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_MPR121 cap = Adafruit_MPR121();

uint16_t lasttouched = 0;
uint16_t currtouched = 0;
int channel = 0;

int mode_position = 0;
bool position_selected = false;

bool command_mode = false;
bool note_mode = false;
bool velocity_mode = false;

int notes[] = {60, 61, 62, 63, 64, 65};
int vel[] = {64, 64, 64, 64, 64, 64};

void setup() {
  
  delay(2000);
  
  // mpr121 irq
  pinMode(4, INPUT);
  
  pixels.begin();
  pixels.setBrightness(20);

  seq.begin(TEMPO, STEPS);
  seq.setMidiHandler(midi);
  seq.setStepHandler(step);
  
  if (!cap.begin(0x5A)) {
    while (1);
  }

}

void loop() {
  
  if(digitalRead(4) == LOW)
    readPads();
  
  seq.run();
  
}

void readPads() {  
  
  currtouched = cap.touched();
  
  switch(currtouched) {
    
    case 0x21:
      command_mode = false;
      note_mode = false;
      velocity_mode = false;
      flash(0,0,0);
      break;
    
    case 0x3:
      command_mode = command_mode ? false : true;
      note_mode = false;
      velocity_mode = false;
      flash(0,0,0);
      break;
      
    case 0x6:
      note_mode = note_mode ? false : true;
      command_mode = false;
      velocity_mode = false;
      flash(0,0,0);
      mode_position = 0;
      position_selected = false;
      break;
   
    case 0xC:
      velocity_mode = velocity_mode ? false : true;
      command_mode = false;
      note_mode = false;
      flash(0,0,0);
      mode_position = 0;
      position_selected = false;
      break;
    
    case 0x30: // panic. pads 5,6
      seq.panic();
      break;
      
    case 0x18: // pause. pads 4,5
      seq.pause();
      break;
          
    default:
    
      if(command_mode) {
        handle_command();
        break;
      } else if(note_mode) {
         handle_note_change();
         break;
      } else if(velocity_mode) {
         handle_velocity_change();
         break;
      }
    
      for (uint8_t i=0; i < BUTTONS; i++) {

        if ( (currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
          midi(channel, 0x9, notes[i], vel[i]);
          seq.setNote(channel, notes[i], vel[i]);
        }

        if ( !(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
          midi(channel, 0x8, notes[i], 0x0);
          seq.setNote(channel, notes[i], 0x0);
        }

      }
    
  }

  lasttouched = currtouched;
  
}

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
      show_range(channel, channel + 1, 128, 128, 128);
      break;
    case 0x20:
      // increase channel number
      channel = channel < 16 ? channel + 1 : 15;
      show_range(channel, channel + 1, 128, 128, 128);
      break;     
  }
  
}

void handle_note_change() {
  
  if(! position_selected) {
    get_position();
    return;
  }
  
  int last = notes[mode_position];
  
  if(currtouched == 0x1)
    notes[mode_position] = notes[mode_position] > 0 ? notes[mode_position] - 1 : 0;
  else if(currtouched == 0x2)
    notes[mode_position] = notes[mode_position] < 127 ? notes[mode_position] + 1 : 127;
    
  if(last != notes[mode_position])
    midi(channel, 0x9, notes[mode_position], vel[mode_position]);
  
  int display_notes = map(notes[mode_position], 0, 127, 0, 16);
  
  flash(0,0,0);
  show_range(0, display_notes, 0, 64, 0); 
  
}

void handle_velocity_change() {
  
  if(! position_selected) {
    get_position();
    return;
  }
   
   int last = vel[mode_position];

  if(currtouched == 0x1)
    vel[mode_position] = vel[mode_position] > 0 ? vel[mode_position] - 1 : 0;
  else if(currtouched == 0x2)
    vel[mode_position] = vel[mode_position] < 127 ? vel[mode_position] + 1 : 127;
    
  if(last != vel[mode_position])
    midi(channel, 0x9, notes[mode_position], vel[mode_position]);
  
  int display_velocity = map(vel[mode_position], 0, 127, 0, 16);
  
  flash(0,0,0);
  show_range(0, display_velocity, 0, 0, 64); 
    
}

void get_position() {
  
  for(int i=0; i < BUTTONS; i++) {
    
    if ( (currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      mode_position = i;
      position_selected = true;
    }
  
  }
  
}

void step(int current, int last) {
  
  if(note_mode && ! position_selected) {

    if((current % 2) == 0)
      flash(0,64,0);
    else
      flash(0,0,0);
    
  }
  
  if(velocity_mode && ! position_selected) {

    if((current % 2) == 0)
      flash(0,0,64);
    else
      flash(0,0,0);
      
  }
  
  if(velocity_mode || note_mode)
    return;
  
  if(command_mode) {
    
    if((current % 2) == 0)
      flash(64,0,0);
    else
      flash(0,0,0);
      
    return;
    
  }

  flash(0,0,0);
  
  if(current % 4 == 0)
    show_range(current, current + 1, 255, 0, 0);
  else
    show_range(current, current + 1, 0, 0, 64);
    
}

void flash(int r, int g, int b) {
  show_range(0, STEPS, r, g, b);
}

void show_range(int start, int last, int r, int g, int b) {
  
  for (; start < last; start++)
    pixels.setPixelColor(start, pixels.Color(r,g,b));
    
  pixels.show();

}

void midi(byte channel, byte command, byte arg1, byte arg2) {
  
  // combine command and channel for usb midi
  byte combined = command;
  
  // shift if necessary
  if(command < 16)
    combined = command << 4;
   
  // add channel
  combined |= channel;
  
  MIDIEvent event = {command, combined, arg1, arg2};
  MIDIUSB.write(event);
  MIDIUSB.flush();
  
}
