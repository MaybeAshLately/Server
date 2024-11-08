List of Contents
1. Introduction
2. Server Overview
3. Data format
    - I2C data from node
    - SD card
    - Bluetooth transmission 
4. Components and Wiring
5. Libraries
6. License
 
# Introduction

Server repository is part of the RF scanner project. For a full project overview, please refer to the [RF_Scanner](https://github.com/MaybeAshLately/RF_Scanner) repository on GitHub account MaybeAshLately. 

# Server overview
The Server’s task is to collect measurements of RF spectrum occupancy from nodes (slaves) through I2C, check if critical number of signals has been reached (and alerts it using LED diode), save measurement on SD card and, on demand from phone, send the results to it through bluetooth.

Measurement has format of an 126 bytes array, each containing the number of signals detected on the corresponding channels. The scanner operates in the frequency of 2,4 - 2,525 GHz. Each channel is 1 MHz wide.

**Warning** in this example, the only slave address is hardcoded as "8". To use multiple nodes (slaves), you must code their addresses, total number and pins where they are connected (each slave apart of SCL and SDA pins to I2C itself is connected to one of I/O pins to trigger external interrupt). On I2C bus there can no be two devices with the same address. You can modify it by changing following lines:

```bash
const uint8_t interruptToSlavePin[]={6};
const uint8_t slaveAddresses[]={8};
const uint8_t numberOfSlaves=1;
```
Addresses can range from 8 to 126. To use more nodes than available I/O pins you can use eg. expander (remembering to modify the code accordingly).

You canchange interval of measurements (approximate time between measurements in seconds) by changing line:
```bash
const int interval=600;
```

#  Data format
### I2C data from node
All measurements have format of 126 bytes transmission. Due to I2C limitations they are send as packets of 32, 32, 32 and 30 bytes.
### SD card
All measurements are saved to files in format nodeNumber.bin (where nodeNumber is number 1-127) as binary data. Each record has size of 130 bytes - 126 bytes of measurement and last 4 bytes for time of measurement (counted as seconds from powering up the server). Mind that if you switch off server and then turn it on again you will not be able to correctly decode all times of measures. 
### Bluetooth transmission
Data is send in binary format in following structure:
- One byte - size of whole message,
- Four byte - time (from phone side current time, from server side time since turning it on),
- One byte - type of message:
  - 0 - initalization of communication, from phone,
  - 2 - list of slaves, from server,
  - 4 - last measure, from slave,
  - 8 - demand of last measure, from phone,
  - 12 - change of critical level, from phone, 
  - 16 - demand of historic measure, from phone (in data section info how many lines behind)
  - 32 - clean file, from phone,
  - 64 - historic measure, from server,
  - 65 - historic measure and end of file info, from server,
- One byte - address, for messages that concern certain slave, eg. demanding of data,
- Data of various size (max is 137 bytes).

# Components and wiring 
The server part of RF scanner uses the following components:
- Arduino Uno R3
- SD reader
- HC-06 - for bluetooth transmission
- LED diode

To connect node to server's Arduino: 
- SCl connects to pin SCL, 
- SDA connects to pin SDA, 
- Interrupt pin of node connects to pin 6. 

To connect Arduino to SD reader:
- CS connects to pin 10, 
- SCK connects to pin 13, 
- MOSI connects to pin 11, 
- MISO connects to pin 12, 
- VCC connects to 5V,
- GND connects to GND.

To connect Arduino to LED diode:
- Anode connects to pin 8,
- Cathode connects to GND through 330Ω resistor.

To connect Arduino to HC-06:
- VCC connects to 5V,
- GND connects to GND,
- HC-06 Tx connects to 0->Rx Arduino,
- HC-06 Rx connects to 1<-Tx Arduino thorugh voltage converter (it is necessary because of HC-06's 3,3V logic to not damage it altough in second pin (0->Rx Arduino) there is no need to use converter). 

**Attention** during uploding programe sketch to Arduino you must unplug the Tx and Rx wires - if anything is connected to hardware Serial it will cause uploading error.


# Libraries
The project uses following libraries:
- Wire.h - for I2C bus
- [Narcoleptic.h](https://github.com/brabl2/narcoleptic) - to put the microcontroller to sleep and save power, 
- [SD.h](https://github.com/arduino-libraries/SD) - to manage SD card reader, version 1.3.0,
- SPI.h - to manage SPI interface, used to communicate with SD reader. 

# License 
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see https://www.gnu.org/licenses/.
