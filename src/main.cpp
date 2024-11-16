#include <Arduino.h>
#include <Wire.h>
#include <Narcoleptic.h>
#include <SPI.h>
#include <SD.h>

//You can modify the following lines to match your application
const uint8_t interruptToSlavePin[]={6};
const uint8_t slaveAddresses[]={8};
const uint8_t numberOfSlaves=1;
const int interval=600;
const uint8_t ledPIN=8;
//-------------------------------------------------------------
uint8_t critcalNumberOfSignals;


uint8_t measBuffer[130]; // 126 bytes for measurment and last 4 for time
uint32_t timeOfLastMeasurement=0;
uint32_t time=0;
const uint8_t CSPin=10;

uint8_t messageBuffer[20];


void readDataFromSlave(uint8_t slaveAddress, uint8_t slavePin);
void performMeasurmentsFromSlaves();
void saveMeasurmentToFile(uint8_t slaveAddress);
void check();
void serialEvent();
void handleRecivedMessage();
void sendListOfSlaves();
void endAlarm();
void sendLatestMeasurement(uint8_t slaveAddress);
void clearHistoricData(uint8_t slaveAddress);
void sendHistoricData(uint8_t slaveAddress);
void setNewCritcalNumberOfSignals();
void sendAck();

void setup() {
  
  if (!SD.begin(CSPin)) while (1);

  for(int i=0;i<numberOfSlaves;++i)
  {
    pinMode(interruptToSlavePin[i],OUTPUT);
    digitalWrite(interruptToSlavePin[i],LOW);
    if(!SD.exists(String(i)+".bin"))
    {
      File file = SD.open(String(i)+".bin",FILE_WRITE);
      if(file) file.close();
    } 
  }
  
  pinMode(ledPIN,OUTPUT);
  digitalWrite(ledPIN,LOW);

  Wire.begin();

  Serial.begin(9600);

  critcalNumberOfSignals=10;
  delay(100);
  performMeasurmentsFromSlaves();

}


