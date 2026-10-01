#!/usr/bin/env python3
"""Host checks for the v1 package; ARM9 hardware build is a separate step."""
import ctypes
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import zlib

PKG = Path(__file__).resolve().parents[1]


def tim_fixture():
    clut = [0, 0x001f, 0x03e0, 0x7c00] + [0] * 12
    clutblock = struct.pack('<IHHHH', 44, 0, 0, 16, 1) + struct.pack('<16H', *clut)
    pixelblock = struct.pack('<IHHHH', 14, 0, 0, 1, 1) + b'\x21\x03'
    return struct.pack('<II', 0x10, 8) + clutblock + pixelblock


def make_fixture(root):
    assets = root / 'assets/USA/1ST'
    assets.mkdir(parents=True)
    (assets / '2ZANKO_E.TIM').write_bytes(tim_fixture())
    (assets / 'KONAMI.TIM').write_bytes(tim_fixture())
    (assets / 'BODYPROG.BIN').write_bytes(b'Original MIPS overlay bytes\0' * 200)
    entries = ['    FILE_1ST_2ZANKO_E_TIM = 0, // 1ST/2ZANKO_E.TIM',
               '    FILE_1ST_KONAMI_TIM = 1, // 1ST/KONAMI.TIM',
               '    FILE_1ST_BODYPROG_BIN = 2, // 1ST/BODYPROG.BIN',
               '    FILE_VIN_MAP0_S00_BIN = 3, // VIN/MAP0_S00.BIN']
    header = root / 'include/main/fileenum.h.USA.inc'
    header.parent.mkdir(parents=True)
    header.write_text('\n'.join(entries) + '\n', encoding='ascii')
    (root / 'nds_port/scene_include').mkdir(parents=True)
    subprocess.run([sys.executable, str(PKG / 'nds_port/tools/prepare_game_assets.py'),
                    '--repo', str(root), '--max-mib', '1'], check=True, capture_output=True, text=True)
    index = (root / 'nds_port/nitrofs/game/index.bin').read_bytes()
    assert index[:4] == b'SHFI'
    assert struct.unpack_from('<II', index, 4) == (1, 4)
    assert struct.unpack_from('<II', index, 12) == (len(tim_fixture()), 1)
    assert struct.unpack_from('<II', index, 12 + 3*8) == (0, 0)
    assert (root / 'nds_port/nitrofs/game/files/0002.bin').read_bytes() == b'Original MIPS overlay bytes\0' * 200
    print('PASS: real e_FsFile ordering, raw file pack, index format and optional-file handling')


def check_installer(root):
    (root / 'nds_port/Makefile.native').write_text('all:\n\t@echo bridge\n', encoding='ascii')
    main = root / 'Makefile'
    main.write_bytes(b'other:\n\t@echo other\n\nnds:\n\t$(MAKE) -C nds_port\n')
    shutil.copy(PKG / 'install_scene.py', root / 'install_scene.py')
    for _ in range(2):
        subprocess.run([sys.executable, str(root / 'install_scene.py')], cwd=root,
                       check=True, capture_output=True, text=True)
    assert main.read_bytes().count(b'-f Makefile.native') == 1
    assert (root / 'Makefile.before_nds_native').read_bytes().count(b'-f Makefile.native') == 0
    print('PASS: root Makefile surgery is narrow, idempotent, backed up')


