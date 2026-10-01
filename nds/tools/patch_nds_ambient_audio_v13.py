from pathlib import Path

src = Path('/src/src/bodyprog/game_boot/background_sound_init.c')
if not src.exists():
    raise SystemExit('ERRORE: background_sound_init.c non trovato.')

text = src.read_text(encoding='utf-8-sig', errors='replace')
backup = src.with_suffix('.c.before_nds_v13')
if not backup.exists():
    backup.write_text(text, encoding='utf-8')

repls = {
    'g_MapOverlayHdr.ambientAudioIdx = 11;':
        '((s_MapOverlayHdr*)&g_MapOverlayHdr)->ambientAudioIdx = 11;',
    'g_MapOverlayHdr.ambientAudioIdx = 4;':
        '((s_MapOverlayHdr*)&g_MapOverlayHdr)->ambientAudioIdx = 4;'
}

for old, new in repls.items():
    if old in text:
        text = text.replace(old, new, 1)
    elif new in text:
        continue
    else:
        raise SystemExit('ERRORE: pattern ambientAudioIdx non trovato: ' + old)

src.write_text(text, encoding='utf-8')

out = src.read_text(encoding='utf-8')
for new in repls.values():
    if new not in out:
        raise SystemExit('ERRORE: patch v13 incompleta.')

print('NDS ambient-audio const v13 patch applicata')
print('  ambientAudioIdx = 11: runtime mutable cast')
print('  ambientAudioIdx = 4: runtime mutable cast')
