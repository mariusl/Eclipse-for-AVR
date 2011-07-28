#include <WProgram.h>
#include "microfat.h"
#include "mmc.h"


static struct
{
  unsigned short sectorsPerCluster;

  unsigned long rootDirSect, cluster2;

  byte* buffer;
}
vars;


bool microfat::initialize(byte* buffer)
{
  vars.buffer = buffer;

  if (RES_OK != mmc::readSectors(vars.buffer, 0, 1))
  {
    return false;
  }

  partition_record* p = (partition_record*)&vars.buffer[0x1be];
  long bootSector = p->lbaAddrOfFirstSector;

  if (RES_OK != mmc::readSectors(vars.buffer, bootSector, 1))
  {
    return false;
  }

  boot_sector* b = (boot_sector*)vars.buffer;

  if (BYTESPERSECTOR != b->bytesPerSector)
  {
    return false;
  }

  vars.sectorsPerCluster = b->sectorsPerCluster;

  vars.rootDirSect = bootSector + b->reservedSectors + (b->fatCopies * b->sectorsPerFAT);

  long dirBytes = b->rootDirectoryEntries * 32;

  long dirSects = dirBytes / BYTESPERSECTOR;
  if (dirBytes % BYTESPERSECTOR != 0)
  {
    ++dirSects;
  }

  vars.cluster2 = vars.rootDirSect + dirSects;

  return true;
}



// get start sector for given filename
//
bool microfat::locateFileStart(const char* filename, unsigned long& sector, unsigned long& size)
{
  if (RES_OK == mmc::readSectors(vars.buffer, vars.rootDirSect, 1))
  {
    char cookedName[11];
    for(int i = 0; i < 12; ++i)
    {
      cookedName[i] = 0x20;
    }

    for (int i = 0, j = 0; i < 12 && filename[i]; ++i)
    {
      if (filename[i] != '.')
      {
        cookedName[j] = filename[i] >= 96 ? filename[i] - 32 : filename[i];
        ++j;
      }
      else
      {
        j = 8;
      }
    }

    for (int i = 0; i < BYTESPERSECTOR; i += 32)
    {
      directory_entry* de = (directory_entry*)&vars.buffer[i];

      // don't match with deleted, [system/volname/subdir/hidden] files
      if (de->filespec[0] != 0xe5 && (de->attributes & 0x1e) == 0 && memcmp(cookedName, de->filespec, 11) == 0)
      {
        sector = vars.cluster2 + ((de->startCluster-2) * vars.sectorsPerCluster);
        size = de->fileSize;
        return true;
      }
    }
  }

  return false;
}

