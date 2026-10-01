from pathlib import Path

root = Path('/src')

gpu = root / 'include' / 'gpu.h'
gte = root / 'pc_port' / 'include' / 'gpu_gte_pc.h'

if not gpu.exists():
    raise SystemExit('ERRORE: /src/include/gpu.h non trovato')
if not gte.exists():
    raise SystemExit('ERRORE: /src/pc_port/include/gpu_gte_pc.h non trovato')

# ---- gpu.h: choose the portable PsyCross GTE macros on NDS too ----
t = gpu.read_text(encoding='utf-8', errors='replace')

old = '''#ifdef SH_PC_PORT
/* PC port: use PsyCross C-based GTE register access instead of MIPS asm */
#include "gpu_gte_pc.h"
#else'''
new = '''#if defined(SH_PC_PORT) || defined(SH_NDS_PORT)
/* PC/NDS: use PsyCross C-based GTE register access instead of MIPS PS1 asm.
 * ARM9 cannot assemble the original MIPS cop2 instructions. */
#include "gpu_gte_pc.h"
#else'''

if new not in t:
    if old not in t:
        raise SystemExit('ERRORE: blocco GTE di include/gpu.h non riconosciuto')
    t = t.replace(old, new, 1)

t = t.replace('#endif /* !SH_PC_PORT */', '#endif /* !SH_PC_PORT && !SH_NDS_PORT */', 1)
gpu.write_text(t, encoding='utf-8')

# ---- gpu_gte_pc.h: make its portable macro implementations available on NDS ----
t = gte.read_text(encoding='utf-8', errors='replace')

old_guard = '#ifdef SH_PC_PORT'
new_guard = '#if defined(SH_PC_PORT) || defined(SH_NDS_PORT)'
if new_guard not in t:
    if old_guard not in t:
        raise SystemExit('ERRORE: guard SH_PC_PORT in gpu_gte_pc.h non trovato')
    t = t.replace(old_guard, new_guard, 1)

# PGXP/per-pixel-flashlight tracking is desktop-only.  Do not create NDS
# references to g_PsxUsePgxp/g_PsyX_UsePerPixelFlashlight/PGXP_StoreAddr.
old_decls = '''/* PGXP shadow store: gte_FetchScreen0_1_2_XYZ records the destination address ->
 * precise GTE projection (like gte_stsxy3c), so the lit-character drawer's
 * screenXy_0 entries are shadow-backed and propagate to prims. */
#ifdef __cplusplus
extern "C" {
#endif
extern int  g_PsxUsePgxp;
extern int  g_PsyX_UsePerPixelFlashlight;
extern void PGXP_StoreAddr(void* addr, int slot);
#ifdef __cplusplus
}
#endif'''

new_decls = '''/* PGXP shadow tracking is meaningful only for the desktop renderer.
 * NDS still uses the same portable GTE register helpers but does not import
 * desktop-only PGXP/flashlight globals. */
#ifdef SH_PC_PORT
#ifdef __cplusplus
extern "C" {
#endif
extern int  g_PsxUsePgxp;
extern int  g_PsyX_UsePerPixelFlashlight;
extern void PGXP_StoreAddr(void* addr, int slot);
#ifdef __cplusplus
}
#endif
#define SH_GTE_STORE_SCREEN_SHADOWS(xy) do { \
    if (g_PsxUsePgxp || g_PsyX_UsePerPixelFlashlight) { \
        PGXP_StoreAddr(&(xy)[0], 0); \
        PGXP_StoreAddr(&(xy)[1], 1); \
        PGXP_StoreAddr(&(xy)[2], 2); \
    } \
} while (0)
#else
#define SH_GTE_STORE_SCREEN_SHADOWS(xy) ((void)0)
#endif'''

if new_decls not in t:
    if old_decls not in t:
        raise SystemExit('ERRORE: blocco PGXP in gpu_gte_pc.h non riconosciuto')
    t = t.replace(old_decls, new_decls, 1)

old_shadow = '''    if (g_PsxUsePgxp || g_PsyX_UsePerPixelFlashlight) { PGXP_StoreAddr(&_xy[0], 0); PGXP_StoreAddr(&_xy[1], 1); PGXP_StoreAddr(&_xy[2], 2); } \\
} while(0)'''
new_shadow = '''    SH_GTE_STORE_SCREEN_SHADOWS(_xy); \\
} while(0)'''

if new_shadow not in t:
    if old_shadow not in t:
        raise SystemExit('ERRORE: macro gte_FetchScreen0_1_2_XYZ non riconosciuto')
    t = t.replace(old_shadow, new_shadow, 1)

t = t.replace('#endif /* SH_PC_PORT */', '#endif /* SH_PC_PORT || SH_NDS_PORT */', 1)
gte.write_text(t, encoding='utf-8')

# ---- world renderer: NDS equivalents of desktop-only draw-distance helpers ----
world_draw = root / 'src' / 'bodyprog' / 'gfx' / 'bodyprog_80055028.c'
if not world_draw.exists():
    raise SystemExit('ERRORE: world renderer non trovato')

t = world_draw.read_text(encoding='utf-8', errors='replace')

# Close the outer desktop renderer guard if a prior interrupted reconstruction
# removed its matching directive; the canonical replacement below only owns the
# nested fog/draw-distance section.
outer_guard = '#ifdef SH_PC_PORT\n#include <stdio.h>'
if outer_guard in t:
    outer_start = t.find(outer_guard)
    per_poly = t.find('/* Per-poly far drop.', outer_start)
    if per_poly < 0:
        raise SystemExit('ERRORE: sezione renderer non trovata')
    # A nested FOG_FAR_DIST guard is not the matching close for the includes.
    # Normalise the gap so the outer desktop-only include block is closed before
    # the portable macro section on every invocation.
    gap = t[outer_start:per_poly]
    if '#endif\n\n' not in gap:
        t = t[:per_poly] + '#endif\n\n' + t[per_poly:]

