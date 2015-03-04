#include "FifteenStep.h"
#include "Adafruit_NeoPixel.h"
#include "Wire.h"
#include "Adafruit_MPR121.h"

#define PIN      6
#define STEPS    16
#define TEMPO    60

FifteenStep seq = FifteenStep();
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(STEPS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_MPR121 cap = Adafruit_MPR121();

uint16_t lasttouched = 0;
uint16_t currtouched = 0;
int base = 60; // middle c

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
    
    case 0x38: // panic. pads 4,5,6
      seq.panic();
      break;
      
    default:
    
      for (uint8_t i=0; i<6; i++) {

        if ( (currtouched & _BV(i)) && !(lasttouched & _BV(i)) )
          seq.setNote(0x0, base + i, 0x40);

        if ( !(currtouched & _BV(i)) && (lasttouched & _BV(i)) )
          seq.setNote(0x0, base + i, 0x00);

      }
    
  }

  lasttouched = currtouched;
  
}

void step(int current, int last) {
  
  int r = 0;
  int g = 0;
  int b = 0;
  
  pixels.setPixelColor(last, pixels.Color(0,0,0));
  
  if(current % 4 == 0)
    r = 255;
  else if(current % 2 == 0)
    b = 128;
  else
    b = 64;
    
  pixels.setPixelColor(current, pixels.Color(r,g,b));

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
