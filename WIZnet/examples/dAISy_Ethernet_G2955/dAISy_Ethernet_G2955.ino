/*
  MarineTraffic connection with dAISy Ethernet Adapter
  Version 3 Rev B
  
  Receives data from dAISy and forwards it to MarineTraffic.  Power comes the 5V power header on a
  dAISy and is dropped to 3V3 with a LDO.  A TI MSP430G2955 and WizNet W5500 then make the Ethernet
  connection.  Serial data is received from the dAISy on P3.5 (RX) and DEBUG print is on P3.4 (TX).
  A LaunchPad must be connected and a PC present to see the debug print.  If a connection cannot
  be made with MarineTraffic the status LED will flash regularly and rapidly.  If a connection is
  made the status LED will flash whenever a valid message is received from dAISy and transmitted
  to MarineTraffic. 

  Tested with:
  dAISy Hardware         dAISy Firmware           dAISy Ethernet Adapter   Energia         
  --------------------   ----------------------   ----------------------   --------------------------  
  USB 3 Rev C            Daisy_hw3_fw4_07         Rev B                    v17 custom for G2955 Rev A           
  
  Notes:
  1) Enable the auxiliary serial output from the dAISy debug menu for this sketch to receive data.
     Enter "2" to connect at 38400 baud.  Make sure that debug is off. Then save the settings with #.
  2) For MSP430 processors it is necessary to increase the serial ring buffer size in Energia 
     (HardwareSerial.cpp) to get stable operation, e.g. #define SERIAL_BUFFER_SIZE 256.
  
  Programming Header Connections: 
  dAISy Ethernet      MSP430F5529
  Adapter             LaunchPad           Comments
  --------------      ---------------     ------------------------------------------------
  GND                 GND
  TST                 TST        
  RST                 RST        
  TX                  P3_4 (TX)           Necessary only if debug print is desired
  
  Notes: 
  1) DO NOT POWER dAISy Ethernet from the dAISy module and the programming header 
     simultaneously.  I.E. do not connect 3V3 on the dAISy Ethernet Adapter when the dAISy
     module is active. Recommend that power come from the dAISy through the LDO only.
  2) Make the MSP430F5529 connections on the FET side of the LaunchPad.

  Created by Frank Milburn, May 2016
*/

#include <SPI.h>
#include <Ethernet.h>

#define MAXLEN 100                               // maximum allowed length of a NMEA sentence
#define DEBUG 1                                  // make DEBUG 0 to turn off extra print
#define STATUS_LED P2_0                          // flashes when messages are sent or rapidly if no
                                                 // Ethernet connection
#define nRST P1_2                                // Reset for W5500

byte mac[] = {0x00,0xA3,0xBE,0xEF,0xFE,0xEE};    // Your ethernet MAC
byte server[] = { 5, 9, 207, 224 };              // Marine Traffic IP   
int  serverPort = 6050;                          // User's port at Marine Traffic

char nmea[MAXLEN];                               // holds incoming NMEA sentences   
int blinkSentence = 0;                           // toggles STATUS_LED when sentences are sent

EthernetClient client;

void setup()
{   
  pinMode(STATUS_LED, OUTPUT);
  Serial.begin(38400);                           // dAISy transmissions received at 38400 baud
  if (DEBUG) Serial.println("Starting..."); 
  
  /*
  NOTE:  The following code is not currently working.  Need to figure out how to use
  nRST or PWDN to clear the W5500.
  */
  
  digitalWrite(nRST, LOW);                       // Pull W5500 reset low for 1 ms to reset
  delay(1);
  digitalWrite(nRST, HIGH);                      // Release W5500 reset and allow 200 ms to stabilize
  delay(200);
  
  
  if (Ethernet.begin(mac) == 0){                 // start the Ethernet connection:
    if (DEBUG) Serial.println("Failed to configure Ethernet");
    while(1){
      digitalWrite(STATUS_LED,HIGH);             // Could not make a connection
      delay(100);                                // so loop and blink forevermore
      digitalWrite(STATUS_LED,LOW);
      delay(100);
    }    
  } 
  else {                                         // Ethernet started
    delay(100);                                  // let Ethernet stabilize
    if (DEBUG){
      Serial.print("My IP address: "); 
      for (byte thisByte = 0; thisByte < 4; thisByte++){
        Serial.print(Ethernet.localIP()[thisByte], DEC);
        Serial.print(".");
      }
    }
  } 
  if (DEBUG) Serial.println("\nConnecting to MarineTraffic...");  
  if (client.connect(server, serverPort)) {
    if (DEBUG) Serial.println("Connected");
  }   
}

void loop() 
{  
  while (!client.connected()) {                 // disconnected from MarineTraffic
    client.flush();                            
    client.stop();
    delay(5000);
    if (DEBUG) Serial.println("Not connected to MarineTraffic - attempt to connect");
    client.connect(server, serverPort);
    delay(5000);
  }
   
  static int i = 0;                             // tracks characters read into the nmea array

  while (Serial.available()) {                  // read incoming NMEA sentences from dAISy       
    if (i < MAXLEN-1) {
      nmea[i] = (char)Serial.read();            // place new chars into nmea[]
    }
    nmea[++i] = 0;                              // increment the counter and store a end of string                                                     
    if (nmea[i-1] == '\n') {                    // newline indicates the end of the NMEA sentence
      blinkSentence = !blinkSentence;
      digitalWrite(STATUS_LED, blinkSentence);  // toggle the red LED for successful NMEA sentence
      if (DEBUG) Serial.print(nmea);
      client.print(nmea);                       // send the sentence to MarineTraffic
      i = 0;                                    // now set the counter back to 0
      nmea[i] = '\0';                           // and initialize the first element of nema to null          
    }                                           
  }
}

