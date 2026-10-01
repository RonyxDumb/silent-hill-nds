#!/usr/bin/env python3
import argparse
from pathlib import Path

EXCLUDED_NAMES = {"main.c", "memcpy.c", "bodyprog_80032D1C.c", "text_draw_jp.c"}
NDS_ADAPTER_SOURCES = {
    "pc_port/src/stubs/data_stubs.c",
    "pc_port/src/stubs/func_stubs.c",
    "pc_port/src/math_impl.c",
    "pc_port/src/cheryl_anim_info.c",
    "pc_port/src/d_800294f4_data.c",
}
EXCLUDED_PARTS = {"hp_safe1", "s__safe2"}

def collect(root: Path):
    files = []
    for base in (root / "src/main", root / "src/bodyprog", root / "src/screens"):
        for path in base.rglob("*.c"):
            if path.name in EXCLUDED_NAMES or any(part in EXCLUDED_PARTS for part in path.parts):
                continue
            files.append(path)
    files += sorted((root / "src/maps/map0_s00").glob("*.c"))
    # Decode/MDEC streaming is a PSX FMV subsystem. The DS build starts in the
    # original map and does not link a software decoder into constrained ARM9 RAM.
    files = [path for path in files if path != root / "src/screens/stream/stream.c"]
    return sorted(set(files))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    root = args.root.resolve()
    required = [
        root / "src/bodyprog/sys/game_main.c",
        root / "src/maps/map0_s00/map0_s00_header.c",
        root / "src/main/fileinfo.c",
        root / "include/main/fileenum.h.USA.inc",
    ]
    missing = [str(path.relative_to(root)) for path in required if not path.exists()]
    if missing:
        raise SystemExit("missing required original sources: " + ", ".join(missing))
    game = collect(root)
    if not game:
        raise SystemExit("no original game sources found")
    game_rel = ["../" + path.relative_to(root).as_posix() for path in game]
    game_rel += ["../" + path for path in sorted(NDS_ADAPTER_SOURCES)]
    nds_dir = Path(__file__).resolve().parent.parent
    nds_rel = sorted(path.relative_to(nds_dir).as_posix()
                     for path in (nds_dir / "platform").glob("*.c"))
    print("NDS platform sources:", " ".join(nds_rel))
    separator = " " + "\\" + "\n  "
    output = "# Generated. Every GAME_SOURCES entry belongs to the original decomp.\n"
    output += "GAME_SOURCES := " + "\\" + "\n  " + separator.join(game_rel) + "\n"
    output += "NDS_SOURCES := " + " ".join(nds_rel) + "\n"
    output += "GTE_C_SOURCES := ../pc_port/PsyCross/src/psx/inline_c.c ../pc_port/PsyCross/src/psx/libgte.c\n"
    output += "GTE_CXX_SOURCES := ../pc_port/PsyCross/src/gte/PsyX_GTE.cpp\n"
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(output, encoding="utf-8")
    print(f"configured {len(game)} original game translation units + {len(nds_rel)} DS adapters")

if __name__ == "__main__":
    main()
