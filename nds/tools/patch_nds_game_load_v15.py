from pathlib import Path

src = Path('/src/src/bodyprog/game_boot/game_load.c')
if not src.exists():
    raise SystemExit('ERRORE: game_load.c non trovato.')

text = src.read_text(encoding='utf-8-sig', errors='replace')
backup = src.with_suffix('.c.before_nds_v15')
if not backup.exists():
    backup.write_text(text, encoding='utf-8')

old = '''#ifdef SH_PC_PORT
static void GameBoot_LoadingScreen(void);
static bool Pc_LoadScreenHoldingMinimum(void);
static void Pc_LoadScreenReset(void);
#endif'''

new = '''#if defined(SH_PC_PORT) || defined(SH_NDS_PORT)
static void GameBoot_LoadingScreen(void);
#endif
#ifdef SH_PC_PORT
static bool Pc_LoadScreenHoldingMinimum(void);
static void Pc_LoadScreenReset(void);
#endif'''

if new not in text:
    if old not in text:
        raise SystemExit('ERRORE: blocco forward declarations game_load.c non riconosciuto.')
    text = text.replace(old, new, 1)

src.write_text(text, encoding='utf-8')

out = src.read_text(encoding='utf-8')
if new not in out:
    raise SystemExit('ERRORE: patch v15 non applicata.')

print('NDS game_load guard v15 patch applicata')
print('  GameBoot_LoadingScreen: SH_PC_PORT || SH_NDS_PORT')
print('  helper Pc_*: solo SH_PC_PORT')
