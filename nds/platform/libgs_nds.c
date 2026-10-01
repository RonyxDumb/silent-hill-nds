#include <nds.h>
#include <stdint.h>
#include <string.h>
#include <psx/libgte.h>
#include <psx/libgpu.h>
#include "psyq/libgs.h"

extern void DrawOTag(u_long* start);
extern u_long* ClearOTagR(u_long* ot, int n);
extern DISPENV* PutDispEnv(DISPENV* env);
extern DRAWENV* PutDrawEnv(DRAWENV* env);
extern DISPENV* SetDefDispEnv(DISPENV* env, int x, int y, int w, int h);
extern DRAWENV* SetDefDrawEnv(DRAWENV* env, int x, int y, int w, int h);

static DISPENV s_dispEnv[2];
static DRAWENV s_drawEnv[2];
static int s_activeBuffer;
static int s_screenWidth = 256;
static int s_screenHeight = 192;
static int s_lightMode;

_GsFCALL GsFCALL4;
MATRIX GsWSMATRIX;
MATRIX GsWSMATRIX_ORG;
MATRIX GsLSMATRIX;
MATRIX GsLIGHTWSMATRIX;
MATRIX GsIDMATRIX = {{{4096, 0, 0}, {0, 4096, 0}, {0, 0, 4096}}, {0, 0, 0}};
MATRIX GsIDMATRIX2 = {{{4096, 0, 0}, {0, 4096, 0}, {0, 0, 4096}}, {0, 0, 0}};
DISPENV GsDISPENV;
DRAWENV GsDRAWENV;
unsigned long GsLMODE;
unsigned long GsLIGNR;
unsigned long GsLIOFF;
unsigned long GsZOVER;
unsigned long GsBACKC;
unsigned long GsNDIV;

static void initDisplayEnvs(void)
{
    SetDefDispEnv(&s_dispEnv[0], 0, 0, s_screenWidth, s_screenHeight);
    SetDefDispEnv(&s_dispEnv[1], 0, 0, s_screenWidth, s_screenHeight);
    SetDefDrawEnv(&s_drawEnv[0], 0, 0, s_screenWidth, s_screenHeight);
    SetDefDrawEnv(&s_drawEnv[1], 0, 0, s_screenWidth, s_screenHeight);

    s_drawEnv[0].ofs[0] = s_screenWidth / 2;
    s_drawEnv[0].ofs[1] = s_screenHeight / 2;
    s_drawEnv[1].ofs[0] = s_screenWidth / 2;
    s_drawEnv[1].ofs[1] = s_screenHeight / 2;
    s_drawEnv[0].isbg = 1;
    s_drawEnv[1].isbg = 1;
    s_drawEnv[0].dfe = 1;
    s_drawEnv[1].dfe = 1;

    GsDRAWENV = s_drawEnv[0];
    GsDISPENV = s_dispEnv[0];
}

void GsInitGraph(int x, int y, int mode, int dither, int vramMode)
{
    (void)mode;
    (void)dither;
    (void)vramMode;
    s_screenWidth = x;
    s_screenHeight = y;
    s_activeBuffer = 0;
    initDisplayEnvs();
}

void GsInitGraph2(unsigned short x, unsigned short y, unsigned short intmode,
                  unsigned short dith, unsigned short vrammode)
{
    GsInitGraph(x, y, intmode, dith, vrammode);
}

void GsDefDispBuff2(unsigned short x0, unsigned short y0,
                    unsigned short x1, unsigned short y1)
{
    (void)x0;
    (void)y0;
    (void)x1;
    (void)y1;
    initDisplayEnvs();
}

void GsInit3D(void)
{
    InitGeom();
    SetGeomOffset(0, 0);
    SetGeomScreen(240);
    GsWSMATRIX = GsIDMATRIX;
    GsWSMATRIX_ORG = GsIDMATRIX;
    GsLSMATRIX = GsIDMATRIX;
    GsLIGHTWSMATRIX = GsIDMATRIX;
}

void GsInitVcount(void) {}
int GsGetVcount(void) { return 0; }
void GsClearVcount(void) {}

void GsSwapDispBuff(void)
{
    s_activeBuffer ^= 1;
    s_drawEnv[s_activeBuffer].isbg = GsDRAWENV.isbg;
    s_drawEnv[s_activeBuffer].r0 = GsDRAWENV.r0;
    s_drawEnv[s_activeBuffer].g0 = GsDRAWENV.g0;
    s_drawEnv[s_activeBuffer].b0 = GsDRAWENV.b0;
    PutDispEnv(&s_dispEnv[s_activeBuffer]);
    PutDrawEnv(&s_drawEnv[s_activeBuffer]);
}

