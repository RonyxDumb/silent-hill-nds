from pathlib import Path

candidates = [
    Path('/src/include/gpu.h'),
    Path('../include/gpu.h'),
    Path('include/gpu.h'),
]
gpu = next((p for p in candidates if p.exists()), None)
if gpu is None:
    raise SystemExit('ERRORE: include/gpu.h non trovato.')

text = gpu.read_text(encoding='utf-8', errors='replace')
backup = gpu.with_suffix('.h.before_nds_v5')
if not backup.exists():
    backup.write_text(text, encoding='utf-8')

old_ot = '''#ifdef SH_PC_PORT
/*
 * PSX OT byte offset conversion.
 * On PSX, OT entries (GsOT_TAG) are 4 bytes. Game code uses raw byte offsets
 * like (u8*)ot->org + 24 to reach entry 6. On 64-bit PC, entries are 12 bytes,
 * so byte offsets must be scaled.
 */
#define PSX_OT_OFS(n) (((n) / 4) * (int)sizeof(GsOT_TAG))
#endif'''

new_ot = '''#if defined(SH_PC_PORT) || USE_EXTENDED_PRIM_POINTERS
/*
 * OT byte offset conversion.
 * The original PSX assumes 4-byte OT entries. Extended primitive pointers
 * enlarge the tag header on NDS too, so raw PSX byte offsets must be scaled.
 */
#define PSX_OT_OFS(n) (((n) / 4) * (int)sizeof(GsOT_TAG))
#endif'''

old_code = '''#ifdef SH_PC_PORT
/* PsyCross uses 12-byte P_TAG header, so offset +4 is wrong (writes into addr).
 * Use struct field access instead. */
#define setCodeWord(p, code, rgb24) \\
    *(u32*)(&(p)->r0) = (((code) << 24) | ((rgb24) & 0xFFFFFF))
#else
#define setCodeWord(p, code, rgb24) \\
    *(u32*)(((u8*)(p)) + 4) = (((code) << 24) | ((rgb24) & 0xFFFFFF))
#endif'''

new_code = '''#if defined(SH_PC_PORT) || USE_EXTENDED_PRIM_POINTERS
/* Extended primitive headers are larger than the original PSX 4-byte tag.
 * Field access is therefore required on both PC and NDS extended-pointer builds. */
#define setCodeWord(p, code, rgb24) \\
    *(u32*)(&(p)->r0) = (((code) << 24) | ((rgb24) & 0xFFFFFF))
#else
#define setCodeWord(p, code, rgb24) \\
    *(u32*)(((u8*)(p)) + 4) = (((code) << 24) | ((rgb24) & 0xFFFFFF))
#endif'''

old_add = '''#ifdef SH_PC_PORT
#define addPrimFast(ot, p, _len) \\
    (setlen(p, _len), addPrim(ot, p))
#else
#define addPrimFast(ot, p, _len) \\
    (((p)->tag = getaddr(ot) | ((_len) << 24)), setaddr(ot, p))
#endif'''

new_add = '''#if defined(SH_PC_PORT) || USE_EXTENDED_PRIM_POINTERS
/* With extended headers POLY_FT4/POLY_* have addr+len instead of a packed tag.
 * Use the generic accessors instead of touching a non-existent .tag member. */
#define addPrimFast(ot, p, _len) \\
    (setlen(p, _len), addPrim(ot, p))
#else
#define addPrimFast(ot, p, _len) \\
    (((p)->tag = getaddr(ot) | ((_len) << 24)), setaddr(ot, p))
#endif'''

for old, new, name in (
    (old_ot, new_ot, 'PSX_OT_OFS'),
    (old_code, new_code, 'setCodeWord'),
    (old_add, new_add, 'addPrimFast'),
):
    if new in text:
        continue
    if old not in text:
        raise SystemExit(f'ERRORE: blocco {name} non riconosciuto in {gpu}; patch interrotta.')
    text = text.replace(old, new, 1)

gpu.write_text(text, encoding='utf-8')

if new_ot not in text or new_code not in text or new_add not in text:
    raise SystemExit('PATCH GPU NON COMPLETA.')

print('NDS GPU primitive-layout v5 patch applicata a', gpu)
print('  addPrimFast: extended-pointer safe')
print('  setCodeWord: extended-pointer safe')
print('  PSX_OT_OFS: extended-pointer safe')
