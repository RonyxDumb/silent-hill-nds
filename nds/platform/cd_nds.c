#include <stdint.h>
#include <string.h>
#include <psyq/libcd.h>

#define NDS_CD_SECTOR_SIZE 2048

static int s_cdSector;
static CdlLOC s_cdLocation;

static unsigned int shNdsBcdToBinary(unsigned int value)
{
    return ((value >> 4) * 10u) + (value & 0x0fu);
}

static unsigned char shNdsBinaryToBcd(unsigned int value)
{
    return (unsigned char)(((value / 10u) << 4) | (value % 10u));
}

CdlLOC* CdIntToPos(int sector, CdlLOC* location)
{
    unsigned int absoluteSector = sector < 0 ? 0u : (unsigned int)sector + 150u;

    if (location != NULL)
    {
        location->minute = shNdsBinaryToBcd(absoluteSector / (60u * 75u));
        absoluteSector %= 60u * 75u;
        location->second = shNdsBinaryToBcd(absoluteSector / 75u);
        location->sector = shNdsBinaryToBcd(absoluteSector % 75u);
        location->track = 0;
    }
    return location;
}

int CdPosToInt(CdlLOC* location)
{
    unsigned int absoluteSector;

    if (location == NULL)
    {
        return 0;
    }
    absoluteSector = (shNdsBcdToBinary(location->minute) * 60u +
                      shNdsBcdToBinary(location->second)) * 75u +
                     shNdsBcdToBinary(location->sector);
    return absoluteSector >= 150u ? (int)(absoluteSector - 150u) : 0;
}

int CdControl(unsigned char command, unsigned char* parameter, unsigned char* result)
{
    if (command == CdlSetloc && parameter != NULL)
    {
        memcpy(&s_cdLocation, parameter, sizeof(s_cdLocation));
        s_cdSector = CdPosToInt(&s_cdLocation);
    }
    if (result != NULL)
    {
        memset(result, 0, 8);
    }
    return 1;
}

int CdControlB(unsigned char command, unsigned char* parameter, unsigned char* result)
{
    return CdControl(command, parameter, result);
}

int CdControlF(unsigned char command, unsigned char* parameter)
{
    return CdControl(command, parameter, NULL);
}

int CdSync(int mode, unsigned char* result)
{
    (void)mode;
    if (result != NULL)
    {
        memset(result, 0, 8);
    }
    return 0;
}

int CdReadSync(int mode, unsigned char* result)
{
    return CdSync(mode, result);
}

int CdRead(int sectors, unsigned long* destination, int mode)
{
    (void)mode;
    if (sectors < 0 || destination == NULL)
    {
        return 0;
    }

    /* The native filesystem backend must replace this PS1 sector interface.
     * Returning a completed zeroed read keeps optional CD-audio/stream paths
     * from touching uninitialized memory until their asset mapping is added. */
    memset(destination, 0, (size_t)sectors * NDS_CD_SECTOR_SIZE);
    s_cdSector += sectors;
    CdIntToPos(s_cdSector, &s_cdLocation);
    return 1;
}

int CdRead2(int mode)
{
    (void)mode;
    return 1;
}

int CdReset(int mode)
{
    (void)mode;
    s_cdSector = 0;
    memset(&s_cdLocation, 0, sizeof(s_cdLocation));
    return 1;
}

int CdMix(CdlATV* volume)
{
    (void)volume;
    return 1;
}

void CdFlush(void) {}

int CdInit(void)
{
    return CdReset(0);
}
