// see header for credits

#include "mmc.h"


// PORTB pin numbers
//
// arduino D10
#define SS    2

// arduino D11
#define MOSI  3

// arduino D12
#define MISO  4

// arduino D13
#define SCK    5

// Macros for setting slave select:
//
#define SPI_SS_HIGH()  PORTB |= _BV(SS)
#define SPI_SS_LOW()   PORTB &= ~_BV(SS)


static volatile diskstates disk_state;


static byte spiTransferByte(byte data)
{
  // send the given data
  SPDR = data;

  // wait for transfer to complete
  loop_until_bit_is_set(SPSR, 7);
  // *** reading of the SPSR and SPDR are crucial
  // *** to the clearing of the SPIF flag
  // *** in non-interrupt mode

  // return the received data
  return SPDR;
}


static uint32_t spiTransferLong(const uint32_t data)
{
  // It seems to be necessary to use the union in order to get efficient
  // assembler code.
  // Beware, endian unsafe union
  union {
    unsigned long l;
    unsigned char c[4];
  } 
  long2char;

  long2char.l = data;

  // send the given data
  SPDR = long2char.c[3];
  // wait for transfer to complete
  loop_until_bit_is_set(SPSR, SPIF);
  long2char.c[3] = SPDR;

  SPDR = long2char.c[2];
  // wait for transfer to complete
  loop_until_bit_is_set(SPSR, SPIF);
  long2char.c[2] = SPDR;

  SPDR = long2char.c[1];
  // wait for transfer to complete
  loop_until_bit_is_set(SPSR, SPIF);
  long2char.c[1] = SPDR;

  SPDR = long2char.c[0];
  // wait for transfer to complete
  loop_until_bit_is_set(SPSR, SPIF);
  long2char.c[0] = SPDR;

  return long2char.l;
}


static char sdResponse(byte expected)
{
  unsigned short count = 0x0FFF;

  while ((spiTransferByte(0xFF) != expected) && count )
    count--;

  // If count didn't run out, return success
  return (count != 0);
}


static char sdWaitWriteFinish(void)
{
  unsigned short count = 0xFFFF; // wait for quite some time

  while ((spiTransferByte(0xFF) == 0) && count )
    count--;

  // If count didn't run out, return success
  return (count != 0);
}


static void deselectCard(void) {
  // Send 8 clock cycles
  SPI_SS_HIGH();
  spiTransferByte(0xff);
}

static byte crc7update(byte crc, const byte data) {
  byte i;
  bool bit;
  byte c;

  c = data;
  for (i = 0x80; i > 0; i >>= 1) {
    bit = crc & 0x40;
    if (c & i) {
      bit = !bit;
    }
    crc <<= 1;
    if (bit) {
      crc ^= 0x09;
    }
  }
  crc &= 0x7f;
  return crc & 0x7f;
}


/**
 * sendCommand - send a command to the SD card
 * @command  : command to be sent
 * @parameter: parameter to be sent
 * @deselect : Flags if the card should be deselected afterwards
 *
 * This function calculates the correct CRC7 for the command and
 * parameter and transmits all of it to the SD card. If requested
 * the card will be deselected afterwards.
 */
int mmc::sendCommand(const byte  command,
const uint32_t parameter,
const byte  deselect) {
  union {
    unsigned long l;
    unsigned char c[4];
  } 
  long2char;

  byte  i,crc,errorcount;
  uint16_t counter;

  long2char.l = parameter;
  crc = crc7update(0  , 0x40+command);
  crc = crc7update(crc, long2char.c[3]);
  crc = crc7update(crc, long2char.c[2]);
  crc = crc7update(crc, long2char.c[1]);
  crc = crc7update(crc, long2char.c[0]);
  crc = (crc << 1) | 1;

  errorcount = 0;
  while (errorcount < CONFIG_SD_AUTO_RETRIES) {
    // Select card
    SPI_SS_LOW();

    // Transfer command
    spiTransferByte(0x40+command);
    spiTransferLong(parameter);
    spiTransferByte(crc);

    // Wait for a valid response
    counter = 0;
    do {
      i = spiTransferByte(0xff);
      counter++;
    } 
    while (i & 0x80 && counter < 0x1000);

    // Check for CRC error
    // can't reliably retry unless deselect is allowed
    if (deselect && (i & STATUS_CRC_ERROR)) {
      //      uart_putc('x');
      deselectCard();
      errorcount++;
      continue;
    }

    if (deselect) deselectCard();
    break;
  }

  return i;
}



