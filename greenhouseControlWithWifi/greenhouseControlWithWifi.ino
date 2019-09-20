//WIFI

#define Serial SerialUSB

//////////////////////
// Library Includes //
//////////////////////
// SoftwareSerial is required (even you don't intend on
// using it).
#include <SparkFunESP8266WiFi.h>

//////////////////////////////
// WiFi Network Definitions //
//////////////////////////////
// Replace these two character strings with the name and
// password of your WiFi network.
const char mySSID[] = "wahoo";
const char myPSK[] = "";

//////////////////
// HTTP Strings //
//////////////////
const char destServer[] = "34.209.142.24";
const char sHttpRequest[] = "GET /~dmk3pc/set_setpoints.php";   //just call the basic webpage
const char gHttpRequest[] = "GET /~dmk3pc/get_setpoints.php";

// All functions called from setup() are defined below the
// loop() function. They modularized to make it easier to
// copy/paste into sketches of your own.




//GREENHOUSE
#include <DHT.h>

#define SENSOR_INTERVAL 500UL

#define DHT_PIN_1 8
DHT dht_I(DHT_PIN_1, DHT22);

#define DHT_PIN_2 9
DHT dht_O(DHT_PIN_2, DHT22);

#include <SPI.h>
#include "button.h"
#include "EventTimer.h"
#include "Greenhouse.h"

int csPin = 10; 
int upPin = 2;
int downPin = 3;
int heatPin = 5;
int capPin = A0;
int pumpPin = 7;
int servoPin = 4;

Button upButton(upPin);
Button downButton(downPin);

const uint32_t READ_INTERVAL = 500;

EventTimer timer;

Greenhouse greenhouse(heatPin, servoPin, 20, 30); //GCL: separate class is nice

//GCL: why all the global strings?
String dhtIString;
String dhtOString;
String rhString;
String heaterString;
String string1;
String string2;
boolean lcdSet = true;

//GCL: wait. you have a greenhouse class -- why are these declared global?
float dhtITemperature;
float dhtIHumidity;
float dhtOTemperature;
float dhtOHumidity;
int capMoisture;

void setup() 
{
  //WIFI
  // Serial Monitor is used to control the demo and view
  // debug information.
  //Serial.begin(57600);
  delay(2000); //give the serial a chance to come up
  //while(!Serial) {} //remove for headless...

  Serial1.begin(57600);

  // initializeESP8266 verifies communication with the WiFi
  // shield, and sets it up.
  initializeESP8266();

  // connectESP8266 connects to the defined WiFi network.
  connectESP8266();

  // displayConnectInfo prints the Shield's local IP
  // and the network it's connected to.
  displayConnectInfo();

  

  //Greenhouse
  //GCL: This could all go to an Init() function in Greenhouse
  pinMode(csPin, OUTPUT);
  pinMode(greenhouse.heatPin, OUTPUT);
  
  digitalWrite(csPin, HIGH); //By default, don't be selecting OpenLCD

  SPI.begin(); //Start SPI communication
  //SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  SPI.setClockDivider(SPI_CLOCK_DIV128); //Slow down the master a bit

  dht_I.begin();
  dht_O.begin();

  pinMode(pumpPin, OUTPUT);//Set 7 as an OUTPUT
  digitalWrite(pumpPin, LOW);//Set to LOW so no power is flowing through the sensor

  timer.Start(5000);

}

bool buttonOverride = false;

