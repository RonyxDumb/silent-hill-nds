#!/usr/bin/env python3
import argparse
from pathlib import Path


PATCHES = {
    "world_draw": (
        "void WorldGfx_CharaModelMaterialSet(s32 charaId, s32 blendMode)",
        "void WorldGfx_CharaModelMaterialSet(e_CharaId charaId, s32 blendMode)",
    ),
}


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--unit", choices=PATCHES, required=True)
    args = parser.parse_args()

    source = args.input.read_text(encoding="utf-8")
    old, new = PATCHES[args.unit]
    count = source.count(old)
    if count != 1:
        raise SystemExit(
            f"expected exactly one {args.unit} compatibility site, found {count}"
        )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(source.replace(old, new, 1), encoding="utf-8")


if __name__ == "__main__":
    main()
