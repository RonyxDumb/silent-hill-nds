#include <stdint.h>
#include <string.h>
#include <psx/libgte.h>
#include <psx/libgpu.h>
#include <psyq/libapi.h>
#include "gpu.h"

#define NDS_EVENT_COUNT 32

typedef struct
{
    int active;
    int enabled;
    int signaled;
    long (*callback)(void);
} s_NdsEvent;

static s_NdsEvent s_events[NDS_EVENT_COUNT];
static unsigned int s_criticalSectionDepth;

static MATRIX s_lightMatrix = {{{4096, 0, 0}, {0, 4096, 0}, {0, 0, 4096}}, {0, 0, 0}};
static unsigned long s_psxStackPointer;

unsigned long SetSp(unsigned long newStackPointer)
{
    unsigned long oldStackPointer = s_psxStackPointer;
    s_psxStackPointer = newStackPointer;
    return oldStackPointer;
}

int EnterCriticalSection(void)
{
    ++s_criticalSectionDepth;
    return 1;
}

void ExitCriticalSection(void)
{
    if (s_criticalSectionDepth != 0)
    {
        --s_criticalSectionDepth;
    }
}

int OpenEvent(unsigned int descriptor, int spec, int mode, long (*callback)())
{
    int i;

    (void)descriptor;
    (void)spec;
    (void)mode;
    for (i = 0; i < NDS_EVENT_COUNT; ++i)
    {
        if (!s_events[i].active)
        {
            s_events[i].active = 1;
            s_events[i].enabled = 0;
            s_events[i].signaled = 0;
            s_events[i].callback = callback;
            return i + 1;
        }
    }
    return 0;
}

int CloseEvent(unsigned int event)
{
    if (event <= 0 || event > NDS_EVENT_COUNT)
    {
        return 0;
    }
    memset(&s_events[event - 1], 0, sizeof(s_events[0]));
    return 1;
}

int EnableEvent(unsigned int event)
{
    if (event <= 0 || event > NDS_EVENT_COUNT || !s_events[event - 1].active)
    {
        return 0;
    }
    s_events[event - 1].enabled = 1;
    s_events[event - 1].signaled = 1;
    return 1;
}

int DisableEvent(unsigned int event)
{
    if (event <= 0 || event > NDS_EVENT_COUNT || !s_events[event - 1].active)
    {
        return 0;
    }
    s_events[event - 1].enabled = 0;
    return 1;
}

int TestEvent(unsigned int event)
{
    s_NdsEvent* state;

    if (event <= 0 || event > NDS_EVENT_COUNT)
    {
        return 0;
    }
    state = &s_events[event - 1];
    if (!state->active || !state->enabled)
    {
        return 0;
    }
    if (state->callback != NULL)
    {
        state->callback();
    }
    if (state->signaled)
    {
        state->signaled = 0;
        return 1;
    }
    return 0;
}

int SetRCnt(int counter, unsigned short target, int mode)
{
    (void)counter;
    (void)target;
    (void)mode;
    return 1;
}

int StartRCnt(int counter)
{
    (void)counter;
    return 1;
}

int StopRCnt(int counter)
{
    (void)counter;
    return 1;
}

int ResetRCnt(int counter)
{
    (void)counter;
    return 0;
}

int GetRCnt(int counter)
{
    (void)counter;
    return 0;
}

void SetPriority(PACKET* packet, s32 priority, s32 length)
{
    (void)priority;
    if (packet != NULL)
    {
        setlen(packet, length);
    }
}

void ReadLightMatrix(MATRIX* matrix)
{
    if (matrix != NULL)
    {
        *matrix = s_lightMatrix;
    }
}

void OuterProduct12(VECTOR* left, VECTOR* right, VECTOR* output)
{
    int64_t x;
    int64_t y;
    int64_t z;

    if (left == NULL || right == NULL || output == NULL)
    {
        return;
    }

    x = (int64_t)left->vy * right->vz - (int64_t)left->vz * right->vy;
    y = (int64_t)left->vz * right->vx - (int64_t)left->vx * right->vz;
    z = (int64_t)left->vx * right->vy - (int64_t)left->vy * right->vx;
    output->vx = (long)(x >> 12);
    output->vy = (long)(y >> 12);
    output->vz = (long)(z >> 12);
    output->pad = 0;
}

