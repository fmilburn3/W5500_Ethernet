/*
  Receives data from dAISy and forwards it to alfa.marifact.net using a TI F5529 LP with a WIZnet
  ioShield-L and Energia using UDP.
  
  If an Ethernet connection cannot be configured with DHCP the red LED will flash regularly and rapidly.
  If a connection is made the red LED will flash whenever a valid message is received from dAISy and
  a UDP transmission is attempted.

  Tested with:
  dAISy            ioShield-L LaunchPad   Energia         Comments
  --------------   --------------------   -------------   -------------------------------------------
  F5529/BackPack   MSP-430F5529           v17 
  dAISy            MSP-430F5529           v17
  
  Notes:
  1) You must enable the auxiliary serial output from the dAISy debug menu for this sketch
     to receive data.  Enter "2" to connect at 38400 baud.  Make sure that debug is off.  
     Then save the settings.
  2) For MSP430 processors it is necessary to increase the serial ring buffer size in Energia 
     (HardwareSerial.cpp) to get stable operation, e.g. #define SERIAL_BUFFER_SIZE 256.
  3) There is not enough memory on the G2553 for this sketch to work.
  
  Connections: 
  dAISy                   MSP430F5529       
  --------------          --------------   
  P4.4 (TX)               Pin 3 RX(1)
  GND                     GND                   
        
  Created by Frank Milburn, March 2016
*/

#include <SPI.h>
#include <Ethernet.h>

#define MAXLEN 100                             // maximum allowed length of a NMEA sentence
#define DEBUG 1                                // make DEBUG 0 to turn off extra print

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // ethernet MAC
byte server[] = { xxx, xxx, xxx, xxx };               // alfa.marifact IP   
int  serverPort = xxxx;                               // alfa.marifact port

char nmea[MAXLEN];                             // holds incoming NMEA sentences   
int blinkSentence = 0;                         // toggles RED_LED when sentences are sent

EthernetUDP Udp;

void setup()
{  
  
  pinMode(RED_LED, OUTPUT);
  Serial1.begin(38400);                        // dAISy transmissions received at 38400 baud
                                               // on Serial1  
  if (DEBUG) {
    Serial.begin(115200);                      // debug serial output to monitor         
    delay(500);                                // let Serial catch up...
    Serial.println("Starting W5500...");
  }
  
  if (Ethernet.begin(mac) == 0)                 // start the Ethernet connection:
  {
    if (DEBUG) Serial.println("Failed to configure Ethernet using DHCP");
    while(1)
    {
      digitalWrite(RED_LED,HIGH);               // Could not configure Ethernet
      delay(100);                               // so loop and blink forevermore
      digitalWrite(RED_LED,LOW);
      delay(100);
    }    
  } 
  Udp.begin(8888);
  delay(1000);                                  // let Ethernet stabilize  
}

void loop() 
{  
     
  static int i = 0;                            // tracks characters read into the nmea array

  while (Serial1.available()) {                // read incoming NMEA sentences from dAISy       
    if (i < MAXLEN-1) {
      nmea[i] = (char)Serial1.read();          // place new chars into nmea[]
    }
    nmea[++i] = 0;                             // increment the counter and store a end of string                                                     
    if (nmea[i-1] == '\n') {                   // newline indicates the end of the NMEA sentence
      if (DEBUG) Serial.print(nmea);           // debug print of sentence to serial monitor
      if (blinkSentence == 0) {
        blinkSentence = 1;
      }
      else {
        blinkSentence = 0;
      }
      digitalWrite(RED_LED, blinkSentence);    // toggle the red LED for successful NMEA sentence
      Udp.beginPacket(server, serverPort);     // UDP packet transfer
      Udp.write(nmea);
      Udp.endPacket();
      i = 0;                                   // now set the counter back to 0
      nmea[i] = '\0';                          // and initialize the first element of nema to null          
    }                                           
  }
}
