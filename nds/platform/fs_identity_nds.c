#include <stdio.h>
#include <string.h>

#include "main/fileinfo.h"

/*
 * Resolve an original e_FsFile through the original g_FileTable.
 * This is the only accepted identity boundary: callers never pass
 * ad-hoc filenames.
 *
 * NOTE:
 * Do not include <nds.h> here.
 * This translation unit does not use libnds APIs directly, and including
 * nds.h pulls in Calico's s32/u32/bool typedefs, which conflict with the
 * original Silent Hill decomp typedefs from decomp/types.h.
 */
static void unpackName(char out[13], const s_FileInfo* info)
{
    const unsigned long words[2] = {
        info->name0123,
        info->name4567
    };

    unsigned i;

    for (i = 0; i < 8; ++i)
    {
        unsigned v = (words[i >> 2] >> ((i & 3) * 6)) & 0x3F;
        out[i] = (char)(v + 0x20);
    }

    out[8] = '\0';

    while (i && out[i - 1] == ' ')
    {
        out[--i] = '\0';
    }
}

int shNdsOpenFileId(e_FsFile id, FILE** output)
{
    static const char* dirs[] = {
        "1ST",
        "ANIM",
        "BG",
        "CHARA",
        "ITEM",
        "MISC",
        "SND",
        "TEST",
        "TIM",
        "VIN",
        "XA"
    };

    char name[13];
    char path[64];

    const s_FileInfo* info;

    if (output == NULL)
    {
        return 0;
    }

    *output = NULL;

    if ((unsigned)id >= FS_FILE_COUNT)
    {
        return 0;
    }

    info = &g_FileTable[id];

    if ((unsigned)info->pathIdx >= (sizeof(dirs) / sizeof(dirs[0])))
    {
        return 0;
    }

    unpackName(name, info);

    snprintf(
        path,
        sizeof(path),
        "nitro:/%s/%s",
        dirs[info->pathIdx],
        name
    );

    *output = fopen(path, "rb");

    return (*output != NULL);
}
