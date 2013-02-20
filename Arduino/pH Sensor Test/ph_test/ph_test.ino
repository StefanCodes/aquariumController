#include <LCD4Bit_mod.h> 
LCD4Bit_mod lcd = LCD4Bit_mod(2); 

// change this to whatever pin you&#39;ve moved the jumper to 
int ph_pin = 5; 
//int for the averaged reading
int reading; 
//int for conversion to millivolts 
int millivolts; 
//float for the ph value 
float ph_value; 
int i; 
// highly recommended that you hook everything up and check the arduino&#39;s voltage with a multimeter.
// It doesn&#39;t make that much of a difference, but 
// if you want it to be highly accurate than do this step
#define ARDUINO_VOLTAGE 5.02
// PH_GAIN is (4000mv / (59.2 * 7)) // 4000mv is max output and 59.2 * 7 is the maximum range (in millivolts) for the ph probe. 
#define PH_GAIN 9.6525 

void setup() { 
  pinMode(ph_pin,INPUT);
  lcd.init();
  lcd.clear();
} 

char derp[20];

void loop() { 
  reading = 0; 
  for(i = 1; i < 50; i++) { 
    reading += analogRead(ph_pin);
    delay(10);
  } 
  reading /= i; 
  millivolts = ((reading * ARDUINO_VOLTAGE) / 1024) * 1000; 
  ph_value = ((millivolts / PH_GAIN) / 59.2) + 7;
  lcd.clear();
  lcd.printIn(dtostrf(ph_value,2,2,derp));
  lcd.cursorTo(1,0);
  delay(500); 
}

