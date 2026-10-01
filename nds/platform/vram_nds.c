#include <nds.h>
#include <stdint.h>
#include <string.h>
#include <psx/libgte.h>
#include "nds_vram.h"

#define PSX_VRAM_W 1024
#define PSX_VRAM_H 512
#define TILE_SHIFT 4
#define TILE_SIZE (1 << TILE_SHIFT)
#define TILE_COLS (PSX_VRAM_W / TILE_SIZE)
#define TILE_ROWS (PSX_VRAM_H / TILE_SIZE)
#define TILE_COUNT (TILE_COLS * TILE_ROWS)
#define TILE_WORDS (TILE_SIZE * TILE_SIZE)
#define TILE_POOL_COUNT 256
#define TEX_CACHE_COUNT 2

typedef struct { uint16_t pixels[TILE_WORDS]; } VramTile;
typedef struct {
    uint16_t tpage, clut;
    int textureId;
    uint32_t stamp;
    uint8_t valid;
} TextureCacheEntry;

static VramTile s_tilePool[TILE_POOL_COUNT] __attribute__((aligned(32)));
static VramTile* s_tiles[TILE_COUNT];
static uint16_t s_freeTiles[TILE_POOL_COUNT];
static unsigned s_freeCount;
static TextureCacheEntry s_cache[TEX_CACHE_COUNT];
static uint16_t s_convert[256 * 256] __attribute__((aligned(32)));
static uint32_t s_stamp;

static unsigned tileIndex(unsigned x, unsigned y)
{
    return (y >> TILE_SHIFT) * TILE_COLS + (x >> TILE_SHIFT);
}

static VramTile* tileGet(unsigned x, unsigned y, int create)
{
    unsigned idx;
    VramTile* tile;
    if (x >= PSX_VRAM_W || y >= PSX_VRAM_H) return 0;
    idx = tileIndex(x, y);
    tile = s_tiles[idx];
    if (!tile && create && s_freeCount) {
        tile = &s_tilePool[s_freeTiles[--s_freeCount]];
        memset(tile, 0, sizeof(*tile));
        s_tiles[idx] = tile;
    }
    return tile;
}

static uint16_t pixelRead(unsigned x, unsigned y)
{
    VramTile* tile = tileGet(x, y, 0);
    if (!tile) return 0;
    return tile->pixels[((y & 15u) << TILE_SHIFT) | (x & 15u)];
}

static void pixelWrite(unsigned x, unsigned y, uint16_t value)
{
    VramTile* tile = tileGet(x, y, 1);
    if (tile) tile->pixels[((y & 15u) << TILE_SHIFT) | (x & 15u)] = value;
}

static uint16_t psxColorToDs(uint16_t color)
{
    unsigned r = color & 31u;
    unsigned g = (color >> 5) & 31u;
    unsigned b = (color >> 10) & 31u;
    if ((color & 0x7fffu) == 0) return 0;
    return (uint16_t)(RGB15(r, g, b) | BIT(15));
}

void shNdsVramInit(void)
{
    unsigned i;
    memset(s_tiles, 0, sizeof(s_tiles));
    memset(s_cache, 0, sizeof(s_cache));
    for (i=0; i<TILE_POOL_COUNT; ++i) s_freeTiles[i] = (uint16_t)i;
    s_freeCount = TILE_POOL_COUNT;
    s_stamp = 1;
}

void shNdsTextureCacheReset(void)
{
    unsigned i;
    for (i=0; i<TEX_CACHE_COUNT; ++i) {
        if (s_cache[i].valid) glDeleteTextures(1, &s_cache[i].textureId);
        s_cache[i].valid = 0;
    }
}

void shNdsTextureInvalidateRect(const RECT16* rect)
{
    unsigned i;
    (void)rect;
    /* Correctness first: LoadImage may alter a CLUT outside the texture page,
     * so invalidate both bounded cache entries. */
    for (i=0; i<TEX_CACHE_COUNT; ++i) {
        if (s_cache[i].valid) glDeleteTextures(1, &s_cache[i].textureId);
        s_cache[i].valid = 0;
    }
}

int shNdsVramLoad(const RECT16* rect, const u_long* source)
{
    const uint16_t* in = (const uint16_t*)source;
    int x, y;
    if (!rect || !source || rect->w <= 0 || rect->h <= 0) return -1;
    for (y=0; y<rect->h; ++y)
        for (x=0; x<rect->w; ++x)
            pixelWrite((unsigned)(rect->x+x), (unsigned)(rect->y+y), *in++);
    shNdsTextureInvalidateRect(rect);
    return 0;
}

int shNdsVramMove(const RECT16* rect, int dstX, int dstY)
{
    int x, y;
    uint16_t line[1024];
    if (!rect || rect->w <= 0 || rect->w > 1024 || rect->h <= 0) return -1;
    for (y=0; y<rect->h; ++y) {
        for (x=0; x<rect->w; ++x) line[x]=pixelRead(rect->x+x,rect->y+y);
        for (x=0; x<rect->w; ++x) pixelWrite(dstX+x,dstY+y,line[x]);
    }
    shNdsTextureCacheReset();
    return 0;
}

static void decodePage(uint16_t tpage, uint16_t clut)
{
    unsigned depth=(tpage>>7)&3u, baseX=(tpage&15u)*64u, baseY=(tpage&16u)*16u;
    unsigned clutX=(clut&63u)*16u, clutY=(clut>>6)&511u;
    unsigned x,y;
    for (y=0; y<256; ++y) for (x=0; x<256; ++x) {
        uint16_t color;
        if (depth==0) {
            uint16_t word=pixelRead(baseX+(x>>2),baseY+y);
            color=pixelRead(clutX+((word>>((x&3u)*4u))&15u),clutY);
        } else if (depth==1) {
            uint16_t word=pixelRead(baseX+(x>>1),baseY+y);
            color=pixelRead(clutX+((word>>((x&1u)*8u))&255u),clutY);
        } else {
            color=pixelRead(baseX+x,baseY+y);
        }
        s_convert[y*256+x]=psxColorToDs(color);
    }
}

int shNdsTextureBind(unsigned short tpage, unsigned short clut)
{
    TextureCacheEntry* entry=0;
    unsigned i, victim=0;
    for (i=0;i<TEX_CACHE_COUNT;++i) {
        if (s_cache[i].valid && s_cache[i].tpage==tpage && s_cache[i].clut==clut) { entry=&s_cache[i]; break; }
        if (!s_cache[i].valid || s_cache[i].stamp<s_cache[victim].stamp) victim=i;
    }
    if (!entry) {
        entry=&s_cache[victim];
        if (entry->valid) glDeleteTextures(1,&entry->textureId);
        decodePage(tpage,clut);
        glGenTextures(1,&entry->textureId);
        glBindTexture(0,entry->textureId);
        glTexImage2D(0,0,GL_RGBA,TEXTURE_SIZE_256,TEXTURE_SIZE_256,0,
                     TEXGEN_TEXCOORD|GL_TEXTURE_COLOR0_TRANSPARENT,s_convert);
        entry->tpage=tpage; entry->clut=clut; entry->valid=1;
    } else glBindTexture(0,entry->textureId);
    entry->stamp=++s_stamp;
    return entry->textureId;
}
