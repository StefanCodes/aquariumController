


char* serializeTime(time_t time)
{
  char result[5];
  char cHour[2];
  char cMinute[2];
  int mins = time.minute();
  itoa2Digit(mins);
  itoa2Digit(time.hour);
  result[0] = cHour[0];
  result[1] = cHour[1];
  result[2] = cMinute[0];
  result[3] = cMinute[1];
  result[4] = '\0';
  return result;
}

time_t DeserializeTime(String time)
{
  time_t newTime;
}







