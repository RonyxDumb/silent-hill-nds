#include <nds.h>
#include <stdint.h>
#include <string.h>
#include <psx/libgte.h>
#include <psx/libgpu.h>
#include "psyq/libgs.h"
#include "nds_vram.h"

OT_TAG prim_terminator = {0};
int (*GPU_printf)(const char *fmt, ...) = 0;
static DRAWENV s_draw;
static DISPENV s_disp;
static void (*s_drawSyncCallback)(void);
static uint16_t s_currentTPage;
int g_PsxSkipFramebufferStore;

/* The original engine still builds PSY-Q ordering-table packets.  PsyCross'
 * extended packet representation keeps the link pointer in P_TAG, so use its
 * helpers rather than assuming the old 24-bit PS1 tag layout. */
PACKET* GsOUT_PACKET_P;

void AddPrim(void* ot, void* p)
{
    if (ot != NULL && p != NULL)
    {
        addPrim(ot, p);
    }
}

void AddPrims(void* ot, void* p0, void* p1)
{
    if (ot != NULL && p0 != NULL && p1 != NULL)
    {
        addPrims(ot, p0, p1);
    }
}

void CatPrim(void* p0, void* p1)
{
    if (p0 != NULL && p1 != NULL)
    {
        catPrim(p0, p1);
    }
}

void GR_DirectUploadVRAMRegion(int x,int y,int w,int h)
{ (void)x;(void)y;(void)w;(void)h; shNdsTextureCacheReset(); }
float GR_LivePixelAspect(void) { return 1.0f; }
int GR_HorPlusHalfWidths(float* out43,float* outWide)
{ if (out43) *out43=128.0f; if (outWide) *outWide=128.0f; return 0; }
void GR_SetSceneFbRedirect(int x,int y,int w,int h)
{ (void)x;(void)y;(void)w;(void)h; }

static void color3(unsigned r, unsigned g, unsigned b)
{
    glColor3b((uint8_t)r, (uint8_t)g, (uint8_t)b);
}

static void vertex2(int x, int y)
{
    glVertex3v16(inttov16(x), inttov16(y), 0);
}

static void texcoord(unsigned u, unsigned v)
{
    glTexCoord2t16(inttot16((int)u), inttot16((int)v));
}

