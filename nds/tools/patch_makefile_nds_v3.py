from pathlib import Path
import re
import sys

candidates = [Path('/src/nds/Makefile'), Path('Makefile'), Path('nds/Makefile')]
mf = next((p for p in candidates if p.exists()), None)
if mf is None:
    raise SystemExit('ERRORE: Makefile NDS non trovato.')

text = mf.read_text(encoding='utf-8', errors='replace')
backup = mf.with_suffix('.before_nds_v3')
if not backup.exists():
    backup.write_text(text, encoding='utf-8')

# 1) NDS is not the desktop PC port.  Keeping SH_PC_PORT enabled pulls in
# widescreen/SDL/PGXP/randomizer/plugin/debug code and its large globals.
text = re.sub(r'(?<![A-Za-z0-9_])-DSH_PC_PORT(?:=1)?\b', '', text)

# 2) Ensure the native NDS define remains present.
if '-DSH_NDS_PORT' not in text:
    m = re.search(r'(?m)^(CFLAGS\s*[:+?]?=\s*.*)$', text)
    if m:
        text = text[:m.end()] + ' -DSH_NDS_PORT' + text[m.end():]
    else:
        text += '\nCFLAGS += -DSH_NDS_PORT\n'

# 3) Dead-section elimination is mandatory on 4 MiB NDS EWRAM.  The previous
# build put every global/function section from every directly-linked object in
# RAM and ended .bss at 0x025D8648.
def ensure_append(var, flags):
    global text
    line = f'{var} += {flags}'
    if line not in text:
        text += '\n' + line + '\n'

ensure_append('CFLAGS', '-ffunction-sections -fdata-sections -fno-short-enums')
ensure_append('CXXFLAGS', '-ffunction-sections -fdata-sections -fno-short-enums')
ensure_append('LDFLAGS', '-Wl,--gc-sections')

# 4) This project owns a custom final ELF rule, so ds_rules does not rewrite
# ds_arm9.specs for us. Force Calico's current ARM9 specs everywhere.
text = text.replace('-specs=ds_arm9.specs', '-specs=$(DEVKITPRO)/calico/share/ds9.specs')
if '-specs=$(DEVKITPRO)/calico/share/ds9.specs' not in text:
    text += '\nLDFLAGS += -specs=$(DEVKITPRO)/calico/share/ds9.specs\n'

# 5) Ensure search paths used by the custom final link rule.
for flag in (
    '-L$(DEVKITPRO)/libnds/lib',
    '-L$(DEVKITPRO)/portlibs/nds/lib',
    '-L$(DEVKITPRO)/calico/lib',
):
    if flag not in text:
        text += f'\nLDFLAGS += {flag}\n'

# 6) Correct static library order and make it circular-dependency-safe.
# IMPORTANT: patch both LIBS assignments and hard-coded library tails in custom
# recipes, because the previous batch file only edited LIBS while the actual
# recipe still emitted the old sequence.
libs = '-Wl,--start-group -lmm9 -lfilesystem -lfat -lnds9 -lcalico_ds9 -Wl,--end-group -lm'
text = re.sub(r'(?m)^LIBS\s*[:+?]?=\s*.*$', 'LIBS := ' + libs, text)
if not re.search(r'(?m)^LIBS\s*[:+?]?=', text):
    text += '\nLIBS := ' + libs + '\n'

# Replace known old library sequences anywhere in recipes.
patterns = [
    r'-lfat\s+-lfilesystem\s+-lnds9\s+-lmm9\s+-lm',
    r'-lfilesystem\s+-lfat\s+-lnds9\s+-lmm9\s+-lm',
    r'-lmm9\s+-lfilesystem\s+-lfat\s+-lnds9\s+-lm',
    r'-lfat\s+-lfilesystem\s+-lnds9\s+-lmm9(?:\s+-lcalico_ds9)?\s+-lm',
]
for pat in patterns:
    text = re.sub(pat, libs, text)

# 7) Force one coherent Linux toolchain inside Docker/WSL.
tool_block = '''\n# NDS v3: never pick Windows devkitPro binaries through an imported PATH.\noverride CC      := $(DEVKITARM)/bin/arm-none-eabi-gcc\noverride CXX     := $(DEVKITARM)/bin/arm-none-eabi-g++\noverride AR      := $(DEVKITARM)/bin/arm-none-eabi-gcc-ar\noverride AS      := $(DEVKITARM)/bin/arm-none-eabi-as\noverride OBJCOPY := $(DEVKITARM)/bin/arm-none-eabi-objcopy\n'''
if 'override CC      := $(DEVKITARM)/bin/arm-none-eabi-gcc' not in text:
    text += tool_block

# Normalise accidental repeated whitespace left after removing SH_PC_PORT.
text = re.sub(r'[ \t]+\n', '\n', text)
mf.write_text(text, encoding='utf-8')

# Strong verification: stop before wasting a full compile if the linker fix did
# not really reach the Makefile.
problems = []
if '-DSH_PC_PORT' in text:
    problems.append('SH_PC_PORT ancora presente')
if '-specs=ds_arm9.specs' in text:
    problems.append('vecchio ds_arm9.specs ancora presente')
if '-lcalico_ds9' not in text:
    problems.append('calico_ds9 mancante')
if '--gc-sections' not in text:
    problems.append('--gc-sections mancante')
if '-fno-short-enums' not in text:
    problems.append('-fno-short-enums mancante')
if problems:
    raise SystemExit('PATCH NON COMPLETA: ' + '; '.join(problems))

print('NDS Makefile v3 patch applicata a', mf)
print('  SH_PC_PORT: OFF')
print('  SH_NDS_PORT: ON')
print('  dead section GC: ON')
print('  enum ABI: 32-bit (-fno-short-enums)')
print('  specs: Calico ds9')
print('  libs:', libs)
