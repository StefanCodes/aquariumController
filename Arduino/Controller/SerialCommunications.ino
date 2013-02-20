// Highest level operator for a GET. Processes the second character of the command, which specifies the aquarium system to query.
char* FsmGet()
{
  switch(commandBuffer[1])
  {
  case 'L': // Lighting
    return FsmGetLightStatus();
  case 'P': // pH sensor
    return FsmGetPhStatus();
  case 'D': // Dosing pump
    return FsmGetDosingStatus();
  case 'C': // CO2 system
    return FsmGetCo2Status();
  case 'R': // Realtime clock
    return FsmGetRtc();
  case 'T': // Temperature sensor
    return FsmGetTemperatureStatus();
  default:
    return UNKNOWN_GET;
  }
}

char* FsmGetLightStatus()
{
  switch(commandBuffer[2])
  {
    // GLP#
  case 'P':
    int lightFixtureNum = 0;
    if (commandBuffer[3] == '1'){
      return "ON";
    }
    else if (commandBuffer[3] == '2'){
      return "OFF";
    }
    else
      return UNKNOWN_GET_LIGHT;
  }
}

char* FsmGetPhStatus()
{
  return dtostrf(GetPh(),2,2,phHolder);
}

float GetPh()
{
  // recode a
}

char* FsmGetDosingStatus()
{
  return "ON";
}

char* FsmGetCo2Status()
{
  return "OFF";
}

// returns the date as YYYYMMDDHHMMSS (24 hour time)
char* FsmGetRtc()
{
  char result[15];
  String holder;
  int i = 0;
  result[i++] = '2';
  result[i++] = '0';

  holder = itoa2Digit(RTC.getYear());
  result[i++] = holder[0];
  result[i++] = holder[1];
  holder = itoa2Digit(RTC.getMonth());
  result[i++] = holder[0];
  result[i++] = holder[1];
  holder = itoa2Digit(RTC.getDate());
  result[i++] = holder[0];
  result[i++] = holder[1];
  holder = itoa2Digit(RTC.getHours());
  result[i++] = holder[0];
  result[i++] = holder[1];
  holder = itoa2Digit(RTC.getMinutes());
  result[i++] = holder[0];
  result[i++] = holder[1];
  holder = itoa2Digit(RTC.getSeconds());
  result[i++] = holder[0];
  result[i++] = holder[1];
  result[i] = '\0';
  return result;
}



char* FsmGetTemperatureStatus()
{
  return "24.6";
}

char* FsmSet()
{
  switch(commandBuffer[1])
  {
  case 'L': // Lights
    return FsmSetLights();
  case 'P': // pH sensor
    return FsmSetPh();
  case 'D': // Dosing pump
    return FsmSetDosing();
  case 'C': // CO2
    return FsmSetCo2();
  case 'R': // clock
    return FsmSetRtc();
  case 'T': // Temperature
    return FsmSetTemperature();
  default:
    return UNKNOWN_SET;
  }
}

char* FsmSetLights()
{
  //  int lightNum = atoi(commandBuffer[2]);
  //  if (lightNum == 0)
  return UNKNOWN_SET_LIGHT;

}

char* FsmSetPh()
{
}

char* FsmSetDosing()
{
}

char* FsmSetCo2()
{
}

char* FsmSetRtc()
{
  // command is something like SR20130427235900 (YYYYMMDDHHMMSS)
  char holder[3];
  holder[2] = '\0';
  int i=4;
  holder[0] = commandBuffer[i++];
  holder[1] = commandBuffer[i++];
  RTC.setYear(atoi(holder));
  holder[0] = commandBuffer[i++];
  holder[1] = commandBuffer[i++];
  RTC.setMonth(atoi(holder));
  holder[0] = commandBuffer[i++];
  holder[1] = commandBuffer[i++];
  RTC.setDate(atoi(holder));
  holder[0] = commandBuffer[i++];
  holder[1] = commandBuffer[i++];
  RTC.setHours(atoi(holder));
  holder[0] = commandBuffer[i++];
  holder[1] = commandBuffer[i++];
  RTC.setMinutes(atoi(holder));
  holder[0] = commandBuffer[i++];
  holder[1] = commandBuffer[i++];
  RTC.setSeconds(atoi(holder));
  RTC.setClock();
  return "OK";
}

char* FsmSetTemperature()
{
}



