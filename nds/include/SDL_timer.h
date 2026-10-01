#ifndef SH_NDS_SDL_TIMER_H
#define SH_NDS_SDL_TIMER_H

/* Minimal SDL timer surface used by the original PC-port diagnostics and
 * frame-pacing code. The actual implementation is provided by the ARM9
 * platform adapter, so the Nintendo DS build does not link against SDL. */
#include <stdint.h>

typedef uint32_t Uint32;

#endif
