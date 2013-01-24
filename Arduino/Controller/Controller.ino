/***************************************************
 * Author: Stefan Mohr (stefan@stefanmohr.com)
 * License: This software is released into the public domain without license.
 * 
 * This Arduino sketch implements an aquarium controller. It expects an aquarium system
 * with a light fixture with 2 separate circuits (wired to 110V outlet box and switched by 
 * relay by Arduino), a (PH/KH) dosing pump (diaphragm style, powered by an independent power supply
 * and switched by relay), a pH sensor (connected via an a BNC pH Sensor Shield: 
 * http://www.robotshop.com/ca/productinfo.aspx?pc=RB-Pra-06&lang=en-US), a temperature 
 * probe (connected to Arduino's analog input pins), and a solenoid controlling pressurized CO2
 * (wired to 110V outlet and switched by relay on Arduino).
 * 
 * The sketch is assumes a very strict and limited serial communication protocol with
 * a monitoring device (in this case a Raspberry Pi connected via its serial pins).
 * 
 ** Note that you cannot directly connect an Arduino and Raspberry Pi's serial pins!
 * The Arduino signals at 5V whereas the Raspberry Pi signals at 3.3V. If directly connected,
 * the 5V transmitted by the Arduino will burn out the serial port on the Raspberry Pi.
 * I built a simple level converter as laid out in: http://www.fritz-hut.com/connecting-an-arduino-and-raspberry-pi/
 * The circuit can fit on a small circuit board and I will etch a matchbook-sized PCB and try to
 * fit it inside the relatively tight Raspberry Pi case.
 * 
 * 
 ***************************************************/

// Configurable globals


// Default constants
#define BAUD_RATE 19200
#define COMMANDBUFFERSIZE 4 // the maximum length of a command sent by a serial controller

// Some sanity checking constants to ensure the controller can't force the tank to dangerous levels
#define MIN_ALLOWABLE_TARGET_PH 6.0 // minimum target pH the Arduino will accept
#define MAX_ALLOWABLE_TARGET_PH 7.2 // maximum target pH the Arduino will accept
#define MAX_ALLOWABLE_ACTION_PH_DRIFT 0.5 // the maximum drift in pH the Arduino will allow 

/* 
 Light Circuits (GL command: Get Lights)
 A light circuit is relay controlled by the Arduino. 
 It's assumed the Raspberry Pi monitor knows which light circuits to poll.
 1: Full day lights. 2x54W T5HO
 2: Peak daytime lights. 2x54W T5HO
 
 Dosing Pump (GD command: Get Dosing)
 A dosing pump is relay controlled by the Arduino. 
 There is only one dosing pump in the system.
 
 */