void loop() 
{
  //GREENHOUSE
  bool forceUpdate = false;
  

  if (upButton.CheckForPress())
  {
    if (digitalRead(downPin) == 0)
      buttonOverride = false;
    else
    {
      greenhouse.setPoint1 += 0.5; //sets heating set point
      greenhouse.setPoint2 += 0.5; //sets cooling set point relative to heating
      forceUpdate = true;
      buttonOverride = true;
    }
  }

  if (downButton.CheckForPress())
  {
    if (digitalRead(upPin) == 0)
      buttonOverride = false;
    else
    {
      greenhouse.setPoint1 -= 0.5; //sets heating setpoint
      greenhouse.setPoint2 -= 0.5; //sets cooling setpoint relative to heating
      forceUpdate = true;
      buttonOverride = true;
    }
  }
  
  


  if (timer.CheckExpired() || forceUpdate) //GCL: the forceUpdate is nice --> we don't have to wait for a timer to expire
  {
      if (dht_I.read())
      {
        dhtITemperature = dht_I.CalcTemperature();
        dhtIHumidity = dht_I.CalcHumidity();
      }
    
      if (dht_O.read())
      {
        dhtOTemperature = dht_O.CalcTemperature();
        dhtOHumidity = dht_O.CalcHumidity();
      }
    
      capMoisture = analogRead(capPin);
      
      if (greenhouse.CheckTooCold(dhtITemperature) == 1)
      {
        //turn on heaters
        greenhouse.HeatItUpBaby();
      }

      if (greenhouse.CheckTooCold(dhtITemperature) == 2)
      {
        //close lid
        greenhouse.TakeSomeTimeOffBaby();
      }
           
      if (greenhouse.CheckTooHot(dhtITemperature) == 1)
      {
        //turn off heaters
        greenhouse.TakeSomeTimeOffBaby();
      }

      if (greenhouse.CheckTooHot(dhtITemperature) == 2)
      {
        //open lid
        greenhouse.CoolItDownBaby();
      }

      if (greenhouse.CheckTooDry(capMoisture))
      {
        //water plant
        greenhouse.SoakItUpBaby(pumpPin);
      }
  
      if (greenhouse.stateHeat == 1)
      {
        heaterString = "Y";
      }
      else
      {
        heaterString = "N";
      }

     

     dhtIString = String("I: ") + 
                  String(dhtITemperature, 1) + String("C ");
     dhtOString = String("O: ") + 
                  String(dhtOTemperature, 1) + String("C ");
     rhString = String(dhtIHumidity, 1) + String("%RH");


     string1 = dhtIString + rhString + dhtOString + "Heat: " + heaterString;
     string2 = "Heat SP: " + String(greenhouse.setPoint1, 1) + "C  Cool SP: " + String(greenhouse.setPoint2, 1) + "C  "; 

     if (lcdSet) //GCL: good to use this variable
     {
        handleLCD(string1, "");
        lcdSet = !lcdSet;
     }
     else
     {
        handleLCD(string2, "");
        lcdSet = !lcdSet;
     }
     
     
     
     timer.Start(5000);

  }


  


  
  //WIFI
    static unsigned long last = 5000;
    if(millis() > last)
    {
      
      
      //Get SP
      if (!buttonOverride)
        getSetPoints();
  
      
      //Reading
      sendReadingToDB(1, dhtITemperature);
      sendReadingToDB(2, dhtOTemperature);
  
  
      //Open Close
      if (greenhouse.stateCool == 1)
        sendReadingToDB(3, 16);
      else
        sendReadingToDB(3, 0);
  
      
      //Send SP
      sendReadingToDB(4, greenhouse.setPoint1);
      sendReadingToDB(5, greenhouse.setPoint2);

      //Send Heater Power
      if (greenhouse.stateHeat == 1)
         sendReadingToDB(6, 24);
      else
         sendReadingToDB(6, 0);

      //Send %RH
      sendReadingToDB(7, dhtIHumidity);  

      //Send Moisture
      sendReadingToDB(8, capMoisture);
      
      
      
  
      
      last += 30000;   //call the server every 30 seconds
    }
    
}


//Greenhouse Methods
//Sends a string over SPI
void spiSendString(char* data)
{
  digitalWrite(csPin, LOW); //Drive the CS pin low to select OpenLCD
  for(byte x = 0 ; data[x] != '\0' ; x++) //Send chars until we hit the end of the string
    SPI.transfer(data[x]);
  digitalWrite(csPin, HIGH); //Release the CS pin to de-select OpenLCD
}

void handleLCD(String temp, String other)
{
  //Send the clear display command to the display - this forces the cursor to return to the beginning of the display
    digitalWrite(csPin, LOW); //Drive the CS pin low to select OpenLCD
    SPI.transfer('|'); //Put LCD into setting mode
    SPI.transfer('-'); //Send clear display command
    digitalWrite(csPin, HIGH); //Release the CS pin to de-select OpenLCD
  
      
    spiSendString(temp.c_str());
    spiSendString(other.c_str());
    //Serial.println("update");
    
}