def test_host_c(root):
    cc = shutil.which('gcc')
    if not cc or os.name == 'nt':
        print('SKIP: host-C tests need Unix gcc; packer and installer checks passed')
        return
    fake = root / 'fake'
    fake.mkdir()
    (fake / 'filesystem.h').write_text('#include <stdbool.h>\nstatic inline bool nitroFSInit(char **s) {(void)s; return true;}\n')
    (fake / 'shim.h').write_text(r'''#include <stdio.h>
#include <string.h>
static inline FILE *sh_test_fopen(const char *name, const char *mode) {
    char path[1024];
    if (strncmp(name,"nitro:/",7) == 0) {
        snprintf(path,sizeof(path),"nds_port/nitrofs/%s",name+7);
        return fopen(path,mode);
    }
    return fopen(name,mode);
}
#define fopen sh_test_fopen
''')
    (fake / 'nds.h').write_text(r'''#ifndef TEST_NDS_H
#define TEST_NDS_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef int16_t v16;
typedef int16_t t16;
typedef struct {int unused;} PrintConsole;
#define KEY_A 1
#define KEY_B 2
#define KEY_X 4
#define KEY_SELECT 8
#define KEY_START 16
#define MODE_0_3D 1
#define MODE_0_2D 2
#define VRAM_A_TEXTURE 1
#define VRAM_C_SUB_BG 2
#define BgType_Text4bpp 0
#define BgSize_T_256x256 0
#define GL_PROJECTION 1
#define GL_MODELVIEW 2
#define GL_RGBA 7
#define GL_TEXTURE_2D 1
#define TEXTURE_SIZE_256 5
#define GL_MAX_DEPTH 0x7fff
#define GL_TRIANGLES 0
#define GL_QUADS 1
#define POLY_CULL_NONE 192
#define POLY_ALPHA(x) ((x)<<16)
#define inttot16(n) ((n)<<4)
void lcdMainOnTop(void);
void videoSetMode(int);
void videoSetModeSub(int);
void vramSetBankA(int);
void vramSetBankC(int);
void glInit(void);
void glViewport(int,int,int,int);
void glMatrixMode(int);
void glLoadIdentity(void);
void glOrtho(float,float,float,float,float,float);
void glClearColor(int,int,int,int);
void glClearDepth(int);
void glPolyFmt(int);
int glGenTextures(int,int*);
void glBindTexture(int,int);
int glTexImage2D(int,int,int,int,int,int,int,const void*);
void glBegin(int);
void glEnd(void);
void glColor3b(int,int,int);
void glVertex3v16(int,int,int);
void glTexCoord2t16(int,int);
void glEnable(int);
void glDisable(int);
void glFlush(int);
void DC_FlushRange(const void*,int);
void swiWaitForVBlank(void);
void scanKeys(void);
int keysDown(void);
int keysHeld(void);
int pmMainLoop(void);
void consoleClear(void);
void consoleSelect(PrintConsole*);
void consoleInit(PrintConsole*,int,int,int,int,int,int,int);
int iprintf(const char*,...);
#endif
''')
    includes = ['-I', str(fake), '-I', str(PKG / 'nds_port/scene_include')]
    src = PKG / 'nds_port/scene_source'
    core = [src/'sh_nds_fs.c', src/'sh_nds_overlay.c', src/'sh_nds_tim.c']
    lib = root / 'sh_test.so'
    subprocess.run([cc, '-std=gnu11', '-O2', '-Wall', '-Wextra', '-Werror', '-shared', '-fPIC',
                    '-include', str(fake/'shim.h'), *includes, *map(str,core), '-o', str(lib)],
                   check=True, capture_output=True, text=True)
    for file in (src/'sh_nds_gpu.c', src/'main.c'):
        subprocess.run([cc, '-std=gnu11', '-Wall', '-Wextra', '-Werror=implicit-function-declaration',
                        '-fsyntax-only', *includes, str(file)],
                       check=True, capture_output=True, text=True)
    native = ctypes.CDLL(str(lib))
    native.sh_fs_mount.restype = ctypes.c_bool
    native.sh_fs_info.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_uint32)]
    native.sh_fs_info.restype = ctypes.c_bool
    old = Path.cwd()
    try:
        os.chdir(root)
        assert native.sh_fs_mount()
        length = ctypes.c_uint32()
        assert native.sh_fs_info(2, ctypes.byref(length)) and length.value == len(b'Original MIPS overlay bytes\0'*200)
        assert not native.sh_fs_info(3, ctypes.byref(length))
        native.sh_overlay_probe.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_uint32), ctypes.POINTER(ctypes.c_uint32)]
        native.sh_overlay_probe.restype = ctypes.c_bool
        checksum = ctypes.c_uint32()
        assert native.sh_overlay_probe(2, ctypes.byref(length), ctypes.byref(checksum))
        assert checksum.value == zlib.crc32(b'Original MIPS overlay bytes\0'*200)
        native.sh_tim_decode_screen.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_uint,
                                                ctypes.c_void_p, ctypes.c_size_t,
                                                ctypes.POINTER(ctypes.c_uint), ctypes.POINTER(ctypes.c_uint)]
        native.sh_tim_decode_screen.restype = ctypes.c_bool
        image = (ctypes.c_uint16 * (256*256))()
        width = ctypes.c_uint(); height=ctypes.c_uint()
        tim = ctypes.create_string_buffer(tim_fixture())
        assert native.sh_tim_decode_screen(tim,len(tim_fixture()),0,image,256*256,
                                           ctypes.byref(width),ctypes.byref(height))
        assert (width.value,height.value)==(4,1)
        assert image[64*256] == 0x801f
        assert not native.sh_tim_decode_screen(tim,8,0,image,256*256,
                                               ctypes.byref(width),ctypes.byref(height))
    finally:
        os.chdir(old)
    print('PASS: all bridge C files parse; real TIM decode; streamed overlay CRC; indexed NitroFS reads')


def main():
    with tempfile.TemporaryDirectory(prefix='shnds_native_test_') as tmp:
        root=Path(tmp)
        make_fixture(root)
        check_installer(root)
        test_host_c(root)
    print('PASS: host smoke suite complete. ARM9 runtime not tested here.')

if __name__=='__main__':
    try:
        main()
    except subprocess.CalledProcessError as e:
        print('SUBPROCESS STDERR:', e.stderr or '(none)', file=sys.stderr)
        raise
