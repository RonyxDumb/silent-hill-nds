#ifndef SH_NDS_PSX_RENDER_COMPAT_H
#define SH_NDS_PSX_RENDER_COMPAT_H

/* Minimal renderer surface used by original decomp files under SH_PC_PORT.
 * Desktop OpenGL types must never enter the ARM9 build. */
extern int g_PsxSkipFramebufferStore;
void GR_DirectUploadVRAMRegion(int x, int y, int w, int h);
float GR_LivePixelAspect(void);
int GR_HorPlusHalfWidths(float* out43, float* outWide);
void GR_SetSceneFbRedirect(int x, int y, int w, int h);

#endif

