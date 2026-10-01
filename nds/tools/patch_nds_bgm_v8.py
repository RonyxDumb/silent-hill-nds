from pathlib import Path

src = Path('/src/src/bodyprog/events/bgm.c')
if not src.exists():
    raise SystemExit('ERRORE: /src/src/bodyprog/events/bgm.c non trovato.')

text = src.read_text(encoding='utf-8', errors='replace')
backup = src.with_suffix('.c.before_nds_v8')
if not backup.exists():
    backup.write_text(text, encoding='utf-8')

# These are declared extern in bodyprog.h, so the definitions must have
# external linkage as well.
text = text.replace('static s32 D_800BCD5C;', 's32 D_800BCD5C;', 1)
text = text.replace('static s32 D_800A99A0 = 0;', 's32 D_800A99A0 = 0;', 1)

# The decomp exposes the current map header as const, but the original game
# changes bgmIdx at runtime.  On NDS keep the public declaration untouched and
# cast only at the mutation site.
old = '        g_MapOverlayHdr.bgmIdx = bgmIdx;'
new = '        ((s_MapOverlayHdr*)&g_MapOverlayHdr)->bgmIdx = bgmIdx;'
if new not in text:
    if old not in text:
        raise SystemExit('ERRORE: assegnazione g_MapOverlayHdr.bgmIdx non trovata.')
    text = text.replace(old, new, 1)

src.write_text(text, encoding='utf-8')

out = src.read_text(encoding='utf-8')
if 'static s32 D_800BCD5C;' in out or 'static s32 D_800A99A0 = 0;' in out:
    raise SystemExit('ERRORE: linkage BGM non corretto.')
if new not in out:
    raise SystemExit('ERRORE: write bgmIdx non corretto.')

print('NDS BGM linkage/const v8 patch applicata')
print('  D_800BCD5C: external linkage')
print('  D_800A99A0: external linkage')
print('  g_MapOverlayHdr.bgmIdx: runtime mutable cast')
