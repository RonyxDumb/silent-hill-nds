from pathlib import Path
import re

mf = Path("Makefile")
if not mf.exists():
    raise SystemExit("ERRORE: esegui questo script dalla cartella nds (Makefile non trovato).")

text = mf.read_text(encoding="utf-8", errors="replace")

# 0) Never resolve the cross tools from PATH. WSL commonly imports the Windows
#    PATH, which makes a Linux build compile with C:/devkitPro/*.exe while all
#    headers and libraries come from /opt/devkitpro. That hybrid toolchain is
#    exactly what produces "cannot find -lnds9" despite libnds being installed.
tool_block = """
# NDS toolchain: use the binaries belonging to this DEVKITARM installation.
override CC      := $(DEVKITARM)/bin/arm-none-eabi-gcc
override CXX     := $(DEVKITARM)/bin/arm-none-eabi-g++
override AR      := $(DEVKITARM)/bin/arm-none-eabi-gcc-ar
override AS      := $(DEVKITARM)/bin/arm-none-eabi-as
override OBJCOPY := $(DEVKITARM)/bin/arm-none-eabi-objcopy
"""
if "override CC      := $(DEVKITARM)/bin/arm-none-eabi-gcc" not in text:
    text += "\n" + tool_block

# 1) Use Calico's DS9 specs explicitly. ds_rules normally performs this
#    substitution inside its generic %.elf rule, but this port owns a custom
#    silent_hill.elf recipe and therefore bypasses that transformation.
calico_specs = "-specs=$(DEVKITPRO)/calico/share/ds9.specs"
text = text.replace("-specs=ds_arm9.specs", calico_specs)
if calico_specs not in text:
    text += f"\nLDFLAGS += {calico_specs}\n"

# 2) Ensure all NDS library roots exist in variables used by custom makefiles.
#    Official templates derive -L<root>/lib from LIBDIRS.
if "$(DEVKITPRO)/libnds" not in text and "$(LIBNDS)" not in text:
    text += "\nLIBDIRS += $(DEVKITPRO)/libnds\n"
if "$(DEVKITPRO)/portlibs/nds" not in text and "$(PORTLIBS)" not in text:
    text += "LIBDIRS += $(DEVKITPRO)/portlibs/nds\n"
if "$(DEVKITPRO)/calico" not in text:
    text += "LIBDIRS += $(DEVKITPRO)/calico\n"

# 3) Custom linker rules may not consume LIBDIRS at all. Add explicit -L paths
#    to LDFLAGS as a fallback. Duplicates are harmless.
for flag in [
    "-L$(DEVKITPRO)/libnds/lib",
    "-L$(DEVKITPRO)/portlibs/nds/lib",
    "-L$(DEVKITPRO)/calico/lib",
]:
    # Do not treat a mention in LIBDIRS/comments as proof that the custom final
    # link rule consumes it. An explicit LDFLAGS line is deterministic.
    assignment = f"LDFLAGS += {flag}"
    if assignment not in text:
        text += assignment + "\n"

# 4) Keep required library order. The custom ELF rule also bypasses ds_rules'
#    automatic _EXTRALIBS, so Calico must be last among the DS libraries.
m = re.search(r'(?m)^LIBS\s*[:+?]?=\s*(.*)$', text)
if m:
    known = {"-lfilesystem", "-lfat", "-lmm9", "-lnds9", "-lcalico_ds9", "-lm"}
    other = [token for token in m.group(1).split() if token not in known]
    ordered = other + ["-lmm9", "-lfilesystem", "-lfat", "-lnds9", "-lcalico_ds9", "-lm"]
    text = text[:m.start()] + "LIBS := " + " ".join(ordered) + text[m.end():]
else:
    text += "\nLIBS := -lmm9 -lfilesystem -lfat -lnds9 -lcalico_ds9 -lm\n"

mf.write_text(text, encoding="utf-8")

print("Makefile linker patch applicata.")
print("Verifica attesa nella riga finale:")
print("  -specs=$(DEVKITPRO)/calico/share/ds9.specs")
print("  -L.../libnds/lib")
print("  -L.../portlibs/nds/lib")
print("  -L.../calico/lib")
print("  -lmm9 -lfilesystem -lfat -lnds9 -lcalico_ds9 -lm")
print("  linker sotto $(DEVKITARM)/bin (mai C:/devkitPro durante una build WSL)")
