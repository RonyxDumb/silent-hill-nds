#include <nds.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <filesystem.h>
#include <psx/libpad.h>

void shNdsVramInit(void);

static uint32_t s_frames;
static void (*s_vsyncCallback)(void);

int VSync(int mode)
{
    if (mode >= 0)
    {
        shNdsFrameWait();
    }

    if (s_vsyncCallback != NULL)
    {
        s_vsyncCallback();
    }

    return (int)s_frames;
}

int VSyncCallback(void (*callback)(void))
{
    void (*previous)(void) = s_vsyncCallback;

    s_vsyncCallback = callback;
    return (int)(uintptr_t)previous;
}

/* Null logging sink required by PC-port diagnostic code that is also present
 * in the original game translation units selected for the ARM9 build. */
FILE* g_ShDebugLog = NULL;

unsigned int shNdsTicksMs(void)
{
    return (unsigned int)(((uint64_t)s_frames * 1000u) / 60u);
}

void shNdsDelayMs(unsigned int milliseconds)
{
    uint32_t frames = (milliseconds * 60u + 999u) / 1000u;

    while (frames-- != 0u)
    {
        swiWaitForVBlank();
        scanKeys();
        s_frames++;
    }
}

static uint8_t* s_padBuffers[2];

static uint16_t shNdsPadButtons(void)
{
    uint32_t keys = keysHeld();
    uint16_t buttons = 0xFFFFu;

    if (keys & KEY_SELECT) buttons &= (uint16_t)~(1u << 0);
    if (keys & KEY_L)      buttons &= (uint16_t)~(1u << 2);
    if (keys & KEY_R)      buttons &= (uint16_t)~(1u << 3);
    if (keys & KEY_START)  buttons &= (uint16_t)~(1u << 11);
    if (keys & KEY_UP)     buttons &= (uint16_t)~(1u << 12);
    if (keys & KEY_RIGHT)  buttons &= (uint16_t)~(1u << 13);
    if (keys & KEY_DOWN)   buttons &= (uint16_t)~(1u << 14);
    if (keys & KEY_LEFT)   buttons &= (uint16_t)~(1u << 15);
    if (keys & KEY_L)      buttons &= (uint16_t)~(1u << 8);
    if (keys & KEY_R)      buttons &= (uint16_t)~(1u << 9);
    if (keys & KEY_A)      buttons &= (uint16_t)~(1u << 14);
    if (keys & KEY_B)      buttons &= (uint16_t)~(1u << 13);
    if (keys & KEY_X)      buttons &= (uint16_t)~(1u << 15);
    if (keys & KEY_Y)      buttons &= (uint16_t)~(1u << 12);
    return buttons;
}

static void shNdsPadUpdate(void)
{
    int port;

    for (port = 0; port < 2; ++port)
    {
        uint8_t* pad = s_padBuffers[port];
        uint16_t buttons;

        if (pad == NULL)
        {
            continue;
        }
        buttons = shNdsPadButtons();
        pad[0] = 0;
        pad[1] = 0x73;
        pad[2] = (uint8_t)(buttons >> 8);
        pad[3] = (uint8_t)buttons;
        pad[4] = 0x80;
        pad[5] = 0x80;
        pad[6] = 0x80;
        pad[7] = 0x80;
    }
}

void PadInitDirect(unsigned char* pad1, unsigned char* pad2)
{
    s_padBuffers[0] = pad1;
    s_padBuffers[1] = pad2;
    shNdsPadUpdate();
}

void PadStartCom(void) { shNdsPadUpdate(); }
void PadStopCom(void) {}
int PadChkVsync(void) { shNdsPadUpdate(); return 0; }
unsigned int PadEnableCom(unsigned int mode) { return mode; }
int PadGetState(int port) { return (port >= 0 && port < 2 && s_padBuffers[port] != NULL) ? PadStateStable : PadStateDiscon; }
int PadInfoMode(int port, int term, int offs) { (void)port; (void)term; (void)offs; return 0x73; }
int PadInfoAct(int port, int acno, int term) { (void)port; (void)acno; (void)term; return 0; }
int PadSetActAlign(int port, unsigned char* table) { (void)port; (void)table; return 0; }
int PadSetMainMode(int socket, int offs, int lock) { (void)socket; (void)offs; (void)lock; return 0; }
void PadSetAct(int port, unsigned char* table, int length) { (void)port; (void)table; (void)length; }

void shNdsInputInit(void) { scanKeys(); }

bool shNdsFilesystemInit(void)
{
    return nitroFSInit(NULL);
}

void shNdsRendererInit(void)
{
    videoSetMode(MODE_0_3D);
    vramSetBankA(VRAM_A_TEXTURE);
    vramSetBankB(VRAM_B_TEXTURE);

    /*
     * Keep VRAM C assigned to the SUB-screen console created by
     * consoleDemoInit(). Reassigning it to MAIN_BG makes all boot/error
     * diagnostics disappear and produces an apparent white-screen hang.
     */
    glInit();
    glEnable(GL_TEXTURE_2D | GL_ANTIALIAS);
    glClearColor(0, 0, 0, 31);
    glClearDepth(0x7FFF);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    shNdsVramInit();
    glOrthof32(0, inttof32(256), inttof32(192), 0, -inttof32(1), inttof32(1));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Present a known frame so a successful renderer init is visible. */
    glFlush(0);
    swiWaitForVBlank();
}

/* Called by the PsyQ VSync adapter once per original game frame. */
void shNdsFrameWait(void)
{
    swiWaitForVBlank();
    scanKeys();
    shNdsPadUpdate();
    s_frames++;
}
