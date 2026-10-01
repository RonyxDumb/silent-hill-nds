#ifndef SH_NDS_COMPAT_H
#define SH_NDS_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#define SH_NDS_PORT 1
#define SH_PLATFORM_CONSTRAINED_MEMORY 1
#define SH_STATIC_MAPS 1
#define USE_PGXP 0
#define PSYX_SKIP_FRAMEBUFFER_STORE 1
#ifndef __cplusplus
#ifndef static_assert
#define static_assert _Static_assert
#endif
#endif
#define SDL_GetTicks() shNdsTicksMs()
#define SDL_Delay(milliseconds) shNdsDelayMs((unsigned int)(milliseconds))

unsigned int shNdsTicksMs(void);
void shNdsDelayMs(unsigned int milliseconds);
void* shNdsPsxAddr(uint32_t offset, size_t minimum);

/* The original portable branches use PSX_ADDR. On DS it resolves into a
 * checked, bounded arena instead of a 3 MiB PC array or literal PSX address. */
#ifndef PSX_ADDR
#define PSX_ADDR(offset) shNdsPsxAddr((uint32_t)(offset), 1u)
#endif

#endif
