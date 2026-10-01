from pathlib import Path

src = Path('/src/src/bodyprog/bodyprog_combat_8008A058.c')
if not src.exists():
    raise SystemExit('ERRORE: bodyprog_combat_8008A058.c non trovato.')

text = src.read_text(encoding='utf-8', errors='replace')
backup = src.with_suffix('.c.before_nds_v7')
if not backup.exists():
    backup.write_text(text, encoding='utf-8')

# Include math.h on NDS too, because func_8008A058 uses the portable sqrtf path.
old_inc = '''#ifdef SH_PC_PORT
#include <math.h>
#endif'''
new_inc = '''#if defined(SH_PC_PORT) || defined(SH_NDS_PORT)
#include <math.h>
#endif'''

if new_inc not in text:
    if old_inc not in text:
        raise SystemExit('ERRORE: blocco include math.h non riconosciuto.')
    text = text.replace(old_inc, new_inc, 1)

# Only the first SH_PC_PORT after the function signature controls the MIPS-register path.
needle = '''u32 func_8008A058(s32 arg0) // 0x8008A058
{
#ifdef SH_PC_PORT'''
replacement = '''u32 func_8008A058(s32 arg0) // 0x8008A058
{
#if defined(SH_PC_PORT) || defined(SH_NDS_PORT)'''

if replacement not in text:
    if needle not in text:
        raise SystemExit('ERRORE: func_8008A058 non riconosciuta.')
    text = text.replace(needle, replacement, 1)

src.write_text(text, encoding='utf-8')

# Safety checks: the ARM build must not enter the register-binding branch.
out = src.read_text(encoding='utf-8')
if replacement not in out:
    raise SystemExit('PATCH v7 non applicata.')
if 'register s32  temp_t0 asm("t0");' not in out:
    raise SystemExit('ERRORE: sorgente inatteso; patch interrotta.')

print('NDS combat/MIPS-register v7 patch applicata')
print('  func_8008A058: NDS usa il ramo portabile sqrtf')
print('  register asm("t0"/"t1"/"t2"/"t3"): esclusi dalla build NDS')