byte mmc::initialize() {
  byte  i;
  uint16_t counter;
  uint32_t answer;

  disk_state = DISK_ERROR;

  // setup SPI I/O pins
  PORTB |=  _BV(SCK) | _BV(SS) | _BV(MISO); // set SCK+SS hi (no chip select), pullup on MISO
  DDRB  |=  _BV(SCK) | _BV(SS) | _BV(MOSI); // set SCK/MOSI/SS as output
  DDRB  &= ~_BV(MISO);                       // set MISO as input

  // setup SPI interface:
  //   interrupts disabled, SPI enabled, MSB first, master mode,
  //   leading edge rising, sample on leading edge, clock = f/4,
  SPCR = B01010011;

  // Enable SPI double speed mode -> clock = f/8
  //  SPSR = _BV(SPI2X);

  // clear status
  i = SPSR;


  // clear recieve buffer
  i = SPDR;


  SPI_SS_HIGH();

  // Send 80 clks
  for (i=0; i<10; i++) {
    spiTransferByte(0xFF);
  }

  // Reset card
  i = sendCommand(GO_IDLE_STATE, 0, 1);
  if (i != 1) {
    return STA_NOINIT | STA_NODISK;
  }

  counter = 0xffff;
  // According to the spec READ_OCR should work at this point
  // without retries. One of my Sandisk-cards thinks otherwise.
  do {
    // Send CMD58: READ_OCR
    i = sendCommand(READ_OCR, 0, 0);
    if (i > 1) {
      // kills my Sandisk 1G which requires the retries in the first place
      // deselectCard();
    }
  } 
  while (i > 1 && counter-- > 0);

  if (counter > 0) {
    answer = spiTransferLong(0);

    // See if the card likes our supply voltage
    if (!(answer & SD_SUPPLY_VOLTAGE)) {
      // The code isn't set up to completely ignore the card,
      // but at least report it as nonworking
      deselectCard();
      return STA_NOINIT | STA_NODISK;
    }
  }

  // Keep sending CMD1 (SEND_OP_COND) command until zero response
  counter = 0xffff;
  do {
    i = sendCommand(SEND_OP_COND, 1L<<30, 1);
    counter--;
  } 
  while (i != 0 && counter > 0);

  if (counter==0) {
    return STA_NOINIT | STA_NODISK;
  }

  // Send MMC CMD16(SET_BLOCKLEN) to 512 bytes
  i = sendCommand(SET_BLOCKLEN, 512, 1);
  if (i != 0) {
    return STA_NOINIT | STA_NODISK;
  }

  // Thats it!
  disk_state = DISK_OK;
  return RES_OK;
}


byte mmc::readSectors(byte *buffer, uint32_t sector, byte count) {
  byte sec,res,tmp,errorcount;
  uint16_t crc,recvcrc;

  for (sec=0;sec<count;sec++) {
    errorcount = 0;
    while (errorcount < CONFIG_SD_AUTO_RETRIES) {
      res = sendCommand(READ_SINGLE_BLOCK, (sector+sec) << 9, 0);

      if (res != 0) {
        SPI_SS_HIGH();
        disk_state = DISK_ERROR;
        return RES_ERROR;
      }

      // Wait for data token
      if (!sdResponse(0xFE)) {
        SPI_SS_HIGH();
        disk_state = DISK_ERROR;
        return RES_ERROR;
      }

      uint16_t i;

      // Get data
      crc = 0;
      for (i=0; i<512; i++) {
        tmp = spiTransferByte(0xff);
        *(buffer++) = tmp;
      }

      // Check CRC
      recvcrc = (spiTransferByte(0xFF) << 8) + spiTransferByte(0xFF);

      break;
    }
    deselectCard();

    if (errorcount >= CONFIG_SD_AUTO_RETRIES) return RES_ERROR;
  }

  return RES_OK;
}


