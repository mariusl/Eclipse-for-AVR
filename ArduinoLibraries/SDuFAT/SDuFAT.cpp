/*
 * SDuFAT.cpp - Use of SD Cards
 *
 * Copyright (C) 2008 Libelium Comunicaciones Distribuidas S.L.
 * http://www.libelium.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * However, D.J. Cuartielles has to credit the following people, since 
 * this library is just a wrapper on code written by others, who deserve
 * all the credit for their effort making this possible:
 *
 *  ** sd2iec - SD/MMC to Commodore serial bus interface/controller
 *     Copyright (C) 2007,2008  Ingo Korb <ingo@akana.de>
 *
 *  ** Inspiration and low-level SD/MMC access based on code from MMC2IEC
 *     by Lars Pontoppidan, Aske Olsson, Pascal Dufour, DTU, Denmark
 *
 * Created February 14th, 2009 by David Cuartielles -aka BlushingBoy-
 */

#include "SDuFAT.h"

/// VARIABLES ///////////////////////////////////////////////////////////////////////////

// declare the verbose variable
byte VERBOSE = ON;

// buffers and other needed variables
byte buffer[512]; // the buffer cannot be any smaller, SD cards are read/written in blocks of 512 bytes
char data[DATABUFFERSIZE];  // aux array to read/write strings from/to the SD card
int inByte = -1;
static unsigned long sector, sectors, length;

/// CONSTRUCTORS ////////////////////////////////////////////////////////////////////////

SDuFAT::SDuFAT()
{
    // nothing to do
}

/// FUNCTIONS ///////////////////////////////////////////////////////////////////////////

//////TESTED FUNCTIONS //////////////////////////////////////////////////////////////////

// printEvent - displays what is happening
void SDuFAT::printEvent(int event, const char* filename)
{
  if (VERBOSE) {
    Serial.print("[");
    Serial.print(filename);
    Serial.print("] - ");
    switch (event) {
    case LS:
      Serial.println(LSm);
      break;
    case DEL:
      Serial.println(DELm);
      break;
    case CAT:
      Serial.println(CATm);
      break;
    case PRINT:
      Serial.println(PRINTm);
      break;
    case WRITE:
      Serial.println(WRITEm);
      break;
    case APPEND:
      Serial.println(APPENDm);
      break;
    case SUCCESS:
      Serial.println(SUCCESSm);
      break;
    case ERROR:
      Serial.println(ERRORm);
      break;
    case WARNING:
      Serial.println(WARNINGm);
      break;
    default:
      break;  
    }
  }
}

// usedBytes - returns how many bytes are used in the file
long SDuFAT::usedBytes(const char* filename)
{
  long bytesUsed = 0;
  mmc::initialize();

  if (microfat::initialize(buffer))
  {

    if (microfat::locateFileStart(filename, sector, length))
    {
      if (length > BYTESPERSECTOR) {
        sectors = length / BYTESPERSECTOR; 
        length = BYTESPERSECTOR;
      }

      for( long j = 0; j <= sectors; j++) 
        if (RES_OK == mmc::readSectors(buffer, sector + j, 1))
        {
          if (length > BYTESPERSECTOR)
            length = BYTESPERSECTOR;

          for(int i = 0; i < length; ++i)
          {
            if (buffer[i] == EOF) {
              return bytesUsed;
            }
            bytesUsed++;
          }

        } 
        else return 0;
    } 
    else return 0;
  } 
  else return 0;
  return bytesUsed;
}

// startSector - returns the sector wher the file starts
long SDuFAT::startSector(const char* filename)
{
  unsigned long theSector = 0;
  mmc::initialize();

  if (microfat::initialize(buffer))
  {
    //DIRTY FIXME: length is lect as global variable, that is baaaad
    if (microfat::locateFileStart(filename, theSector, length))
    {

    } 
    else return 0;
  } 
  else return 0;
  return theSector;
}

