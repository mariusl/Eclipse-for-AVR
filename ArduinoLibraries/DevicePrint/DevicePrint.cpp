// Copyright (c) 2009, Charlie Robson
// All rights reserved.

// See license.txt for further information.

#include "WProgram.h"
#include <avr/pgmspace.h>
#include "DevicePrint.h"

// Using a function pointer breaks any hardware library dependency and allows some external
// control over the writing process. This keeps the library small and tight.
//
void DevicePrint::initialize(unsigned long sector, unsigned long availableSectors, uint8_t* buffer, WRITEFN writer)
{
  m_bufferIndex = 0;

  m_sector = sector;
  m_sectorsAvailable = availableSectors;

  m_buffer = buffer;
  m_writer = writer;
}


// Write the specified byte to the buffer.
//
void DevicePrint::write(uint8_t theByte)
{
  m_buffer[m_bufferIndex] = theByte;

  ++m_bufferIndex;
  if (m_bufferIndex == 512)
  {
    // This will definitely write the sector as m_bufferIndex == 512.
    //
    flush();
  }
}

// Put the specified PROGMEM string into the buffer.
// Unspecified things are happening if the string is not in progmem..!
//
void DevicePrint::print_P(const char *data)
{
  while (pgm_read_byte(data) != 0x00)
  {
     write(pgm_read_byte(data++));
   }
}

// Flush sector buffer to device if appropriate.
//
bool DevicePrint::flush(void)
{
  if (m_bufferIndex != 0 && m_sectorsAvailable != 0)
  {
    if (m_writer(m_buffer, m_sector, 1) != 0)
    {
      return false;
    }

    // Only advance internal counters when flushing a full buffer.
    // This allows periodic flushes to maintain the data integrity.
    //
    if (m_bufferIndex == 512)
    {
      m_bufferIndex = 0;

      ++m_sector;
      --m_sectorsAvailable;
    }
  }

  return true;
}
