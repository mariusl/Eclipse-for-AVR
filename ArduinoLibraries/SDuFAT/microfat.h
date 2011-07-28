#ifndef __MICROFAT_H__
#define __MICROFAT_H__

#include <inttypes.h>

typedef struct
{
  byte bootable;
  byte chsAddrOfFirstSector[3];
  byte partitionType;
  byte chsAddrOfLastSector[3];
  uint32_t lbaAddrOfFirstSector;
  uint32_t partitionLengthSectors;
}
partition_record;

typedef struct
{
  byte jump[3];
  char oemName[8];
  uint16_t bytesPerSector;
  byte sectorsPerCluster;
  uint16_t reservedSectors;
  byte fatCopies;
  uint16_t rootDirectoryEntries;
  uint16_t totalFilesystemSectors;
  byte mediaDescriptor;
  uint16_t sectorsPerFAT;
  uint16_t sectorsPerTrack;
  uint16_t headCount;

  uint32_t hiddenSectors;
  uint32_t totalFilesystemSectors2;
  byte logicalDriveNum;
  byte reserved;
  byte extendedSignature;
  uint32_t partitionSerialNum;
  char volumeLabel[11];
  char fsType[8];
  byte bootstrapCode[447];
  byte signature[2];
}
boot_sector;

typedef struct
{
  char filespec[11];
  byte attributes;
  byte reserved[10];
  uint16_t time;
  uint16_t date;
  uint16_t startCluster;
  uint32_t fileSize;
}
directory_entry;


namespace microfat
{
  bool initialize(byte* buffer);

  // get start sector, file size for given filename
  //
  bool locateFileStart(const char* filename, unsigned long& sector, unsigned long& size);
};


#endif // __MICROFAT_H__
