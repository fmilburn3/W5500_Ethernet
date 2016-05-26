/*
  SPECIAL VERSION for G2955 custom development board
  
  Receives data from dAISy and forwards it to MarineTraffic using a TI MSP430G2955 custom board with a WIZnet
  ioShield-L and Energia.
  
  If a connection cannot be made with MarineTraffic the red LED will flash regularly and rapidly.  If
  a connection is made the red LED will flash whenever a valid message is received from dAISy and
  transmitted to MarineTraffic.

  Tested with:
  dAISy            ioShield-L LaunchPad   Energia         Comments
  --------------   --------------------   -------------   -------------------------------------------
  F5529/BackPack   MSP-430F5529           v17 
  dAISy            MSP-430F5529           v17
  dAISy            MSP430G2955 custom     v17
  
  
  Notes:
  1) You must enable the auxiliary serial output from the dAISy debug menu for this sketch
     to receive data.  Enter "2" to connect at 38400 baud.  Make sure that debug is off.  
     Then save the settings.
  2) For MSP430 processors it is necessary to increase the serial ring buffer size in Energia 
     (HardwareSerial.cpp) to get stable operation, e.g. #define SERIAL_BUFFER_SIZE 256.
  3) There is not enough memory on the G2553 for this sketch to work.
  
  Connections: 
  Programming         MSP430G2955
  --------------      -----------------
  3V3                 3V3
  GND                 GND
  TST                 TST        /  1
  RST                 RST        /  7
  TXD (RX side)       P3_4 (TX)  /  25
  
  dAISy               MSP430G2955
  --------------      ------------------
  P4.4 (TX)           P3_5       / 26
  GND                 GND
        
  ioShield     Desecription         MSP430G2955
  ---------    -----------------    -------------
  J1.1         Power (3V3)          3V3 
  J2.20        Ground               GND 
  J1.7         SPI Chip Select      P4_2  /  19
  J1.8         SPI Clock            P1_2  /  33
  J2.14        SPI MISO             P3_2  /  13
  J2.15        SPI MOSI             P3_1  /  12
  J2.16        nRESET               RST   /  7
  
  Note:  ioShield pin nomenclature is easily converted to regular Energia pin numbering
         (e.g.  ioShield J1.7 is Energia 7, ioShield J2.14 is Energia 14)
  
  Created by Frank Milburn, March 2016
*/

#include <SPI.h>
#include <Ethernet.h>

#define MAXLEN 100                             // maximum allowed length of a NMEA sentence
#define DEBUG 1                                // make DEBUG 0 to turn off extra print
#define LED RED_LED                            // flash when messages are sent

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // Your ethernet MAC
byte server[] = { 5, 9, 207, 224 };            // Marine Traffic IP   
int  serverPort = xxxx;                        // User's port at Marine Traffic

char nmea[MAXLEN];                             // holds incoming NMEA sentences   
int blinkSentence = 0;                         // toggles RED_LED when sentences are sent

EthernetClient client;

void setup()
{   
  pinMode(LED, OUTPUT);
  Serial.begin(38400);                        // dAISy transmissions received at 38400 baud
                                              // on Serial 
  Serial.println("Starting");
  
  if (Ethernet.begin(mac) == 0)               // start the Ethernet connection:
  {
    Serial.println("Failed to configure Ethernet");
    while(1)
    {
      digitalWrite(LED,HIGH);                 // Could not make a connection
      delay(100);                             // so loop and blink forevermore
      digitalWrite(LED,LOW);
      delay(100);
    }    
  } 
  delay(1000);                                // let Ethernet stabilize 
  Serial.println("\nConnecting to MarineTraffic...");  
  if (client.connect(server, serverPort)) {
    Serial.println("Connected");
  }   
}

void loop() 
{  
  while (!client.connected()) {               // disconnected from MarineTraffic
    client.flush();                            
    client.stop();
    delay(5000);
    Serial.println("Not connected to MarineTraffic - attempt to connect");
    client.connect(server, serverPort);
    delay(5000);
  }
   
  static int i = 0;                            // tracks characters read into the nmea array

  while (Serial.available()) {                // read incoming NMEA sentences from dAISy       
    if (i < MAXLEN-1) {
      nmea[i] = (char)Serial.read();          // place new chars into nmea[]
    }
    nmea[++i] = 0;                             // increment the counter and store a end of string                                                     
    if (nmea[i-1] == '\n') {                   // newline indicates the end of the NMEA sentence
      if (blinkSentence == 0) {
        blinkSentence = 1;
      }
      else {
        blinkSentence = 0;
      }
      digitalWrite(LED, blinkSentence);        // toggle the red LED for successful NMEA sentence
      Serial.print(nmea);
      client.print(nmea);                      // send the sentence to MarineTraffic
      i = 0;                                   // now set the counter back to 0
      nmea[i] = '\0';                          // and initialize the first element of nema to null          
    }                                           
  }
}
