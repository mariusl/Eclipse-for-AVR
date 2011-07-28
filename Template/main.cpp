/*
 * main.cpp
 *
 *  Created on: 23 Jun 2011
 *      Author: Marius
 */
#include <Wprogram.h>
// Uncomment if you need it for C++ style new/delete and pointer protection
#include "cppsupport.h"
#include "eeprom/eeprom.h"
#include "keypad/keypad.h"
#include "Liquidcrystal/liquidcrystal.h"
#include "newsoftserial/newsoftserial.h"
#include "sd/sd.h"

int ledPin = 13;
// start reading from the first byte (address 0) of the EEPROM
int address = 0;
byte value;

NewSoftSerial dbP = NewSoftSerial(2, 6);
void eepromWrite()
{
	for (int i = 0; i < 512; ++i) {
		EEPROM.write(i,0xff);
	}
}

void eepromRead()
{
	for (int i = 0; i < 512; ++i) {
		// read a byte from the current address of the EEPROM
		  value = EEPROM.read(address);

		  Serial.print(address);
		  Serial.print("\t");
		  Serial.print(value, DEC);
		  Serial.println();

		  // advance to the next address of the EEPROM
		  address = address + 1;

		  // there are only 512 bytes of EEPROM, from 0 to 511, so if we're
		  // on address 512, wrap around to address 0
		  if (address == 512)
		    address = 0;

		  delay(10);
	}

}

void setup()
{
	pinMode(ledPin, OUTPUT);      // sets the digital pin as output
	Serial.begin(9600);
	//eepromWrite();
	eepromRead();
}

void loop(void)
{
	do{
		digitalWrite(ledPin, HIGH);   // sets the LED on
		delay(100);                  // waits for a second
		digitalWrite(ledPin, LOW);    // sets the LED off
		delay(100);                  // waits for a second
	}while(1);
}

int main(void)
{
   init();

   setup();


      loop();

   return(0);
}