# The host checkout may contain an interrupted prior write with literal "\\n"
# sequences inside the macro section. Reconstruct the complete canonical block
# before compilation so NDS never turns these helpers into implicit functions.
start = t.find('/* Per-poly far drop.')
end = t.find('\n// ========================================\n// ENVIRONMENT AND SCREEN GFX 1', start)
if start < 0 or end < 0:
    raise SystemExit('ERRORE: blocco macro renderer non riconosciuto')
renderer_block = r'''/* Per-poly far drop. The base 0x79C shifted by (shift+2) is ~61u, and it -- not
 * chunk residency, not the cull predicate -- is what actually ends the view:
 * with preload_chunks the whole grid is already in memory. draw_distance_pct
 * scales it, so 200 reaches ~122u. Do not exceed ~210, where view-space Z (Q8
 * in an s16 scratch) wraps past 128u. Past ~64u everything shares the last OT
 * bucket and sorts by submission order, so distant blocks can trade places --
 * acceptable for seeing further with fog turned down, and why this is opt-in. */
#ifdef SH_PC_PORT
#define SH_FAR_BASE(b)                                                          \
    ((s32)((b) * ((g_PcConfig.drawDistancePct <= 0) ? 100 : g_PcConfig.drawDistancePct) / 100))
#else
#define SH_FAR_BASE(b) (b)
#endif

/* On PC, zero the fog depth parameter for dpcl/dpcs vertex color
 * computation. Shader fog via p1/p2/p3 handles all distance fog.
 * This prevents double-fogging and seam lines at face boundaries. */
#define VTXCOL_LDDP(dp) gte_lddp(0)

/* Clamp a polygon's OT depth so the bucket index (depth>>shift>>2) can't overrun
 * org[ORDERING_TABLE_SIZE]; the emulated GTE can return an SZ outside the range
 * real hardware's OTZ register saturates to, and an out-of-range addPrim writes
 * the packet pointer into adjacent memory -> split-pointer cutscene crashes. */
#if defined(SH_PC_PORT)
#define SH_CLAMP_OT_DEPTH(depth, shift)                                         \
    do {                                                                        \
        if ((((depth) >> (shift)) >> 2) >= ORDERING_TABLE_SIZE)                 \
            (depth) = (ORDERING_TABLE_SIZE - 1) << ((shift) + 2);               \
    } while (0)
#define SH_WHOLEMAP_FARCAP(cap) ((void)(cap))
#define SH_WHOLEMAP_DEPTH_RESCUE(depth, shift) ((void)(depth))
#define SH_WHOLEMAP_FAR_POLY(depth, shift) (0)
#elif defined(SH_NDS_PORT)
#define SH_CLAMP_OT_DEPTH(depth, shift)                                         \
    do {                                                                        \
        if ((((depth) >> (shift)) >> 2) >= ORDERING_TABLE_SIZE)                 \
            (depth) = (ORDERING_TABLE_SIZE - 1) << ((shift) + 2);               \
    } while (0)
#define SH_WHOLEMAP_FARCAP(cap) ((void)(cap))
#define SH_WHOLEMAP_DEPTH_RESCUE(depth, shift) ((void)(depth))
#define SH_WHOLEMAP_FAR_POLY(depth, shift) (0)
#else
#define SH_CLAMP_OT_DEPTH(depth, shift) ((void)(depth))
#define SH_WHOLEMAP_FARCAP(cap) ((void)(cap))
#define SH_WHOLEMAP_DEPTH_RESCUE(depth, shift) ((void)(depth))
#define SH_WHOLEMAP_FAR_POLY(depth, shift) (0)
#endif

/* Desktop-only fog/cull helpers collapse to native PSX behavior on NDS. */
#ifdef SH_PC_PORT
#define PC_FACE_FOG_VERTS(sd) ((void)0)
#define PC_SCREEN_Z_TO_FOG(z) (0)
#define PC_FACE_CULL_DEPTH(sd, maxz) (maxz)
#define PC_OBJ_CULL_DEPTH(sd, avgz, quad) (avgz)
#else
#define PC_FACE_FOG_VERTS(sd) ((void)0)
#define PC_SCREEN_Z_TO_FOG(z) (0)
#define PC_FACE_CULL_DEPTH(sd, maxz) (maxz)
#define PC_OBJ_CULL_DEPTH(sd, avgz, quad) (avgz)
#endif
'''
t = t[:start] + renderer_block + t[end:]
world_draw.write_text(t, encoding='utf-8')

# Final sanity checks.
gpu_t = gpu.read_text(encoding='utf-8')
gte_t = gte.read_text(encoding='utf-8')
assert '#if defined(SH_PC_PORT) || defined(SH_NDS_PORT)' in gpu_t
assert '#if defined(SH_PC_PORT) || defined(SH_NDS_PORT)' in gte_t
assert 'SH_GTE_STORE_SCREEN_SHADOWS(_xy)' in gte_t
assert '#elif defined(SH_NDS_PORT)' in t
assert '#define SH_FAR_BASE(b) (b)' in t
assert '\\n' not in t[start:start + len(renderer_block)]
assert '#endif\n\n/* Per-poly far drop.' in t

print('NDS portable-GTE v6 patch applicata')
print('  gpu.h: SH_NDS_PORT usa gpu_gte_pc.h')
print('  gpu_gte_pc.h: macro C abilitate su NDS')
print('  PGXP desktop globals: esclusi su NDS')