void loop() {

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


//called automatically after loop if there is incoming data on Serial
void serialEvent()
{
  uint8_t counter=0;
  while(Serial.available() && counter<20)
  {
    messageBuffer[counter]=Serial.read();
    ++counter;
  }
  
  handleRecivedMessage();
}


void performMeasurmentsFromSlaves()
{
  measBuffer[126] = time & 0xFF;
  measBuffer[127] = (time >> 8) & 0xFF;
  measBuffer[128] = (time >> 16) & 0xFF;
  measBuffer[129] = (time >> 24) & 0xFF;

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

   delay(500);
   digitalWrite(slavePin,LOW);
}


void saveMeasurmentToFile(uint8_t slaveAddress)
{
   File file = SD.open(String(slaveAddress)+".bin",FILE_WRITE);
   if(!file) return;
   file.write(measBuffer,130);
   file.close();
}


void handleRecivedMessage()
{
  if(messageBuffer[5]==0) sendListOfSlaves();
  else if(messageBuffer[5]==255) endAlarm();
  else if(messageBuffer[5]==8) sendLatestMeasurement(messageBuffer[6]);
  else if(messageBuffer[5]==16) sendHistoricData(messageBuffer[6]);
  else if(messageBuffer[5]==32) clearHistoricData(messageBuffer[6]);
  else if(messageBuffer[5]==12) setNewCritcalNumberOfSignals();
}


void sendListOfSlaves()
{
  memset(messageBuffer,0,20);

  messageBuffer[0]=7+numberOfSlaves;
  messageBuffer[1] = time & 0xFF;
  messageBuffer[2] = (time >> 8) & 0xFF;
  messageBuffer[3] = (time >> 16) & 0xFF;
  messageBuffer[4] = (time >> 24) & 0xFF;
  messageBuffer[5]=2;
  
  for(uint8_t i=0;i<7;++i)
  {
   Serial.write(messageBuffer[i]);
   Serial.flush();
  }

  for(uint8_t i=0;i<numberOfSlaves;i++)
  {
    Serial.write(slaveAddresses[i]);
    Serial.flush();
  }
}


void endAlarm()
{
  digitalWrite(ledPIN,LOW);
  sendAck();
}


void sendLatestMeasurement(uint8_t slaveAddress)
{
  memset(messageBuffer,0,20);

  messageBuffer[0]=133;
  messageBuffer[1] = timeOfLastMeasurement & 0xFF;
  messageBuffer[2] = (timeOfLastMeasurement >> 8) & 0xFF;
  messageBuffer[3] = (timeOfLastMeasurement >> 16) & 0xFF;
  messageBuffer[4] = (timeOfLastMeasurement >> 24) & 0xFF;
  messageBuffer[5]=4;
  messageBuffer[6]=slaveAddress;

  File file = SD.open(String(slaveAddress)+".bin", FILE_READ);
  if(!file) return;

  int fileSize=file.size();
  int position=fileSize-130;

  if(position<0) return;

  for(int i=0;i<7;++i) {
    Serial.write(messageBuffer[i]);
  }
 
  file.seek(position);

  uint8_t buffer[32];
  uint8_t idx=0;
  while(file.available() &&idx<126)
  {
    int numberOfBytesToRead=min(32,126-idx);
    file.read(buffer,numberOfBytesToRead);
    Serial.write(buffer,numberOfBytesToRead);
    Serial.flush();
    idx=idx+numberOfBytesToRead;
  }

  file.close();
}


void clearHistoricData(uint8_t slaveAddress)
{
  SD.remove(String(slaveAddress)+".bin");
  File file = SD.open(String(slaveAddress)+".bin",FILE_WRITE);
  if(file) file.close();
  sendAck();
  performMeasurmentsFromSlaves();
}


void sendHistoricData(uint8_t slaveAddress)
{

  uint16_t line = 0; //how many lines go back to
  line |= messageBuffer[7];
  line |= ((uint16_t)messageBuffer[8]) << 8; 
  
  memset(messageBuffer,0,20);

  messageBuffer[0]=130+7;
  messageBuffer[1] = timeOfLastMeasurement & 0xFF;
  messageBuffer[2] = (timeOfLastMeasurement >> 8) & 0xFF;
  messageBuffer[3] = (timeOfLastMeasurement >> 16) & 0xFF;
  messageBuffer[4] = (timeOfLastMeasurement >> 24) & 0xFF;
  messageBuffer[5]=64;
  messageBuffer[6]=slaveAddress;

  File file = SD.open(String(slaveAddress)+".bin", FILE_READ);
  if(!file) return;

  int fileSize=file.size();

  int position=fileSize-130*(line+1);

  if(position<0)
  {
    messageBuffer[6]=65;
    position=0;
  }
  
  for(int i=0;i<7;++i) {
    Serial.write(messageBuffer[i]);
  }
 
  file.seek(position);

  uint8_t buffer[32];
  uint8_t idx=0;
  
  
  while(file.available() &&idx<130)
  {
    int numberOfBytesToRead=min(32,130-idx);
    file.read(buffer,numberOfBytesToRead);
    Serial.write(buffer,numberOfBytesToRead);
    Serial.flush();
    idx=idx+numberOfBytesToRead;
  }
  
  
  file.close();
}


void setNewCritcalNumberOfSignals()
{
  critcalNumberOfSignals=messageBuffer[7];
  sendAck();
}


void sendAck()
{
  memset(messageBuffer,0,20);

  messageBuffer[0]=7;
  messageBuffer[1] = time & 0xFF;
  messageBuffer[2] = (time >> 8) & 0xFF;
  messageBuffer[3] = (time >> 16) & 0xFF;
  messageBuffer[4] = (time >> 24) & 0xFF;
  messageBuffer[5]=128;
  
  for(uint8_t i=0;i<7;++i)
  {
   Serial.write(messageBuffer[i]);
   Serial.flush();
  }
}