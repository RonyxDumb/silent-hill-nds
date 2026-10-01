from pathlib import Path

src = Path('/src/src/bodyprog/events/game_sys_states.c')
if not src.exists():
    raise SystemExit('ERRORE: game_sys_states.c non trovato.')

text = src.read_text(encoding='utf-8', errors='replace')
backup = src.with_suffix('.c.before_nds_v9')
if not backup.exists():
    backup.write_text(text, encoding='utf-8')

repls = {
    'static void (*g_SysStateFuncs[])(void) = {':
        'void (*g_SysStateFuncs[])(void) = {',
    'static s32 g_DeltaTimeCpy;':
        's32 g_DeltaTimeCpy;'
}

for old, new in repls.items():
    # IMPORTANT: test the old declaration first.  The replacement string is
    # a substring of the old one ("static s32 X" contains "s32 X"), so checking
    # `new in text` first produces a false "already patched" result.
    if old in text:
        text = text.replace(old, new, 1)
    elif new in text:
        continue
    else:
        raise SystemExit(f'ERRORE: pattern non trovato: {old}')

src.write_text(text, encoding='utf-8')

out = src.read_text(encoding='utf-8')
for old in repls:
    if old in out:
        raise SystemExit('ERRORE: linkage v9 non applicato completamente.')

print('NDS game_sys_states linkage v9 patch applicata')
print('  g_SysStateFuncs: external linkage')
print('  g_DeltaTimeCpy: external linkage')
