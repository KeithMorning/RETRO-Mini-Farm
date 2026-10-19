TOOLCHAIN ?= $(CURDIR)/../toolchain/xpack-arm-none-eabi-gcc-15.2.1-1.1/bin
HOSTCC    ?= cc
CC      := $(TOOLCHAIN)/arm-none-eabi-gcc
OBJCOPY := $(TOOLCHAIN)/arm-none-eabi-objcopy
SIZE    := $(TOOLCHAIN)/arm-none-eabi-size

CFLAGS  := -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O2 -g \
           -Wall -Wextra -std=gnu11 -DSTM32F4 \
           -Ithird_party/libopencm3/include

LDFLAGS := -Tld/app.ld -nostartfiles -Wl,--gc-sections -Wl,-Map=build/app.map \
           -Wl,--print-memory-usage

LIBOPENCM3_DIR := third_party/libopencm3
LIBOPENCM3_REPO ?= https://github.com/libopencm3/libopencm3.git
LIBOPENCM3_REF ?= v0.8.0
LIBOPENCM3_A := $(LIBOPENCM3_DIR)/lib/libopencm3_stm32f4.a
LDLIBS  := $(LIBOPENCM3_A)

# Host build (simulator + regression tests). Shared objects avoid recompiling
# the large game.c once per test binary.
HOSTCFLAGS := -O1 -std=gnu11 -Isrc -Wall -Wextra
HOST_OBJ   := build/host
HOST_COMMON := $(HOST_OBJ)/game.o $(HOST_OBJ)/font8.o $(HOST_OBJ)/save_stub.o $(HOST_OBJ)/sound_stub.o
HOST_TEST   := $(HOST_COMMON) $(HOST_OBJ)/save_file.o

SRCS    := src/startup.c src/main.c src/game.c src/font8.c src/save.c
OBJS    := $(patsubst src/%.c,build/%.o,$(SRCS))

.PHONY: all clean flash size sim test deps host-test-objs

all: build/app.uf2

build:
	mkdir -p build

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIBOPENCM3_DIR)/Makefile:
	mkdir -p third_party
	git clone --depth 1 --branch $(LIBOPENCM3_REF) $(LIBOPENCM3_REPO) $(LIBOPENCM3_DIR)

$(LIBOPENCM3_A): $(LIBOPENCM3_DIR)/Makefile
	@mkdir -p build/toolshim
	@ln -sf "$$(command -v python3)" build/toolshim/python
	PATH="$(CURDIR)/build/toolshim:$$PATH" $(MAKE) -C $(LIBOPENCM3_DIR) TARGETS=stm32/f4 PREFIX=$(TOOLCHAIN)/arm-none-eabi-

deps: $(LIBOPENCM3_DIR)/Makefile

build/app.elf: $(OBJS) ld/app.ld $(LIBOPENCM3_A) | build
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

build/app.bin: build/app.elf
	$(OBJCOPY) -O binary $< $@

build/app.uf2: build/app.bin tools/mkuf2.py
	python3 tools/mkuf2.py $< $@

size: build/app.elf
	$(SIZE) $<

flash: build/app.uf2
	@test -d /Volumes/ARCADE-F4 || (echo "ARCADE-F4 not mounted"; exit 1)
	cp build/app.uf2 /Volumes/ARCADE-F4/app.uf2
	sync
	@echo "copied to ARCADE-F4; device should reboot"

$(HOST_OBJ):
	mkdir -p $@

$(HOST_OBJ)/%.o: src/%.c | $(HOST_OBJ)
	$(HOSTCC) $(HOSTCFLAGS) -c -o $@ $<

$(HOST_OBJ)/%.o: sim/%.c | $(HOST_OBJ)
	$(HOSTCC) $(HOSTCFLAGS) -c -o $@ $<

$(HOST_OBJ)/game.o: src/game.h src/save.h src/sound.h

host-test-objs: $(HOST_TEST)

test: host-test-objs
	bash sim/run_tests.sh

sim: sim/sim
	rm -rf sim/out
	cd sim && ./sim
	python3 tools/fb2png.py sim/out

sim/sim: sim/main.c $(HOST_COMMON)
	$(HOSTCC) $(HOSTCFLAGS) -o $@ sim/main.c $(HOST_COMMON)

clean:
	rm -rf build sim/sim sim/out