//Esp Methods
void sendReadingToDB(int sensor_id, double value)
{
  // To use the ESP8266 as a TCP client, use tcpConnect()
  
  // tcpConnect([server], [port]) is used to 
  // connect to a server (const char * or IPAddress) on
  // a specified port.
  // Returns: 1 on success, 2 on already connected,
  // negative on fail (-1=TIMEOUT, -3=FAIL).
  int retVal = esp8266.tcpConnect(destServer, 80);
  if (retVal <= 0)
  {
    Serial.print(retVal);
    Serial.println(F(" -- Failed to connect to server."));
    return;
  }

  Serial.print(F("Checking connection: ")); //just to verify
  Serial.println(esp8266.connected());

  // the next lines show the general workflow: begin, send, end
  esp8266.tcpBeginTransmission();   //library call to start transmission
  esp8266.tcpSendPacket("GET /~dmk3pc/insert.php"); //the web page we want


  //here's where you'll add fields:
  esp8266.tcpSendPacket(String("?id=") + String(sensor_id));
  esp8266.tcpSendPacket(String("&value=") + String(value));
  //esp8266.tcpSendPacket("&firstName=Nole");
  //etc., etc.

  
  esp8266.tcpSendPacket(" HTTP/1.1\r\nHost: "); //the beginning of the "footer"
  esp8266.tcpSendPacket(destServer); //this tells the interwebs to route your request through the AWS server
  esp8266.tcpSendPacket("\r\nConnection: close\r\n\r\n"); //tells the server to close the cxn when done. \r\n\r\n tells it we're done
  esp8266.tcpEndTransmission();     //library call to close transmission on our end

  Serial.println(F("Request sent."));

  int16_t rec = 1;

  //check for a response. rec holds the number of bytes in each packet: negative numbers are errors
  //when the server is done, tcpReceive will timeout and return 0 (no bytes), in which case, we're done
  while(rec > 0)
  {
    char recBuffer[128];
    rec = esp8266.tcpReceive(recBuffer, 128); //returns the number of bytes received; recBuffer holds a line

    //print the number of characters received (just for debugging -- comment out when using for realsies)
    Serial.print(rec);
    Serial.print(':');

    if(rec > 0) //if we got data, print it to the Serial Monitor
    {
      Serial.print(recBuffer); 
    }
  }

  Serial.println("Done."); 

  // connected() is a boolean return value: 1 if the 
  // connection is active, 0 if it's closed.
  if (esp8266.connected())
  {
    Serial.println(F("Closing."));  
    esp8266.close(); //explicitly close the TCP connection.
  }
}



void getSetPoints()
{
  
  int retVal = esp8266.tcpConnect(destServer, 80);
  if (retVal <= 0)
  {
    Serial.print(retVal);
    Serial.println(F(" -- Failed to connect to server."));
    return;
  }

  Serial.print(F("Checking connection: ")); //just to verify
  Serial.println(esp8266.connected());

  // the next lines show the general workflow: begin, send, end
  esp8266.tcpBeginTransmission();   //library call to start transmission
  esp8266.tcpSendPacket(gHttpRequest); //the web page we want


  
  esp8266.tcpSendPacket(" HTTP/1.1\r\nHost: "); //the beginning of the "footer"
  esp8266.tcpSendPacket(destServer); //this tells the interwebs to route your request through the AWS server
  esp8266.tcpSendPacket("\r\nConnection: close\r\n\r\n"); //tells the server to close the cxn when done. \r\n\r\n tells it we're done
  esp8266.tcpEndTransmission();     //library call to close transmission on our end

  Serial.println(F("Request sent."));

  int16_t rec = 1;


  //String r = "";
  //check for a response. rec holds the number of bytes in each packet: negative numbers are errors
  //when the server is done, tcpReceive will timeout and return 0 (no bytes), in which case, we're done
  
  while(rec >= 0)
  {
    char recBuffer[128];
    rec = esp8266.tcpReceive(recBuffer, strlen(recBuffer)-1); //returns the number of bytes received; recBuffer holds a line
    
    //print the number of characters received (just for debugging -- comment out when using for realsies)
    Serial.print(rec);
    Serial.print(':');

    

    if(rec > 0) //if we got data, print it to the Serial Monitor
    {
      Serial.println(recBuffer);
      
      String myBuffer = String(recBuffer);
      myBuffer.trim();
      //Serial.println(myBuffer);
      
      //php scrip should return something like <setp>24,32</setp>
      
      if (myBuffer.indexOf("<hsp>") >= 0)
      {
        String heatString = myBuffer.substring(myBuffer.indexOf("<hsp>") + 5, myBuffer.indexOf("</hsp>"));
        //Serial.println("Heat setpoint: " + heatString);
        greenhouse.setPoint1 = heatString.toInt();
        //Serial.println(greenhouse.setPoint1);
      }
      if (myBuffer.indexOf("<csp>") >= 0)
      {
        String coolString = myBuffer.substring(myBuffer.indexOf("<csp>") + 5, myBuffer.indexOf("</csp>"));
        //Serial.println("Cool setpoint: " + coolString);
        greenhouse.setPoint2 = coolString.toInt();
        //Serial.println(greenhouse.setPoint2);
      }
    }
    
  }
  

  Serial.println("Done."); 

  // connected() is a boolean return value: 1 if the 
  // connection is active, 0 if it's closed.
  if (esp8266.connected())
  {
    Serial.println(F("Closing."));  
    esp8266.close(); //explicitly close the TCP connection.
  }
}


















