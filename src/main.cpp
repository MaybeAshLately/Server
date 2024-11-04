#include <Arduino.h>
#include <Wire.h>
#include <Narcoleptic.h>

//You can modify the following lines to match your application
const uint8_t interruptToSlavePin[]={6};
const uint8_t slaveAddresses[]={0};
const uint8_t numberOfSlaves=1;
const int interval=600;
const uint8_t ledPIN=8;
//-------------------------------------------------------------
uint8_t critcalNumberOfSignals;


uint8_t measBuffer[126];
unsigned long timeOfLastMeasurement=0;
unsigned long time=0;


void readDataFromSlave(uint8_t slaveAddress, uint8_t slavePin);
void performMeasurmentsFromSlaves();
void saveMeasurmentToFile(uint8_t slaveAddress);
void check();

void setup() {
  for(int i=0;i<numberOfSlaves;++i)
  {
    pinMode(interruptToSlavePin[i],OUTPUT);
    digitalWrite(interruptToSlavePin[i],LOW);
  }
  
  pinMode(ledPIN,OUTPUT);
  digitalWrite(ledPIN,LOW);

  Wire.begin();

  //for debbuging
  //Serial.begin(9600);
  //pinMode(4,INPUT_PULLUP);

  critcalNumberOfSignals=10;
  delay(100);
  performMeasurmentsFromSlaves();

}


void loop() {
   /*if(digitalRead(4)==LOW) //for debbuging - measurments triggered by switch on pin 4
   {
    readDataFromSlave(slaveAddresses[0],interruptToSlavePin);
   }*/
  

  Narcoleptic.delay(4000);
  long start=millis();
  delay(1000);
  if((timeOfLastMeasurement+interval)<=time)
  {
    timeOfLastMeasurement=time+5;
    performMeasurmentsFromSlaves();
  }
  long end=millis();
  
  time=time+5+((end-start)/1000);  
}


void performMeasurmentsFromSlaves()
{
  //Serial.println("meas");
  for(int i=0;i<numberOfSlaves;++i)
  {
    readDataFromSlave(slaveAddresses[i],interruptToSlavePin[i]);
    check();
    saveMeasurmentToFile(slaveAddresses[i]);
  }
}

void check()
{
  for(int i=0;i<126;++i)
  {
    if(measBuffer[i]>=critcalNumberOfSignals)
    {
      digitalWrite(ledPIN,HIGH);
      break;
    }
  }
}

//126 bytes = 32 + 32 + 32 + 30 (beacuse of I2C limit)
void readDataFromSlave(uint8_t slaveAddress, uint8_t slavePin)
{
    //Serial.println("low"); //for debbuging 
    //digitalWrite(ledPIN,HIGH); //for debbuging
    digitalWrite(slavePin,HIGH);

    int counter=0;
    for(int i=0;i<3;++i)
    {
      Wire.requestFrom(slaveAddress,(uint8_t)32);
      while(Wire.available() && counter<(32*(i+1)))
      {
        measBuffer[counter]=Wire.read();
        ++counter;
      }
    }

    Wire.requestFrom(slaveAddress,(uint8_t)30);
    while(Wire.available() && counter<126)
    {
      measBuffer[counter]=Wire.read();
      ++counter;
    }

    /*Serial.println("data recived"); //for debbuging
    Serial.flush();
    for(int i=0;i<126;++i)
    {
      Serial.print(measBuffer[i]);
      Serial.print(" ");
    }
    Serial.println();*/
   
   delay(500);
   digitalWrite(slavePin,LOW);
   //digitalWrite(ledPIN,LOW); //for debbuging
}


void saveMeasurmentToFile(uint8_t slaveAddress)
{

}