VECTOR* Square0(VECTOR* input, VECTOR* output)
{
    if (input != NULL && output != NULL)
    {
        output->vx = input->vx * input->vx;
        output->vy = input->vy * input->vy;
        output->vz = input->vz * input->vz;
        output->pad = 0;
    }
    return output;
}

long Lzc(long value)
{
    uint32_t bits = (uint32_t)value;
    long count = 0;

    if (bits == 0)
    {
        return 32;
    }
    if (value < 0)
    {
        bits = ~bits;
    }
    while ((bits & 0x80000000u) == 0)
    {
        ++count;
        bits <<= 1;
    }
    return count;
}

long VectorNormal(VECTOR* input, VECTOR* output)
{
    int64_t squared;
    long magnitude;

    if (input == NULL || output == NULL)
    {
        return 0;
    }
    squared = (int64_t)input->vx * input->vx +
              (int64_t)input->vy * input->vy +
              (int64_t)input->vz * input->vz;
    magnitude = SquareRoot0((long)squared);
    if (magnitude != 0)
    {
        output->vx = (input->vx << 12) / magnitude;
        output->vy = (input->vy << 12) / magnitude;
        output->vz = (input->vz << 12) / magnitude;
    }
    else
    {
        output->vx = 0;
        output->vy = 0;
        output->vz = 0;
    }
    output->pad = 0;
    return (long)squared;
}

void LoadAverageCol(unsigned char* first, unsigned char* second, long firstWeight,
                    long secondWeight, unsigned char* output)
{
    int channel;

    if (first == NULL || second == NULL || output == NULL)
    {
        return;
    }
    for (channel = 0; channel < 3; ++channel)
    {
        long value = ((long)first[channel] * firstWeight +
                      (long)second[channel] * secondWeight) >> 12;
        output[channel] = (unsigned char)(value < 0 ? 0 : value > 255 ? 255 : value);
    }
}

int g_PsyX_UsePerPixelFlashlight = 0;

void IpdCollData_FixOffsets(void* rawData)
{
    uint8_t* base = rawData;
    uint32_t* offsets = (uint32_t*)(base + 0x0C);
    void** pointers = (void**)(base + 0x0C);
    int i;

    for (i = 0; i < 7; ++i)
    {
        pointers[i] = base + offsets[i];
    }
}

bool Vw_AabbVisibleInFrustumCheck(MATRIX* modelMatrix, short minX, short minY,
                                  short minZ, int maxX, int maxY, int maxZ,
                                  unsigned short nearPlane, unsigned short farPlane)
{
    (void)modelMatrix;
    (void)minX;
    (void)minY;
    (void)minZ;
    (void)maxX;
    (void)maxY;
    (void)maxZ;
    (void)nearPlane;
    (void)farPlane;
    return true;
}

MATRIX* TransposeMatrix(MATRIX* source, MATRIX* destination)
{
    int row;
    int column;

    if (source == NULL || destination == NULL)
    {
        return destination;
    }
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 3; ++column)
        {
            destination->m[row][column] = source->m[column][row];
        }
    }
    destination->t[0] = source->t[0];
    destination->t[1] = source->t[1];
    destination->t[2] = source->t[2];
    return destination;
}

DRAWENV* GetDrawEnv(DRAWENV* environment)
{
    extern DRAWENV GsDRAWENV;

    if (environment != NULL)
    {
        *environment = GsDRAWENV;
    }
    return environment;
}

void SetDrawEnv(DR_ENV* packet, DRAWENV* environment)
{
    (void)environment;
    if (packet != NULL)
    {
        setlen(packet, 0);
    }
}

void SetDrawMove(DR_MOVE* packet, RECT* rectangle, int x, int y)
{
    (void)rectangle;
    (void)x;
    (void)y;
    if (packet != NULL)
    {
        setlen(packet, 0);
    }
}
