#include <Arduino.h>
#include <Wire.h>

uint8_t measBuffer[126];
const uint8_t ledPIN=8;
const uint8_t interruptToSlavePin=6;
const uint8_t slaveAddresses[]={0};
const uint8_t numberOfSlaves=1;

void readDataFromSlave(uint8_t slaveAddress, uint8_t slavePin);

void setup() {
  pinMode(interruptToSlavePin,OUTPUT);
  digitalWrite(interruptToSlavePin,LOW);
  pinMode(ledPIN,OUTPUT);
  digitalWrite(ledPIN,LOW);

  Wire.begin();

  //debbuging
  Serial.begin(9600);
  pinMode(4,INPUT_PULLUP);

}


void loop() {
   if(digitalRead(4)==LOW) //to debug, change later
   {
    readDataFromSlave(slaveAddresses[0],interruptToSlavePin);
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
      Wire.requestFrom(slaveAddress,32);
      while(Wire.available() && counter<(32*(i+1)))
      {
        measBuffer[counter]=Wire.read();
        ++counter;
      }
    }

    Wire.requestFrom(slaveAddress,30);
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