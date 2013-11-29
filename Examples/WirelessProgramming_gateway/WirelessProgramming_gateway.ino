/*
 * Copyright (c) 2013 by Felix Rusu <felix@lowpowerlab.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

// This sketch is an example of how wireless programming can be achieved with a Moteino
// that was loaded with a custom 1k Optiboot that is capable of loading a new sketch from
// an external SPI flash chip
// This is the GATEWAY node, it does not need a custom Optiboot nor any external FLASH memory chip
// (ONLY the target node will need those)
// The sketch includes logic to receive the new sketch from the serial port (from a host computer) and 
// transmit it wirelessly to the target node
// The handshake protocol that receives the sketch from the serial port 
// is handled by the SPIFLash/WirelessHEX69 library, which also relies on the RFM12B library
// These libraries and custom 1k Optiboot bootloader for the target node are at: http://github.com/lowpowerlab

#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>
#include <WirelessHEX69.h>

#define MYID        1   // node ID used for this unit
#define NETWORKID   250
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define FREQUENCY   RF69_433MHZ
//#define FREQUENCY   RF69_868MHZ
#define FREQUENCY     RF69_915MHZ
//#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define LED         9
#define SERIAL_BAUD 115200
#define ACK_TIME    50  // # of ms to wait for an ack
#define TIMEOUT     3000
#define ENCRYPTKEY  "sampleEncryptKey"

byte targetID = 10;   // default
RFM69 radio;
char c = 0;
char input[64]; //serial input buffer

void setup(){
  Serial.begin(SERIAL_BAUD);
  radio.initialize(FREQUENCY,MYID,NETWORKID);
  //radio.encrypt(ENCRYPTKEY); //OPTIONAL
#ifdef IS_RFM69HW
  radio.setHighPower(); //only for RFM69HW!
#endif
  Serial.print("Start wireless gateway...");
}

void loop(){
  byte inputLen = readSerialLine(input);
  
  if (inputLen == 4 && input[0]=='F' && input[1]=='L' && input[2]=='X' && input[3]=='?') {
    CheckForSerialHEX((byte*)input, inputLen, radio, targetID, TIMEOUT, ACK_TIME, true);
  }
  else if (input[0]=='M' && input[1]=='I' && input[2]=='D' && input[3]==':') {
    targetID = 0;
    for (byte i = 4; i<inputLen; i++) //up to 3 characters for target ID
    {
      if (input[i] >=48 && input[i]<=57)
        targetID = targetID*10+input[i]-48;
    }
  }
  else if (inputLen>0) { //just echo back
    Serial.print("SERIAL IN > ");Serial.println(input);
  }
  
  if (radio.receiveDone())
  {
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
    
    if (radio.ACK_REQUESTED)
    {
      radio.sendACK();
      Serial.print(" - ACK sent");
    }
    
    Serial.println();
  }
  Blink(LED,5); //heartbeat
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
