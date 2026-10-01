#ifndef SH_NDS_VRAM_H
#define SH_NDS_VRAM_H

#include <psx/libgpu.h>

void shNdsVramInit(void);
int shNdsVramLoad(const RECT16* rect, const u_long* source);
int shNdsVramMove(const RECT16* rect, int dstX, int dstY);
int shNdsTextureBind(unsigned short tpage, unsigned short clut);
void shNdsTextureInvalidateRect(const RECT16* rect);
void shNdsTextureCacheReset(void);

#endif

