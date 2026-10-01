# Silent Hill PS1 → Nintendo DS — FCC Cloud Instructions

## Main Goal

The repository already contains an advanced Nintendo DS port of the Silent Hill PS1 decomp and already partially compiles for NDS.

The goal is to **get the real Silent Hill game running on Nintendo DS**, not create another test/demo.

Continue the existing port and implement/fix what is still required:

- PS1/PsyQ → Nintendo DS backend
- GPU renderer and PS1 primitives
- GTE transforms/projection
- ordering tables
- textures/VRAM
- filesystem and original file IDs
- overlays/maps
- audio
- input
- memory handling
- real game loop

## Work Strategy

My cloud plan is very limited. Do NOT waste time with large repository audits or unnecessary analysis.

1. Immediately run the existing `docker_build.bat`.
2. Read the **first real compiler/linker error**.
3. Fix it directly.
4. Rebuild immediately.
5. Repeat until the `.nds` builds.
6. Once it builds, continue directly toward the real Silent Hill game loop and first playable scene.

Use the build errors as the main guide.

Do NOT inspect unrelated files unless required by the current error.

## Build

Do NOT open `docker_build.bat` through the Windows shell or file associations.

Execute it directly using the real Windows command interpreter:

```bat
C:\Windows\System32\cmd.exe /d /c "cd /d <PROJECT_ROOT> && docker_build.bat"
```

Replace `<PROJECT_ROOT>` with the actual repository path.

If the batch file is inside the `nds` directory:

```bat
C:\Windows\System32\cmd.exe /d /c "cd /d <PROJECT_ROOT>\nds && docker_build.bat"
```

Never use `start cmd`, `open cmd`, or Windows "Open with". Invoke `C:\Windows\System32\cmd.exe` directly and capture stdout/stderr from the build.

## Porting Rules

- Reuse the existing NDS code instead of rewriting the port.
- `SH_NDS_PORT` is the Nintendo DS target.
- Do not accidentally include PC-only code through `SH_PC_PORT`.
- Preserve original PS1 behavior where possible.
- Replace MIPS/PsyQ-specific functionality with ARM9/NDS implementations.
- Do not hide ABI/type/layout problems with random casts.
- Use hardware NDS 3D/GPU features where possible instead of building an unnecessary full software renderer.
- Avoid floating point and expensive per-frame allocations when possible.

## Priority

Current priority is:

```text
compile
→ link
→ initialize NDS backend
→ load original game data
→ renderer/GTE
→ overlays
→ real game loop
→ first Silent Hill scene
```

Do not stop after producing a bootable `.nds`.

A ROM that only shows a logo, triangle, test texture, or debug screen is **not the objective**.

Keep working toward executing the actual Silent Hill engine.