static void drawPrimitive(void* raw)
{
    P_TAG* tag = (P_TAG*)raw;
    unsigned code = tag->code & 0xfc;
    switch (code) {
    case 0x20: { POLY_F3* p=raw; glBegin(GL_TRIANGLES); color3(p->r0,p->g0,p->b0); vertex2(p->x0,p->y0); vertex2(p->x1,p->y1); vertex2(p->x2,p->y2); glEnd(); break; }
    case 0x28: { POLY_F4* p=raw; glBegin(GL_QUADS); color3(p->r0,p->g0,p->b0); vertex2(p->x0,p->y0); vertex2(p->x1,p->y1); vertex2(p->x3,p->y3); vertex2(p->x2,p->y2); glEnd(); break; }
    case 0x30: { POLY_G3* p=raw; glBegin(GL_TRIANGLES); color3(p->r0,p->g0,p->b0); vertex2(p->x0,p->y0); color3(p->r1,p->g1,p->b1); vertex2(p->x1,p->y1); color3(p->r2,p->g2,p->b2); vertex2(p->x2,p->y2); glEnd(); break; }
    case 0x38: { POLY_G4* p=raw; glBegin(GL_QUADS); color3(p->r0,p->g0,p->b0); vertex2(p->x0,p->y0); color3(p->r1,p->g1,p->b1); vertex2(p->x1,p->y1); color3(p->r3,p->g3,p->b3); vertex2(p->x3,p->y3); color3(p->r2,p->g2,p->b2); vertex2(p->x2,p->y2); glEnd(); break; }
    /* Textured packets retain their GTE positions and vertex colours. Until
     * the PS1 VRAM/CLUT cache is complete they render coloured, never vanish. */
    case 0x24: { POLY_FT3* p=raw; shNdsTextureBind(p->tpage,p->clut); glBegin(GL_TRIANGLES); color3(p->r0,p->g0,p->b0); texcoord(p->u0,p->v0);vertex2(p->x0,p->y0);texcoord(p->u1,p->v1);vertex2(p->x1,p->y1);texcoord(p->u2,p->v2);vertex2(p->x2,p->y2);glEnd(); break; }
    case 0x2c: { POLY_FT4* p=raw; shNdsTextureBind(p->tpage,p->clut); glBegin(GL_QUADS); color3(p->r0,p->g0,p->b0);texcoord(p->u0,p->v0);vertex2(p->x0,p->y0);texcoord(p->u1,p->v1);vertex2(p->x1,p->y1);texcoord(p->u3,p->v3);vertex2(p->x3,p->y3);texcoord(p->u2,p->v2);vertex2(p->x2,p->y2);glEnd(); break; }
    case 0x34: { POLY_GT3* p=raw; shNdsTextureBind(p->tpage,p->clut); glBegin(GL_TRIANGLES);color3(p->r0,p->g0,p->b0);texcoord(p->u0,p->v0);vertex2(p->x0,p->y0);color3(p->r1,p->g1,p->b1);texcoord(p->u1,p->v1);vertex2(p->x1,p->y1);color3(p->r2,p->g2,p->b2);texcoord(p->u2,p->v2);vertex2(p->x2,p->y2);glEnd(); break; }
    case 0x3c: { POLY_GT4* p=raw; shNdsTextureBind(p->tpage,p->clut); glBegin(GL_QUADS);color3(p->r0,p->g0,p->b0);texcoord(p->u0,p->v0);vertex2(p->x0,p->y0);color3(p->r1,p->g1,p->b1);texcoord(p->u1,p->v1);vertex2(p->x1,p->y1);color3(p->r3,p->g3,p->b3);texcoord(p->u3,p->v3);vertex2(p->x3,p->y3);color3(p->r2,p->g2,p->b2);texcoord(p->u2,p->v2);vertex2(p->x2,p->y2);glEnd(); break; }
    case 0x60: { TILE* p=raw; glBegin(GL_QUADS); color3(p->r0,p->g0,p->b0); vertex2(p->x0,p->y0); vertex2(p->x0+p->w,p->y0); vertex2(p->x0+p->w,p->y0+p->h); vertex2(p->x0,p->y0+p->h); glEnd(); break; }
    case 0x64: { SPRT* p=raw; shNdsTextureBind(s_currentTPage,p->clut); glBegin(GL_QUADS);color3(p->r0,p->g0,p->b0);texcoord(p->u0,p->v0);vertex2(p->x0,p->y0);texcoord(p->u0+p->w,p->v0);vertex2(p->x0+p->w,p->y0);texcoord(p->u0+p->w,p->v0+p->h);vertex2(p->x0+p->w,p->y0+p->h);texcoord(p->u0,p->v0+p->h);vertex2(p->x0,p->y0+p->h);glEnd();break; }
    case 0x74: { SPRT_8* p=raw; shNdsTextureBind(s_currentTPage,p->clut); glBegin(GL_QUADS);color3(p->r0,p->g0,p->b0);texcoord(p->u0,p->v0);vertex2(p->x0,p->y0);texcoord(p->u0+8,p->v0);vertex2(p->x0+8,p->y0);texcoord(p->u0+8,p->v0+8);vertex2(p->x0+8,p->y0+8);texcoord(p->u0,p->v0+8);vertex2(p->x0,p->y0+8);glEnd();break; }
    case 0x7c: { SPRT_16* p=raw; shNdsTextureBind(s_currentTPage,p->clut); glBegin(GL_QUADS);color3(p->r0,p->g0,p->b0);texcoord(p->u0,p->v0);vertex2(p->x0,p->y0);texcoord(p->u0+16,p->v0);vertex2(p->x0+16,p->y0);texcoord(p->u0+16,p->v0+16);vertex2(p->x0+16,p->y0+16);texcoord(p->u0,p->v0+16);vertex2(p->x0,p->y0+16);glEnd();break; }
    case 0xe0: {
        uint32_t command=((DR_TPAGE*)raw)->code[0];
        if ((command>>24)==0xe1) s_currentTPage=(uint16_t)(command&0x1ffu);
        break;
    }
    default: break;
    }
}

void DrawPrim(void* p) { if (p) drawPrimitive(p); }

void DrawOTag(u_long* start)
{
    void* p = start;
    unsigned guard = 65536;
    while (p && !isendprim(p) && guard--) {
        void* next = nextPrim(p);
        if (getlen(p)) drawPrimitive(p);
        if (next == p) break;
        p = next;
    }
    glFlush(0);
}

void DrawOTagIO(u_long* p) { DrawOTag(p); }
void DrawOTagEnv(u_long* p, DRAWENV* env) { PutDrawEnv(env); DrawOTag(p); }
int DrawOTag2(u_int* p) { DrawOTag((u_long*)p); return 0; }

u_long* ClearOTagR(u_long* ot, int n)
{
    int i;
    if (!ot || n <= 0) return ot;
    termPrim(&ot[0]); setlen(&ot[0], 0);
    for (i=1; i<n; ++i) { setaddr(&ot[i], &ot[i-1]); setlen(&ot[i], 0); }
    return ot;
}

u_long* ClearOTag(u_long* ot, int n)
{
    int i;
    if (!ot || n <= 0) return ot;
    for (i=0; i<n-1; ++i) { setaddr(&ot[i], &ot[i+1]); setlen(&ot[i], 0); }
    termPrim(&ot[n-1]); setlen(&ot[n-1], 0);
    return ot;
}

