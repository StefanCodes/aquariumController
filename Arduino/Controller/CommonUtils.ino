char* itoa2Digit(int number)
{
  char result[3];
  itoa(number, result, 10);
  if (number <= 9)
  {
    result[1] = result[0];
    result[0] = '0';
    result[2] = '\0';
  }
  return result;
}
