from pathlib import Path

src = Path('/src/src/bodyprog/events/npc_main.c')
if not src.exists():
    raise SystemExit('ERRORE: npc_main.c non trovato.')

text = src.read_text(encoding='utf-8-sig', errors='replace')
backup = src.with_suffix('.c.before_nds_v11')
if not backup.exists():
    backup.write_text(text, encoding='utf-8')

old = '''#ifdef SH_PC_PORT
static s32 Camera_Distance2dGet(const VECTOR3* pos);
extern int g_DebugAnimKfView;'''

new = '''#if defined(SH_PC_PORT) || defined(SH_NDS_PORT)
static s32 Camera_Distance2dGet(const VECTOR3* pos);
#endif
#ifdef SH_PC_PORT
extern int g_DebugAnimKfView;'''

if new not in text:
    if old not in text:
        raise SystemExit('ERRORE: blocco forward declaration Camera_Distance2dGet non riconosciuto.')
    text = text.replace(old, new, 1)

src.write_text(text, encoding='utf-8')

out = src.read_text(encoding='utf-8')
if '#if defined(SH_PC_PORT) || defined(SH_NDS_PORT)' not in out:
    raise SystemExit('ERRORE: patch v11 non applicata.')
if 'static s32 Camera_Distance2dGet(const VECTOR3* pos);' not in out:
    raise SystemExit('ERRORE: forward declaration mancante.')

print('NDS npc_main forward-declaration v11 patch applicata')
print('  Camera_Distance2dGet: dichiarata anche su SH_NDS_PORT')
print('  logica della funzione: invariata')
