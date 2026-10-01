from pathlib import Path

src = Path('/src/src/bodyprog/game_boot/game_load.c')
if not src.exists():
    raise SystemExit('ERRORE: game_load.c non trovato.')

text = src.read_text(encoding='utf-8-sig', errors='replace')
backup = src.with_suffix('.c.before_nds_v14')
if not backup.exists():
    backup.write_text(text, encoding='utf-8')

decl = 'static void GameBoot_LoadingScreen(void);'

if decl not in text:
    # Insert after includes, before first function use.
    marker = '#include "bodyprog/text/text_draw.h"'
    if marker not in text:
        raise SystemExit('ERRORE: punto di inserimento non trovato in game_load.c')
    text = text.replace(marker, marker + '\n\n' + decl, 1)

src.write_text(text, encoding='utf-8')

out = src.read_text(encoding='utf-8')
if decl not in out:
    raise SystemExit('ERRORE: forward declaration GameBoot_LoadingScreen mancante.')

print('NDS game_load forward-declaration v14 patch applicata')
print('  GameBoot_LoadingScreen: dichiarata prima del primo uso')
print('  logica di caricamento: invariata')
