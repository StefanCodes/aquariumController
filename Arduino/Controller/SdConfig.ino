#include <Time.h>

time_t lightSystemOn[NUM_LIGHT_CIRCUITS];
time_t lightSystemOff[NUM_LIGHT_CIRCUITS];

#define SETTING_BUFFER_SIZE 40
char settingBuffer[SETTING_BUFFER_SIZE];

// loads the config from the SD card. If no config exists, defaults are loaded
void LoadConfigFromSd()
{
  String key;
  char keyArray[20];
  int i;

  for (i=0; i < NUM_LIGHT_CIRCUITS; i++)
  {
    key = "lightSystem"; 
    key += (i+1);
    key += "On";
    key.toCharArray(keyArray,20);
    if (SD.exists(keyArray))
    {
//      char* timeStr = ReadConfigKey(key);
    }
  }
  // Light system 1 ON
}

char* ReadConfigKey(char* configKey)
{
  int i;
  File file = SD.open(configKey, FILE_READ);
  ResetSettingBuffer();
  for (i=0; i < SETTING_BUFFER_SIZE && file.available(); i++)
  {
    settingBuffer[i] = (char)file.read();
  }
  file.close();
  return settingBuffer;
}

void ResetSettingBuffer()
{
  for (int i=0; i < SETTING_BUFFER_SIZE; i++)
  {
    settingBuffer[i] = '\0';
  }
}