// del - erases the file given as parameter for real it just writes the first character of
//       the file's blocks with EOF, the rest will be kept in the card. It could be used
//       to recover data the old-school way
int SDuFAT::del(const char* filename)
{
  printEvent(DEL, filename);
  mmc::initialize();

  if (microfat::initialize(buffer))
  {

    if (microfat::locateFileStart(filename, sector, length))
    {
      sectors = length / BYTESPERSECTOR;

      for(long m = 0; m <= sectors; m++) {
        if (RES_OK == mmc::readSectors(buffer, sector + m, 1))
        {
          buffer[0] = EOF;

          for (int i = 0; i < 10; ++i)
          {
            if (RES_OK == mmc::writeSectors(buffer, sector + m, 1))
            {
            } 
            else return 1;
          }
        } 
        else return 1;
      }
    } 
    else return 1;
  } 
  else return 1;
  return 0;
}

// ls - prints info about the file to the screen
int SDuFAT::ls(const char* filename)
{
  mmc::initialize();

  if (microfat::initialize(buffer))
  {

    printEvent(LS, filename);

    if (microfat::locateFileStart(filename, sector, length))
    {
      Serial.println("length\tsectors\tused");
      Serial.print(length);
      Serial.print("B\t");

      if (length > BYTESPERSECTOR) {
        length = BYTESPERSECTOR;
      }

      sectors = length / BYTESPERSECTOR; 
      Serial.print(sectors);
      Serial.print("\t");

      long bytesUsed = 0;
      for( long j = 0; j <= sectors; j++) 
        if (RES_OK == mmc::readSectors(buffer, sector + j, 1))
        {
          if (length > BYTESPERSECTOR)
            length = BYTESPERSECTOR;

          for(int i = 0; i < length; ++i)
          {
            if (buffer[i] == EOF) {
              Serial.print(bytesUsed);
              Serial.println("B");
              return 0;
            }
            bytesUsed++;
          }

        } 
        else return 1;
    } 
    else return 1;
  } 
  else return 1;
  return 0;
}

int SDuFAT::cat(const char* filename)
{
  printEvent(CAT, filename);
  mmc::initialize();

  if (microfat::initialize(buffer))
  {

    if (microfat::locateFileStart(filename, sector, length))
    {

      if (length > BYTESPERSECTOR) {
        sectors = length / BYTESPERSECTOR; 
        length = BYTESPERSECTOR;
      }

      for( long j = 0; j <= sectors; j++) 
        if (RES_OK == mmc::readSectors(buffer, sector + j, 1))
        {
          if (length > BYTESPERSECTOR)
            length = BYTESPERSECTOR;

          for(int i = 0; i < length; ++i)
          {
            if (buffer[i] == EOF) {
              Serial.println();
              return 0;
            }
            Serial.print(buffer[i],BYTE);
          }

        } 
        else return 1;
    } 
    else return 1;
  } 
  else return 1;
  Serial.println();
  return 0;
}

int SDuFAT::write(const char* filename)
{
  printEvent(WRITE, filename);
  mmc::initialize();

  if (microfat::initialize(buffer))
  {

    if (microfat::locateFileStart(filename, sector, length))
    {
      if (RES_OK == mmc::readSectors(buffer, sector, 1))
      {
        if (length > BYTESPERSECTOR)
          length = BYTESPERSECTOR;

        int count = 0;
        for(count = 0; count < length; ++count)
        {
          while (!Serial.available()) {
          };
          inByte = Serial.read();
          if (inByte != EOL) {
            buffer[count] = inByte;
          } 
          else break;
        }

        for(int i = count; i < length; ++i)
        {
          buffer[i] = EOF;
        }

        for (int i = 0; i < 10; ++i)
        {
          if (RES_OK == mmc::writeSectors(buffer, sector, 1))
          {
          }
          else return 1;
        }
      } 
      else return 1;
    } 
    else return 1;
  } 
  else return 1;
  return 0;
}