int ResetGraph(int mode) { (void)mode; return 0; }
int DrawSync(int mode) { (void)mode; if (s_drawSyncCallback) s_drawSyncCallback(); return 0; }
u_int DrawSyncCallback(void (*fn)(void)) { s_drawSyncCallback=fn; return 0; }
DISPENV* PutDispEnv(DISPENV* e) { if (e) s_disp=*e; return &s_disp; }
DRAWENV* PutDrawEnv(DRAWENV* e) { if (e) s_draw=*e; return &s_draw; }
DISPENV* SetDefDispEnv(DISPENV* e,int x,int y,int w,int h) { memset(e,0,sizeof(*e)); e->disp.x=x;e->disp.y=y;e->disp.w=w;e->disp.h=h; return e; }
DRAWENV* SetDefDrawEnv(DRAWENV* e,int x,int y,int w,int h) { memset(e,0,sizeof(*e)); e->clip.x=x;e->clip.y=y;e->clip.w=w;e->clip.h=h; return e; }
int ClearImage(RECT16* r,u_char red,u_char green,u_char blue) { (void)r; glClearColor(red>>3,green>>3,blue>>3,31); return 0; }
int ClearImage2(RECT16* r,u_char red,u_char green,u_char blue) { return ClearImage(r,red,green,blue); }
int LoadImage(RECT16* r,u_long* p) { return shNdsVramLoad(r,p); }
int LoadImage2(RECT16* r,u_long* p) { return LoadImage(r,p); }
int MoveImage(RECT16* r,int x,int y) { return shNdsVramMove(r,x,y); }
int StoreImage(RECT16* r,u_long* p) { (void)r;(void)p; return 0; }
u_short GetClut(int x,int y) { return (u_short)(((y&0x1ff)<<6)|((x>>4)&0x3f)); }
u_short GetTPage(int tp,int abr,int x,int y) { return (u_short)((tp&3)|((abr&3)<<5)|((y&0x100)>>4)|((x&0x3ff)>>6)); }

void SetPolyF3(POLY_F3* p) { setPolyF3(p); }
void SetPolyF4(POLY_F4* p) { setPolyF4(p); }
void SetPolyFT3(POLY_FT3* p) { setPolyFT3(p); }
void SetPolyFT4(POLY_FT4* p) { setPolyFT4(p); }
void SetPolyG3(POLY_G3* p) { setPolyG3(p); }
void SetPolyG4(POLY_G4* p) { setPolyG4(p); }
void SetPolyGT3(POLY_GT3* p) { setPolyGT3(p); }
void SetPolyGT4(POLY_GT4* p) { setPolyGT4(p); }
void SetSprt(SPRT* p) { setSprt(p); }
void SetSprt16(SPRT_16* p) { setSprt16(p); }
void SetSprt8(SPRT_8* p) { setSprt8(p); }
void SetTile(TILE* p) { setTile(p); }
void SetTile1(TILE_1* p) { setTile1(p); }
void SetTile16(TILE_16* p) { setTile16(p); }
void SetTile8(TILE_8* p) { setTile8(p); }
void SetSemiTrans(void* p,int enabled) { setSemiTrans(p,enabled); }
void SetShadeTex(void* p,int enabled) { setShadeTex(p,enabled); }
void TermPrim(void* p) { termPrim(p); }

void SetDrawTPage(DR_TPAGE* p,int dfe,int dtd,int tpage)
{ (void)dfe;(void)dtd; setlen(p,1); p->code[0]=0xe1000000u|(tpage&0x1ffu); }
void SetDrawMode(DR_MODE* p,int dfe,int dtd,int tpage,RECT16* tw)
{ (void)tw; setlen(p,2); p->code[0]=0xe1000000u|(tpage&0x1ffu)|((dtd&1)<<9)|((dfe&1)<<10); p->code[1]=0; }
void SetDrawArea(DR_AREA* p,RECT16* r)
{ setlen(p,2); p->code[0]=0xe3000000u|((r->x&0x3ff)|((r->y&0x1ff)<<10)); p->code[1]=0xe4000000u|(((r->x+r->w-1)&0x3ff)|(((r->y+r->h-1)&0x1ff)<<10)); }
void SetDrawOffset(DR_OFFSET* p,u_short* ofs)
{ setlen(p,2); p->code[0]=0xe5000000u|(ofs[0]&0x7ff)|((ofs[1]&0x7ff)<<11); p->code[1]=0; }
void SetTexWindow(DR_TWIN* p,RECT16* tw)
{ (void)tw; setlen(p,2); p->code[0]=0xe2000000u; p->code[1]=0; }
void SetDrawStp(DR_STP* p,int pbw)
{ setlen(p,2); p->code[0]=0xe6000000u|(pbw&3); p->code[1]=0; }
int StoreImage2(RECT16* r,u_long* p) { return StoreImage(r,p); }
int MoveImage2(RECT16* r,int x,int y) { return MoveImage(r,x,y); }