/* Supported serial commands: (sent from Raspberry Pi to the Arduino)
 
 GET Commands
 GET commands are triggered by a multi-character command received on the serial port. GET commands begin
 with a 'G'. The second letter indicates which aquarium system to poll (lighting, CO2, etc)
 
 Command    Result                                          Example Response
 Light
 GLP#       The on/off state of light fixture # (1 or 2)    OFF
 GLT#N      The time light fixture # turns the lights on    0930 (24 hour clock)
 GLT#F      The time light fixture # turns the lights off   2200 (24 hour clock)
 GLS#       The light snooze timer for fixture #            0100 (HHMM format)
 pH
 GPV        pH value detected on pH probe                   7.00
 GPT        Target pH (#.##)                                7.00
 GPA        Action pH threshold                             0.20
 GPW        Warning pH threshold                            0.50
 Dosing
 GD        The on/off status of the dosing pump.            ON
 // TODO: Can we measure the speed of the dosing pump? Should we?
 CO2
 GC        CO2 injection on/off state                       OFF
 Clock
 GR      RTC clock time                                     20130214233000
 Temperature
 GTV    Current temperature sensor value in degrees Celsius. 23.81
 
 
 SET Commands
 SET commands allow the behaviour of the controller to be modified by the Raspberry Pi. SET commands begin
 with an 'S'. The second letter indicates which aquarium system to affect, and the third (or more) indicate the new settings.
 Command    Effect                                      Example
 Light
 SLP#1      Powers on light fixture # (1 or 2)          SL11 (turn on light 1)
 SLP#0      Powers off light fixture # (1 or 2)         SL20 (turn off light 2)
 SLT#N####  Sets light timer for fixture # to turn oN   SLT1M0800 (set light 1 to turn on at 8AM)
 SLT#F####  Sets light timer for fixture # to turn oFf  SLT2N2200 (sets light 2 to turn off at 10PM)
 SLS#####   Sets the light snooze timer for fixture #   SLS20130 (sets light 2 to snooze for 1 hour 30 mins)
 pH
 SPT#.##    Sets target pH to #.##.                     SPT6.85 (set target pH to 6.85)
 SPA#.##    Sets action pH threshold to #.##            SPA0.20 (set action pH to +/- 0.20 of the target pH. 6.65-7.05 using the example above)
 SPW#.##    Sets warning pH threshold to #.##           SPW1.00 (set warning pH to +/- 1.00 of the target pH. 5.85-7.85 using the example above)
 Dosing
 SD1        Turns the dosing pump on.
 SD0        Turns the dosing pump off.
 // TODO: Can we control the speed of the dosing pump? Should we?
 CO2
 SC1        Turns CO2 injection on.
 SC0        Turns CO2 injection off.
 Clock
 SRT#####   Sets the RTC clock time (YYYYMMDDHHMMSS)    SRT20120118231100 (sets Arduino clock time to Jan 18, 2012, 11:11:00PM)
 
 
 
 Light Control
 Lights are controlled by start and end times. The Arduino controls relays which gives us a lighting timer. Each light fixture
 can have its on and off times programmed independently. Each light fixture can also be forced on or off. The snooze setting
 lets you configure how long the lights will stay on (or off) during the times they've been forced outside their schedule.
 Put another way, if an SLP#1 command is recieved in the middle of the night, the lights will turn on. However, since their
 regular schedule is to be turned off at night, the lights can only remain on as long as the snooze time. If set to 1 hour,
 the lights that were activated in the middle of the night will turn back off in an hour. Snooze can be disabled by setting it to 0.
 
 Target pH and Warning pH
 The target pH is a global variable on the Arduino. The main loop constantly tests the pH, and if the pH wanders a certain 
 direction, we control various physical devices to correct it. The target pH is the pH level that the Arduino tries to maintain
 by controlling pressurized CO2 injection (drops pH), and a dosing pump of carbonate (increases pH and KH).
 If the warning pH is reached, CO2 injection stops completely (the pH is crashing - the CO2 is a contributor).
 The action pH is the pH level at which we put the dosing pump into action. The action pH could be the same as the target pH, but
 this would cause the dosing pump to be constantly pulsing on and off. I prefer to let the pH drift a bit and then turn on the pump
 for a longer time.
 My desired aquarium behaviour is:
 * pH is at my target pH.
 * CO2 runs constantly, lowering my pH.
 * pH eventually drops to the action pH level.
 * Dosing pump is switched on and begins pumping a carbonate buffer that will boost pH and KH
 * pH rises until it reaches the target pH
 * Dosing pump shuts off.
 * pH eventually drops to the action pH level (and on and on and on)
 
 There's also the safety measure behaviour in case this happens:
 * CO2 runs constantly, lowering my pH
 * pH drops to action pH level and dosing pump turns on
 * pH continues to drop despite buffer and drops to warning level
 * CO2 injection is turned off, regardless of other programming.
 * pH rises with carbonate buffer and no CO2 injection to compete with it
 (CO2 injection will not begin again until the pH reached the target pH level)
 
 The converse occurs as well. If the pH begins rising too high, CO2 will remain on (regardless of time of day - but perhaps that
 means at night we should power an airstone with another relay...) and the dosing pump will be kept off.
 
 It's important to note that the Arduino will accept SET commands at any time, but the settings will be ignored if
 any parameters are at warning levels. For example, if the pH is crashing below (target pH)-(warning pH), a command
 to turn on CO2 injection will be ignored as unsafe. It's generally intended that the Arduino will not be ordered around by 
 a controller, but will instead be given parameters to make decisions (such as target pH and action pH levels). As such,
 the Arduino's internal safeties take precedent over a (potentially untrustworthy) Raspberry Pi.
 
 
 
 
 */

#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX

void setup()  
{
  mySerial.begin(BAUD_RATE);
  mySerial.println("Startup");
}

char commandBuffer[COMMANDBUFFERSIZE]; // the buffer for a command recieved by a serial controller (Raspberry Pi)
int commandBufferIndex;
void loop()
{
  for (commandBufferIndex = 0; commandBufferIndex < COMMANDBUFFERSIZE; commandBufferIndex++){
    commandBuffer[commandBufferIndex] = '\0';
  }
  if (mySerial.available()){
    // messages can be terminated by '~' to be processed immediately, or just wait 1000ms for readBytesUntil to timeout
    mySerial.readBytesUntil('~', commandBuffer, COMMANDBUFFERSIZE);

    // commandBuffer now contains an ASCII command.
    switch (commandBuffer[0]){
    case 'G':
    case 'g':
      Get(commandBuffer);
      break;
    case 'S':
    case 's':
      Set(commandBuffer);
      break;
    default:
      break;
    }
  }
}


// Sends the result message to the serial partner (Raspberry Pi).
void sendSerial(char message[]){
  mySerial.println(message);
}


// Sends the current pH reading as an ASCII string (e.g. 6.85)
void sendPh(char probeNum){
  // add support for more probes if you want
  // TODO: implement code to read pH from probe and format into an ASCII string
  sendSerial("6.85");
}


void sendDosingPumpState(char pumpNum){
  //  add support for more pumps if you want
  // TODO: implement code to tap into dosing pump status. none of this code exists yet.
  sendSerial("OFF");
}

// Sends the current status of the lights
void sendLights(char lightCircuitNum){
  // TODO: Get the relay shield...
  if (lightCircuitNum == '1'){
    // figure out if relay is powered. return ON or OFF if NO relay is closed
    sendSerial("ON");
  }
  else if (lightCircuitNum == '2'){
    // figure out if relay is powered. return ON or OFF if NO relay is closed
    sendSerial("OFF");
  }
}












