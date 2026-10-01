#include <nds.h>
#include <stddef.h>
#include <stdint.h>

/*
 * NDS EWRAM is only 4 MiB.  The previous 1280 KiB PSX mirror contributed
 * directly to the .bss overflow.  1 MiB still covers the currently wired
 * BODYPROG/B_KONAMI base addresses (highest base: 0x000C9578) while saving
 * 256 KiB.  Later overlays must be streamed/remapped instead of growing this
 * into a 2 MiB PSX mirror.
 */
#define SH_NDS_ARENA_SIZE (1024u * 1024u)
static uint8_t s_psxArena[SH_NDS_ARENA_SIZE] __attribute__((aligned(32)));

void shNdsMemoryInit(void)
{
    dmaFillWords(0, s_psxArena, sizeof(s_psxArena));
}

void* shNdsPsxAddr(uint32_t offset, size_t minimum)
{
    if (offset >= 0x80000000u)
        offset &= 0x001FFFFFu;

    if (minimum > SH_NDS_ARENA_SIZE || offset > SH_NDS_ARENA_SIZE - minimum)
        return NULL;

    return s_psxArena + offset;
}
