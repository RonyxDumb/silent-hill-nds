ifeq ($(strip $(DEVKITPRO)),)
$(error DEVKITPRO is not set; run inside devkitPro or use docker_build.bat)
endif

ifeq ($(strip $(DEVKITARM)),)
$(error DEVKITARM is not set; run inside devkitPro or use docker_build.bat)
endif

include $(DEVKITARM)/ds_rules

TARGET      := silent_hill
BUILD       := build
NITROFS     := nitrofs
GAME_ICON   := icon.bmp
ARM7_ELF    := $(DEVKITPRO)/calico/bin/ds7_maine.elf

ARCH        := -mthumb -mthumb-interwork
CFLAGS      := -std=gnu11 -g -Wall -Wextra -Os $(ARCH) \
               -fno-strict-aliasing -fwrapv -fno-common \
               -DARM9 -D__NDS__ -DSH_NDS_PORT  \
               -DUSE_EXTENDED_PRIM_POINTERS=1 -DNO_OPENGL=1 \
               -DVER_USA -DSKIP_ASM -DMAP0_S00 -DSH_MAP_NAME=map0_s00 \
               -include $(CURDIR)/include/nds_compat.h
CFLAGS      += -Wno-incompatible-pointer-types -Wno-int-to-pointer-cast \
               -Wno-pointer-to-int-cast -Wno-unused-function -Wno-unused-variable \
               -Wno-error=implicit-function-declaration -Wno-error=implicit-int \
               -Wno-error=int-conversion -Wno-error=return-mismatch
CXXFLAGS    := $(filter-out -std=gnu11,$(CFLAGS)) -std=gnu++17 -fno-rtti -fno-exceptions
LDFLAGS     := -g $(ARCH) -Wl,-Map,$(TARGET).map
LIBS := -Wl,--start-group -lmm9 -lfilesystem -lfat -lnds9 -lcalico_ds9 -Wl,--end-group -lm
LIBDIRS     := $(LIBNDS)
INCLUDES    := -I$(CURDIR)/include -I$(CURDIR)/../pc_port/include \
               -I$(CURDIR)/../include -I$(CURDIR)/../include/decomp \
               -I$(CURDIR)/../pc_port/PsyCross/include/psx \
               -I$(CURDIR)/../pc_port/PsyCross/include

ifeq ($(BUILD_READY),1)
include $(BUILD)/sources.mk
else
-include $(BUILD)/sources.mk
endif

GAME_OBJECTS := $(patsubst ../%.c,$(BUILD)/game/%.o,$(GAME_SOURCES))
NDS_OBJECTS  := $(patsubst %.c,$(BUILD)/nds/%.o,$(NDS_SOURCES))
GTE_C_OBJECTS   := $(patsubst ../%.c,$(BUILD)/game/%.o,$(GTE_C_SOURCES))
GTE_CXX_OBJECTS := $(patsubst ../%.cpp,$(BUILD)/game/%.o,$(GTE_CXX_SOURCES))
OFILES       := $(GAME_OBJECTS) $(GTE_C_OBJECTS) $(GTE_CXX_OBJECTS) $(NDS_OBJECTS)
DEPS         := $(OFILES:.o=.d)

.PHONY: all rom configure clean audit

# NDS system headers
INCLUDES += -I$(DEVKITPRO)/libnds/include
INCLUDES += -I$(DEVKITPRO)/calico/include

all: configure
	@$(MAKE) --no-print-directory -f Makefile.postconfigure BUILD_READY=1 rom

rom: $(TARGET).nds audit

.PHONY: FORCE
FORCE:

configure: FORCE
	@mkdir -p $(BUILD)
	@python3 tools/configure.py --root .. --out $(BUILD)/sources.mk

$(TARGET).nds: $(TARGET).elf
	ndstool -c $@ -9 $< -7 $(ARM7_ELF) -d $(NITROFS) $(if $(wildcard $(GAME_ICON)),-b $(GAME_ICON) "SILENT HILL DS;Native decomp port;By Francesco Pio Pipino",) -g SHDS

$(TARGET).elf: $(OFILES)
	$(CXX) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

$(BUILD)/game/%.o: ../%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# GCC 16 correctly rejects the stale s32 definition in the decomp because its
# public prototype uses e_CharaId. Patch a generated copy for ARM9, leaving the
# original repository sources untouched.
$(BUILD)/game/src/bodyprog/gfx/world_draw.o: $(BUILD)/patched/world_draw.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

$(BUILD)/patched/world_draw.c: ../src/bodyprog/gfx/world_draw.c tools/prepare_source.py
	@mkdir -p $(dir $@)
	@python3 tools/prepare_source.py --input $< --output $@ --unit world_draw

$(BUILD)/game/%.o: ../%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

$(BUILD)/nds/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

audit:
	@python3 tools/audit.py --root .. --sources $(BUILD)/sources.mk $(if $(wildcard $(TARGET).elf),--elf $(TARGET).elf,)

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).nds $(TARGET).map

-include $(DEPS)


# NDS toolchain: use the binaries belonging to this DEVKITARM installation.
override CC      := $(DEVKITARM)/bin/arm-none-eabi-gcc
override CXX     := $(DEVKITARM)/bin/arm-none-eabi-g++
override AR      := $(DEVKITARM)/bin/arm-none-eabi-gcc-ar
override AS      := $(DEVKITARM)/bin/arm-none-eabi-as
override OBJCOPY := $(DEVKITARM)/bin/arm-none-eabi-objcopy
LIBDIRS += $(DEVKITPRO)/portlibs/nds
LDFLAGS += -L$(DEVKITPRO)/libnds/lib
LDFLAGS += -L$(DEVKITPRO)/portlibs/nds/lib
LDFLAGS += -L$(DEVKITPRO)/calico/lib

CFLAGS += -ffunction-sections -fdata-sections
CXXFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

LDFLAGS += -specs=$(DEVKITPRO)/calico/share/ds9.specs

CFLAGS += -ffunction-sections -fdata-sections -fno-short-enums

CXXFLAGS += -ffunction-sections -fdata-sections -fno-short-enums