int GsGetActiveBuff(void)
{
    return s_activeBuffer;
}

void GsDrawOt(GsOT* ot)
{
    if (ot != NULL && ot->tag != NULL)
    {
        DrawOTag((u_long*)ot->tag);
    }
}

void GsClearOt(int offset, int point, GsOT* ot)
{
    int count;

    (void)offset;
    (void)point;
    if (ot == NULL || ot->org == NULL)
    {
        return;
    }

    count = 1 << ot->length;
    ClearOTagR((u_long*)ot->org, count);
    ot->tag = &ot->org[count - 1];
}

void GsSortClear(unsigned char r, unsigned char g, unsigned char b, GsOT* ot)
{
    (void)ot;
    GsDRAWENV.r0 = r;
    GsDRAWENV.g0 = g;
    GsDRAWENV.b0 = b;
    GsDRAWENV.isbg = 1;
    s_drawEnv[s_activeBuffer].r0 = r;
    s_drawEnv[s_activeBuffer].g0 = g;
    s_drawEnv[s_activeBuffer].b0 = b;
    s_drawEnv[s_activeBuffer].isbg = 1;
    PutDrawEnv(&s_drawEnv[s_activeBuffer]);
}

void GsSetAmbient(long r, long g, long b)
{
    SetBackColor(r >> 4, g >> 4, b >> 4);
}

void GsSetFlatLight(int id, GsF_LIGHT* light)
{
    (void)id;
    (void)light;
}

void GsSetLightMode(int mode)
{
    s_lightMode = mode;
}

void GsSetLightMatrix(MATRIX* matrix)
{
    if (matrix != NULL)
    {
        SetLightMatrix(matrix);
    }
}

void GsSetLsMatrix(MATRIX* matrix)
{
    if (matrix != NULL)
    {
        GsLSMATRIX = *matrix;
        SetRotMatrix(matrix);
        SetTransMatrix(matrix);
    }
}

void GsSetProjection(long h)
{
    SetGeomScreen(h);
}

void GsInitCoordinate2(void* super, GsCOORDINATE2* coord)
{
    if (coord == NULL)
    {
        return;
    }

    memset(coord, 0, sizeof(*coord));
    coord->coord = GsIDMATRIX;
    coord->workm = GsIDMATRIX;
    coord->super = (GsCOORDINATE2*)super;
    coord->flg = super == NULL;
}

void GsLinkObject4(unsigned long tmdBase, GsDOBJ2* object, int index)
{
    (void)index;
    if (object != NULL)
    {
        object->tmd = (unsigned long*)(uintptr_t)tmdBase;
    }
}

void GsMapModelingData(unsigned long* data)
{
    (void)data;
}

void GsSortObject4J(GsDOBJ2* object, GsOT* ot, int shift, unsigned long* scratch)
{
    (void)object;
    (void)ot;
    (void)shift;
    (void)scratch;
    (void)s_lightMode;
}

void GsSortOt(GsOT* src, GsOT* dst)
{
    GsOT_TAG* dstTail;

    if (src == NULL || dst == NULL || src->org == NULL || dst->org == NULL || src->tag == NULL)
    {
        return;
    }

    dstTail = &dst->org[0];
    setaddr(&src->org[0], getaddr(dstTail));
    setaddr(dstTail, src->tag);
}

void GsTMDfastG3LFG(void* op, VERT* vp, VERT* np, PACKET* pk, int n, int shift, GsOT* ot, unsigned long* scratch)
{
    (void)op; (void)vp; (void)np; (void)pk; (void)n; (void)shift; (void)ot; (void)scratch;
}

void GsTMDfastTG3LFG(void* op, VERT* vp, VERT* np, PACKET* pk, int n, int shift, GsOT* ot, unsigned long* scratch)
{
    GsTMDfastG3LFG(op, vp, np, pk, n, shift, ot, scratch);
}

void GsTMDfastG4LFG(void* op, VERT* vp, VERT* np, PACKET* pk, int n, int shift, GsOT* ot, unsigned long* scratch)
{
    GsTMDfastG3LFG(op, vp, np, pk, n, shift, ot, scratch);
}

void GsTMDfastTG4LFG(void* op, VERT* vp, VERT* np, PACKET* pk, int n, int shift, GsOT* ot, unsigned long* scratch)
{
    GsTMDfastG3LFG(op, vp, np, pk, n, shift, ot, scratch);
}
