#include <LCD4Bit_mod.h> 
LCD4Bit_mod lcd = LCD4Bit_mod(2); 

// PH_GAIN is (4000mv / (59.2 * 7))
// 3960/59.2*7
// 4000mv is max output and 59.2 * 7 is the maximum range (in millivolts) for the ph probe. 
#define PH_GAIN 4.82625//9.6525
#define ARDUINO_VOLTAGE 5.03
#define MAGIC_CONSTANT 199.25
#define PH_PIN 5

char derp[20];

void setup()
{
  Serial.begin(9600);
  lcd.init();
  lcd.clear();
  pinMode(PH_PIN,INPUT);
}

void loop()
{
  int i, millivolts;
  long reading = 0;
  float ph_value;
  
  for(i = 1; i <= 50; i++)
  {
    reading += analogRead(PH_PIN);
    delay(10);
  }

  reading /= i;
  millivolts = (reading/MAGIC_CONSTANT)*1000;
  ph_value = millivolts / 59.2 / 4.0; 

  lcd.cursorTo(1,0);
  lcd.printIn(dtostrf(ph_value,2,2,derp));
//  lcd.cursorTo(2,0);
//  lcd.print(millivolts);
//  lcd.printIn("mV  ");
  delay(600); 
}


