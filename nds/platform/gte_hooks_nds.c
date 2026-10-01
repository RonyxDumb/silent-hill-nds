#include <stddef.h>

/* The software GTE core is taken from PsyCross. PGXP is intentionally disabled
 * on ARM9: it stores doubles and large shadow tables that do not fit the DS
 * memory budget. These symbols isolate that optional desktop-only machinery. */
int g_PsxUsePgxp = 0;
int g_PgxpUseUnquantizedDepth = 0;
float g_PgxpGteOfx = 0.0f, g_PgxpGteOfy = 0.0f, g_PgxpGteH = 0.0f;
int g_PsxWholeMapFar = 0;
int g_PsxWholeMapLastSz = 0;

int GR_NeedViewSpaceData(void) { return 0; }
unsigned PGXP_CurGen(void) { return 0; }
void Shadow_Store(void* a, float x, float y, float w, unsigned v)
{ (void)a; (void)x; (void)y; (void)w; (void)v; }
void VShadow_Store(void* a, float x, float y, float z, float sx, float sy, float w, unsigned v)
{ (void)a; (void)x; (void)y; (void)z; (void)sx; (void)sy; (void)w; (void)v; }

void PsyX_CaptureGteDepths(void* prim) { (void)prim; }
void PsyX_ClearGteDepthTable(void) {}

