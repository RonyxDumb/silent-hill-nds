#ifndef SH_NDS_PSX_MEMORY_H
#define SH_NDS_PSX_MEMORY_H
#include <stddef.h>
#include <stdint.h>
void* shNdsPsxAddr(uint32_t offset, size_t minimum);
#define PSX_ADDR(offset) shNdsPsxAddr((uint32_t)(offset), 1u)
#endif

