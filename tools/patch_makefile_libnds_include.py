from pathlib import Path

# Run this script from the nds directory.
makefile = Path("Makefile")
if not makefile.exists():
    raise SystemExit("ERRORE: Makefile non trovato. Esegui questo script dentro la cartella nds.")

text = makefile.read_text(encoding="utf-8", errors="replace")

needle = "-I$(DEVKITPRO)/libnds/include"
if needle in text:
    print("Il path libnds è già presente nel Makefile.")
    raise SystemExit(0)

# The compile commands already use the project's INCLUDES variable.
# Appending here keeps the fix shared by C and C++ sources, including platform/*.c.
marker = "# NDS system headers"
block = """
# NDS system headers
INCLUDES += -I$(DEVKITPRO)/libnds/include
INCLUDES += -I$(DEVKITPRO)/calico/include
"""

# Insert before the first build rule when possible; otherwise append safely.
candidates = [
    "\nall:",
    "\n.PHONY:",
    "\n$(BUILD)",
]

inserted = False
for c in candidates:
    pos = text.find(c)
    if pos != -1:
        text = text[:pos] + "\n" + block + text[pos:]
        inserted = True
        break

if not inserted:
    text += "\n" + block

makefile.write_text(text, encoding="utf-8")
print("Makefile aggiornato.")
print("Aggiunti:")
print("  -I$(DEVKITPRO)/libnds/include")
print("  -I$(DEVKITPRO)/calico/include")
