#!/usr/bin/env python3
import argparse, re, subprocess
from pathlib import Path

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", type=Path, required=True)
    ap.add_argument("--sources", type=Path, required=True)
    ap.add_argument("--elf", type=Path)
    ns = ap.parse_args()
    data = ns.sources.read_text(encoding="utf-8")
    checks = {
        "real MainLoop": "src/bodyprog/sys/game_main.c",
        "original FS table": "src/main/fileinfo.c",
        "original map0_s00": "src/maps/map0_s00/map0_s00_header.c",
    }
    failed = False
    for label, needle in checks.items():
        ok = needle in data
        print(("OK   " if ok else "FAIL ") + label + ": " + needle)
        failed |= not ok
    if ns.elf and ns.elf.exists():
        nm = subprocess.run(["arm-none-eabi-nm", "-u", str(ns.elf)], text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        unresolved = [x for x in nm.stdout.splitlines() if re.search(r"\bU\b", x)]
        print(f"unresolved symbols: {len(unresolved)}")
        for line in unresolved[:80]: print("  " + line)
    if failed: raise SystemExit(1)

if __name__ == "__main__":
    main()

