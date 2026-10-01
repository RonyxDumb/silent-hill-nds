from pathlib import Path

src = Path('/src/src/bodyprog/events/title.c')
if not src.exists():
    raise SystemExit('ERRORE: title.c non trovato.')

text = src.read_text(encoding='utf-8-sig', errors='replace')
backup = src.with_suffix('.c.before_nds_v12')
if not backup.exists():
    backup.write_text(text, encoding='utf-8')

# 1) Forward declarations: they already exist upstream but are PC-only.
old_block = '''#ifdef SH_PC_PORT
/* Forward declarations for static functions used before definition */
static void MainMenu_MainTextDraw(void);
static void MainMenu_AchievementHintDraw(void);
static void MainMenu_DifficultyTextDraw(s32 idx);
static void MainMenu_BackgroundDraw(void);
static void func_8003BCF4(void);
#endif'''

new_block = '''#if defined(SH_PC_PORT) || defined(SH_NDS_PORT)
/* Forward declarations for static functions used before definition.
 * GCC/ARM also needs these to avoid implicit extern declarations. */
static void MainMenu_MainTextDraw(void);
#ifdef SH_PC_PORT
static void MainMenu_AchievementHintDraw(void);
#endif
static void MainMenu_DifficultyTextDraw(s32 idx);
static void MainMenu_BackgroundDraw(void);
static void func_8003BCF4(void);
#endif'''

if new_block not in text:
    if old_block not in text:
        raise SystemExit('ERRORE: blocco forward declaration title.c non riconosciuto.')
    text = text.replace(old_block, new_block, 1)

# 2) GameBoot_MapLoad takes a map index.  MapOverlayId_MAP0_S00 is not
# available in the NDS configuration; MapIdx_MAP0_S00 is the canonical enum.
old_map = 'GameBoot_MapLoad(MapOverlayId_MAP0_S00);'
new_map = 'GameBoot_MapLoad(MapIdx_MAP0_S00);'
if new_map not in text:
    if old_map not in text:
        raise SystemExit('ERRORE: chiamata GameBoot_MapLoad MAP0_S00 non trovata.')
    text = text.replace(old_map, new_map, 1)

src.write_text(text, encoding='utf-8')

out = src.read_text(encoding='utf-8')
for marker in [
    'static void MainMenu_MainTextDraw(void);',
    'static void MainMenu_DifficultyTextDraw(s32 idx);',
    'static void MainMenu_BackgroundDraw(void);',
    'static void func_8003BCF4(void);',
    'GameBoot_MapLoad(MapIdx_MAP0_S00);'
]:
    if marker not in out:
        raise SystemExit('ERRORE: patch v12 incompleta: ' + marker)

print('NDS title/menu v12 patch applicata')
print('  static menu forward declarations: abilitate su NDS')
print('  GameBoot_MapLoad: MapIdx_MAP0_S00')
