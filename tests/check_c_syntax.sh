#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
cat > "$TMP/nds.h" <<'END'
#ifndef TEST_NDS_H
#define TEST_NDS_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
typedef int16_t s16;
typedef uint16_t u16;
typedef struct { int unused; } PrintConsole;
#define MODE_0_2D 0
#define VRAM_C_SUB_BG 0
#define BgType_Text4bpp 0
#define BgSize_T_256x256 0
#define KEY_A 1u
#define KEY_B 2u
#define KEY_SELECT 4u
#define KEY_START 8u
#define KEY_LEFT 16u
#define KEY_RIGHT 32u
#define KEY_UP 64u
#define KEY_DOWN 128u
#define SoundFormat_16Bit 0
void videoSetModeSub(int);
void vramSetBankC(int);
void consoleInit(PrintConsole*,int,int,int,int,int,int,int);
void consoleSelect(PrintConsole*);
void consoleClear(void);
int iprintf(const char*,...);
void swiWaitForVBlank(void);
void scanKeys(void);
int keysDown(void);
void* bgGetGfxPtr(int);
void dmaFillHalfWords(unsigned int,void*,unsigned int);
void DC_FlushRange(const void*, unsigned int);
void soundKill(int);
int soundPlaySample(const void*,int,unsigned int,u16,int,int,bool,int);
#endif
END
cat > "$TMP/filesystem.h" <<'END'
#include <stdbool.h>
bool nitroFSInit(const char*);
END
cat > "$TMP/platform_nds.h" <<'END'
#ifndef TEST_PLATFORM_H
#define TEST_PLATFORM_H
#include <nds.h>
void sh_nds_video_init(void);
bool sh_nds_video_load_bitmap(const char*);
void sh_nds_audio_init(void);
void sh_nds_audio_confirm(void);
#endif
END
for F in main.c sh_asset_gallery.c; do
  gcc -std=c23 -Wall -Wextra -Werror -fsyntax-only -I"$TMP" -I"$ROOT/nds_port/include" "$ROOT/nds_port/source/$F"
done
echo "PASS: C23 host syntax check (mock libnds headers)"
