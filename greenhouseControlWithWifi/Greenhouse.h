#include <Servo.h>


class Greenhouse
{
  public:
    Greenhouse(int hPin, int sPin, float sPoint1, float sPoint2);
    int CheckTooHot(float temp);
    int CheckTooCold(float temp);
    bool CheckTooDry(int value);
    bool CheckOpen();
    //bool CheckClosed();
    void HeatItUpBaby();
    void TakeSomeTimeOffBaby();
    void CoolItDownBaby();
    void SoakItUpBaby(int pin);
    void DryItOffBaby();
    void openLid();
    void closeLid();

    int heatPin;
    int servoPin;
    //int coolPin;
    float setPoint1;
    float setPoint2;

    Servo myservo;
    
    int stateHeat = 0; //heaters start off
    int stateCool = 0; //cooling starts off
    //int statePump = 0; //pump starts off
  private:   
    
};

Greenhouse::Greenhouse(int hPin, int sPin, float sPoint1, float sPoint2)
{
  heatPin = hPin;
  setPoint1 = sPoint1;
  setPoint2 = sPoint2;
  servoPin = sPin;
  
}

int Greenhouse::CheckTooCold(float temp)
{
  int retVal = 0;

  if(stateHeat == 0 && stateCool == 0) //only if the heaters are off & cooling is off
  { 
    if(setPoint2 - 1 > temp)
    { 
      retVal = 1;
    }        
  }
  
  if(stateHeat == 0 && stateCool == 1) //only if the heaters are off & cooling is on
  { 
    if(setPoint2 > temp)
    { 
      retVal = 2;
    }        
  }
  return retVal;
}

int Greenhouse::CheckTooHot(float temp)
{
  int retVal = 0;
  
  if(stateHeat == 1 && stateCool == 0) //only if the heaters are on & coolers are off
  { 
    if(setPoint1 < temp)
    { 
      retVal = 1;
    } 
  }

  if(stateHeat == 0 && stateCool == 0) //only if the heaters are off & cooling is off
  { 
    if(setPoint2 + 1 < temp)
    { 
      retVal = 2;
    }        
  }
  
  return retVal;
}

void Greenhouse::HeatItUpBaby()
{
  //turn on the heaters
  digitalWrite(heatPin, HIGH);
  stateHeat = 1;
  stateCool = 0;
  if (CheckOpen())
  {
    closeLid();
  }
}

void Greenhouse::TakeSomeTimeOffBaby()
{
  //reset everything
  digitalWrite(heatPin, LOW);
  stateHeat = 0;
  stateCool = 0;
  if (CheckOpen())
  {
    closeLid();
  }
}
void Greenhouse::CoolItDownBaby()
{
  //open the lid
  digitalWrite(heatPin, LOW);
  stateHeat = 0;
  stateCool = 1;
  openLid();
  
}

bool Greenhouse::CheckTooDry(int value)
{
  if (value > 750)
    return true;
  
  return false;
}


void Greenhouse::SoakItUpBaby(int pin)
{
  EventTimer pumpTimer;
  pumpTimer.Start(5000);

  digitalWrite(pin, HIGH);

  while (!pumpTimer.CheckExpired()) {}

  digitalWrite(pin, LOW);  
}

//Servo Methods
void Greenhouse::closeLid()
{
  myservo.attach(servoPin);
  myservo.write(0); //closed
  delay(2000);
  myservo.detach();
}

void Greenhouse::openLid()
{
  myservo.attach(servoPin);
  myservo.write(180); //opened
  delay(2000);
  myservo.detach();
}

bool Greenhouse::CheckOpen()
{
  return myservo.read() != 0;
}