byte mmc::readSectors(byte *buffer, uint32_t sector, byte count, uint16_t offset1, uint16_t offset2) {
  byte sec,res,tmp,errorcount;
  uint16_t crc,recvcrc;

  // offset1 refers to the place where to start writting on the buffer
  // offset2 keeps track of how much is the offset from where to read
  // offset1 + offset2 = BYTESPERSECTOR in the first call
  // after that, offset1 will become 0 and offset2 will keep size
  // until the end, when we will have to consider adding EOFs
  buffer += offset1;

  for (sec=0;sec<count;sec++) {
    errorcount = 0;
    while (errorcount < CONFIG_SD_AUTO_RETRIES) {
      res = sendCommand(READ_SINGLE_BLOCK, (sector+sec) << 9, 0);

      if (res != 0) {
        SPI_SS_HIGH();
        disk_state = DISK_ERROR;
        return RES_ERROR;
      }

      // Wait for data token
      if (!sdResponse(0xFE)) {
        SPI_SS_HIGH();
        disk_state = DISK_ERROR;
        return RES_ERROR;
      }

      uint16_t i;

      // Get data for the first half of the package
      crc = 0;
      if (offset1 + offset2 == BYTESPERSECTOR) 
      {
        // case when we come in the first time
        // the first half of the buffer is filled up
        // with the data from the other file
        for (i=0; i<offset2; i++) {
          tmp = spiTransferByte(0xff);
          *(buffer++) = tmp;
        }
        // finish reading the block to avoid errors
        for (i=offset2; i<512; i++) {
          tmp = spiTransferByte(0xff);
        }
      }
      else {
        // case when we are starting with an empty buffer, but with offset
        // the first offset2 bytes are there from the last time
        for (i=0; i<offset2; i++) {
          tmp = spiTransferByte(0xff);
        }
        // the second 512-offset2 bytes gotta be stored in the array
        for (i=offset2; i<512; i++) {
          tmp = spiTransferByte(0xff);
          *(buffer++) = tmp;
        }
      }

      // Check CRC
      recvcrc = (spiTransferByte(0xFF) << 8) + spiTransferByte(0xFF);

      break;
    }
    deselectCard();

    if (errorcount >= CONFIG_SD_AUTO_RETRIES) return RES_ERROR;
  }

  return RES_OK;
}


byte mmc::writeSectors(const byte *buffer, uint32_t sector, byte count) {
  byte res,sec,errorcount,status;
  uint16_t crc;

  for (sec=0;sec<count;sec++) {
    errorcount = 0;
    while (errorcount < CONFIG_SD_AUTO_RETRIES) {
      res = sendCommand(WRITE_BLOCK, (sector+sec)<<9, 0);

      if (res != 0) {
        SPI_SS_HIGH();
        disk_state = DISK_ERROR;
        return RES_ERROR;
      }

      // Send data token
      spiTransferByte(0xFE);

      uint16_t i;
      const byte *oldbuffer = buffer;

      // Send data
      crc = 0;
      for (i=0; i<512; i++) {
        spiTransferByte(*(buffer++));
      }

      // Send CRC
      spiTransferByte(crc >> 8);
      spiTransferByte(crc & 0xff);

      // Get and check status feedback
      status = spiTransferByte(0xFF);

      // Retry if neccessary
      if ((status & 0x0F) != 0x05) {
        //	uart_putc('X');
        deselectCard();
        errorcount++;
        buffer = oldbuffer;
        continue;
      }

      // Wait for write finish
      if (!sdWaitWriteFinish()) {
        SPI_SS_HIGH();
        disk_state = DISK_ERROR;
        return RES_ERROR;
      }
      break;
    }
    deselectCard();

    if (errorcount >= CONFIG_SD_AUTO_RETRIES) {
      if (!(status & STATUS_CRC_ERROR))
        disk_state = DISK_ERROR;
      return RES_ERROR;
    }
  }

  return RES_OK;
}

