# Silent Hill DS — native integration target

This directory is a build target for the **original decompiled game sources**.
It is not a second `src_nds` game and it does not contain replacement gameplay.

The target compiles:

- `src/main` (except the PSX boot entry and MIPS-only memcpy)
- all of `src/bodyprog`
- the normal screen overlays
- the original `src/maps/map0_s00` overlay
- only the hardware-facing adapters in `nds/platform`

## Install

Copy this `nds` directory into the repository root, next to `src`, `include`,
`pc_port` and `Makefile`.

Place extracted game files under `nds/nitrofs` while retaining their original
directories (`1ST`, `ANIM`, `BG`, `CHARA`, `ITEM`, `MISC`, `SND`, `TIM`, `VIN`,
`XA`). No copyrighted game data is included.

Run:

```bat
nds\docker_build.bat
```

or, in a devkitPro shell:

```sh
make -C nds configure
make -C nds -j4
```

`configure` generates `build/sources.mk` directly from the real source tree and
fails if `MainLoop`, `map0_s00` or the original file table are absent.

## Current boundary

This package deliberately makes the architectural switch first: the linked
program is now the real decompiled engine and starting-map code. The backend is
an integration layer, not a demo. It provides the DS entry, a bounded PSX RAM
arena, NitroFS identity/path resolution, controller polling and frame timing.

The ARM9 target now compiles PsyCross's software GTE core (`doCOP2`, register
loads/stores, matrix operations, RTPS/RTPT, lighting and clipping) and supplies
a DS GPU packet/ordering-table backend for F3/F4/G3/G4 and FT/GT packets. PGXP
is disabled because its desktop shadow tables and doubles are unsuitable for
DS RAM. A 256 KiB sparse PS1-VRAM mirror now backs `LoadImage`/`MoveImage`;
4/8/16-bpp pages and CLUTs are decoded into a bounded two-entry DS texture LRU.
FT3/FT4/GT3/GT4 and sprites bind textures using their original `tpage + clut`.
The bounded cache intentionally trades conversions for a predictable RAM/VRAM
ceiling on original 4 MiB hardware.

`make audit` reports the remaining unresolved PsyQ surface instead of hiding it
behind gameplay stubs. A successful final link is the next milestone, followed
by the first real frame from `MainLoop()`.
