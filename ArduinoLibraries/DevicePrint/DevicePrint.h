// Copyright (c) 2009, Charlie Robson
// All rights reserved.

// See license.txt for further information.

#ifndef __DEVICEPRINT_H__
#define __DEVICEPRINT_H__

#include "WConstants.h"

// V 1.0 February 2009
// arduinonut.blogspot.com

// Modified from original source by bobemoe @ arduino.cc forums,
// with enormous thanks.

// Sector write function definition. If your hardware support library doesn't
// provide a sector write function that uses this signature then you need to
// provide a proxy function that does.
//
typedef uint8_t (*WRITEFN)(const uint8_t* buffer,
   unsigned long sector,
      uint8_t count);


class DevicePrint: public Print
{
public:
  // Parameters:
  //  sector - indicates the 1st sector to start writing to,
  //  availableSectors - count of sectors available.
  //  buffer - pointer to at least 512 bytes of allocated storage.
  //  writer - pointer-to-function which is called when data needs to be written.
  //
  void initialize(unsigned long sector,
    unsigned long availablerSectors,
      uint8_t* buffer,
        WRITEFN writer);

  // Write a byte to the sector buffer
  //
  // Calls flush() when the buffer is filled, so you don't have to!
  // Services the base class print() functions.
  // Will fail silently if m_sectorsAvailable == 0.
  //
  virtual void write(uint8_t theByte);

  // Put a string stored in PROGMEM to the sector buffer.
  //
  // Use like so:
  //   xxx.print_P(PSTR("lovely baps"));
  //
  void print_P(const char *data);

  // Writes the sector buffer to the device at the current sector.
  //
  // Called by write() when the buffer is full.
  // Can be called every so often or when you've written a big ol' chunk of
  // stuff in order to get the warm fuzzy feeling that your data is safe.
  // It only writes when there's data in the buffer, and it'll only advance
  // the sector index and available sector count when flushing a full buffer.
  //
  bool flush(void);

  // I've left these visible as there's little point in hiding things here.
  //
  unsigned long m_sector;
  unsigned long m_sectorsAvailable;

  int m_bufferIndex;

  uint8_t* m_buffer;

  WRITEFN m_writer;
};

#endif //  __DEVICEPRINT_H__
