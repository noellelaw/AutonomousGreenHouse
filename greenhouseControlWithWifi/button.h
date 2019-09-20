class Button
{
  private:
    int pin = -1;
    int prevButtonState = HIGH;
  public:
    Button(int p, bool useIntPullup = true);
    bool CheckForPress();
    
};

Button::Button(int p, bool useIntPullup)
{
  pin = p;
  if (useIntPullup)
    pinMode(pin, INPUT_PULLUP);
  else
    pinMode(pin, INPUT);
}

bool Button::CheckForPress()
{
  bool retVal = false;
  
  int currButtonState = digitalRead(pin);
  if(prevButtonState != currButtonState)
  {
    delay(10); 
    if(currButtonState == LOW) 
      retVal = true;  //button is down => pin reads LOW
  }
  prevButtonState = currButtonState;

  return retVal;
}