//Other
void initializeESP8266()
{
  // esp8266.begin() verifies that the ESP8266 is operational
  // and sets it up for the rest of the sketch.
  // It returns either true or false -- indicating whether
  // communication was successul or not.
  // true
  int test = esp8266.begin(&Serial1);
  if (test != true)
  {
    Serial.println(F("Error talking to ESP8266."));
    errorLoop(test);
  }
  Serial.println(F("ESP8266 Shield Present"));
}

void connectESP8266()
{
  // The ESP8266 can be set to one of three modes:
  //  1 - ESP8266_MODE_STA - Station only
  //  2 - ESP8266_MODE_AP - Access point only
  //  3 - ESP8266_MODE_STAAP - Station/AP combo
  // Use esp8266.getMode() to check which mode it's in:
  int retVal = esp8266.getMode();
  if (retVal != ESP8266_MODE_STA)
  { // If it's not in station mode.
    // Use esp8266.setMode([mode]) to set it to a specified
    // mode.
    retVal = esp8266.setMode(ESP8266_MODE_STA);
    if (retVal < 0)
    {
      Serial.println(F("Error setting mode."));
      errorLoop(retVal);
    }
  }
  Serial.println(F("Mode set to station"));

  char macAddr[24]; //???
  esp8266.localMAC(macAddr);
  Serial.println(macAddr);

  // esp8266.status() indicates the ESP8266's WiFi connect
  // status.
  // A return value of 1 indicates the device is already
  // connected. 0 indicates disconnected. (Negative values
  // equate to communication errors.)
  retVal = esp8266.status();
  
  if (retVal == 5) //ick
  {
    Serial.print(F("Connecting to "));
    Serial.println(mySSID);
    // esp8266.connect([ssid], [psk]) connects the ESP8266
    // to a network.
    // On success the connect function returns a value >0
    // On fail, the function will either return:
    //  -1: TIMEOUT - The library has a set 30s timeout
    //  -3: FAIL - Couldn't connect to network.
    retVal = esp8266.connect(mySSID, myPSK);
  }
  
  if (retVal < 0)
  {
    Serial.println(F("Error connecting"));
    errorLoop(retVal);
  }

  retVal = esp8266.status();
}

void displayConnectInfo()
{
  char connectedSSID[24];
  memset(connectedSSID, 0, 24);
  // esp8266.getAP() can be used to check which AP the
  // ESP8266 is connected to. It returns an error code.
  // The connected AP is returned by reference as a parameter.
  int retVal = esp8266.getAP(connectedSSID);
  if (retVal > 0)
  {
    Serial.print(F("Connected to: "));
    Serial.println(connectedSSID);
  }

  // esp8266.localIP returns an IPAddress variable with the
  // ESP8266's current local IP address.
  IPAddress myIP = esp8266.localIP();
  Serial.print(F("My IP: ")); Serial.println(myIP);
}

// errorLoop prints an error code, then goes into serial passthrough mode for debugging.
void errorLoop(int error)
{
  Serial.print(F("Error: ")); Serial.println(error);
  Serial.println(F("Entering debug mode."));
  while(1)
  {
    while (Serial.available())
    {
      char ch = Serial.read();
      Serial.print(ch);
      Serial1.write(ch);
    }
  
    while (Serial1.available())
    {
      char ch = Serial1.read();
      Serial.write(ch);
    }
  }
}