int SDuFAT::println(const char* filename, char* data)
{
  int aux = print(filename, data);
  aux |= print(filename, "\n");
  return aux;
}

int SDuFAT::print(const char* filename, byte* data)
{
  int aux = print(filename, (char*) data);
  return aux;
}

int SDuFAT::println(const char* filename, byte* data)
{
  int aux = println(filename, (char*) data);
  return aux;
}

int SDuFAT::print(const char* filename, char* data)
{
  printEvent(PRINT, filename);

  // check where to start writing in the file
  // these calls will init sector and length
  int offset1 = 0;
  long file1Length = usedBytes(filename);
  offset1 = (int) (file1Length % BYTESPERSECTOR);
  sectors = file1Length / BYTESPERSECTOR;

  // commands that are run by previous functions and that force errors here
  //  mmc::initialize()
  //  microfat::initialize(buffer)
  //  microfat::locateFileStart(filename, sector, length)

  if (RES_OK == mmc::readSectors(buffer, sector + sectors, 1))
  {
    if (length > BYTESPERSECTOR) 
      length = BYTESPERSECTOR;

    int count = 0;
    for(count = offset1; count < length; ++count)
    {
      if (data[count-offset1] != 0) {  // 0 is end of string
        buffer[count] = data[count-offset1];
      } 
      else break;
    }

    for(int i = count; i < length; ++i)
    {
      buffer[i] = EOF;
    }

    for (int i = 0; i < 10; ++i)
    {
      if (RES_OK == mmc::writeSectors(buffer, sector + sectors, 1))
      {
      } 
      else return 1;
    }

    if (data[count-offset1] != 0) 
    {
      // it's not the end of the string, we gotta
      // push data into the next block
      int count2 = 0;
      for(count2 = 0; count2 < length; ++count2)
      {
        if (data[count2 + count - offset1] != 0) {  // 0 is end of string
          buffer[count2] = data[count2 + count - offset1];
        } 
        else break;
      }

      if (count2 < length)
      {
        buffer[count2] = EOF;

        // this is to keep the file clean by adding blank spaces
        for(int i = count2 + 1; i < length; ++i)
        {
          buffer[i] = ' ';
        }

        for (int i = 0; i < 10; ++i)
        {
          if (RES_OK == mmc::writeSectors(buffer, sector + sectors + 1, 1))
          {
          } 
          else return 1;
        }
      }
      else
      {
        // with the size of variables we operate this case will never happen
      }

    }
  } 
  else return 1;
  return 0;
}


int SDuFAT::append(const char* filename)
{
  printEvent(APPEND, filename);
  for(int count = 0; count < DATABUFFERSIZE; ++count)
  {
    while (!Serial.available()) {
    };
    inByte = Serial.read();
    Serial.print(inByte, BYTE);
    if (inByte != EOL) {
      data[count] = inByte;
    } 
    else break;
  }
  Serial.println();
  return print("hola.txt", data);
}

void SDuFAT::verbose(byte mode)
{
  VERBOSE = mode;
}

//////END TESTED FUNCTIONS //////////////////////////////////////////////////////////////

//////EXPERIMENTAL FUNCTIONS ////////////////////////////////////////////////////////////

// UNDER CONSTRUCTION, WILL COME SOON ///////////////////////////////////////////////////

/*
 * - append(filename1, filename2): appends filename2 at the end of filename1
 * - indexOf(filename, string): looks for a string in a file answering the position
 * - indexOfLine(filename, int): gets the offset in the file to the line determined by the parameter
 */

//////END EXPERIMENTAL FUNCTIONS ////////////////////////////////////////////////////////

/// END FUNCTIONS ///////////////////////////////////////////////////////////////////////

// PREINSTANTIATE OBJECTS ///////////////////////////////////////////////////////////////

SDuFAT SD = SDuFAT();

