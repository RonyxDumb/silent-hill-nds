#include <nds.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Keep this translation unit on the libnds side of the ABI boundary.
 * Do not include Silent Hill's decomp headers here: they define s32/u32/bool
 * differently from Calico/libnds on this toolchain.
 */
void shNdsMemoryInit(void);
void shNdsInputInit(void);
bool shNdsFilesystemInit(void);
void shNdsRendererInit(void);
void* shNdsPsxAddr(uint32_t offset, size_t minimum);

void Fs_QueueInitialize(void);
void MainLoop(void);

/* Symbols normally owned by the PSX boot overlay. */
void* g_OvlDynamic;
void* g_OvlBodyprog;

static void shNdsBootFail(const char* reason)
{
    iprintf("\n[FATAL] %s\n", reason);
    for (;;)
        swiWaitForVBlank();
}

int main(void)
{
    consoleDemoInit();

    iprintf("Silent Hill DS\n");
    iprintf("[BOOT] ARM9 started\n");

    iprintf("[1/6] memory...");
    shNdsMemoryInit();
    iprintf(" OK\n");

    iprintf("[2/6] NitroFS...");
    if (!shNdsFilesystemInit())
        shNdsBootFail("nitroFSInit failed");
    iprintf(" OK\n");

    iprintf("[3/6] input...");
    shNdsInputInit();
    iprintf(" OK\n");

    iprintf("[4/6] renderer...");
    shNdsRendererInit();
    iprintf(" OK\n");

    iprintf("[5/6] PSX arena...");
    g_OvlBodyprog = shNdsPsxAddr(0x00024B60u, 1u);
    g_OvlDynamic  = shNdsPsxAddr(0x000C9578u, 1u);

    if (g_OvlBodyprog == NULL || g_OvlDynamic == NULL)
        shNdsBootFail("PSX arena mapping failed");
    iprintf(" OK\n");

    iprintf("[6/6] FS queue...");
    Fs_QueueInitialize();
    iprintf(" OK\n");

    iprintf("\nEntering original MainLoop...\n");
    MainLoop();

    shNdsBootFail("MainLoop returned");
    return 0;
}
