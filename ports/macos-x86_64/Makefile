# "Looking for the East" - Fuzzion, 2003 - macOS x86-64 port
#
#   make            build the packed intro       -> ./east      (fullscreen)
#   make windowed   build a windowed version     -> build/east-windowed
#   make intro      build it linked, unpacked    -> build/intro (easier to debug)
#   make verify     render frames + audio offscreen, packed vs linked
#   make clean

NASM      := nasm
COMMON    := ../../common
NASMFLAGS := -f macho64 -Isrc/ -I$(COMMON)/data/
LDFLAGS   := -framework OpenGL -framework ApplicationServices -framework AudioToolbox -Wl,-w
SRC       := $(wildcard src/*.asm src/*.inc $(COMMON)/data/*)

.PHONY: all intro windowed verify heatmap clean

all: east

east: $(SRC) tools/pack.py | build
	@python3 tools/pack.py

intro: build/intro
build/intro: $(SRC) | build
	$(NASM) $(NASMFLAGS) src/intro.asm -o build/intro.o
	$(CC) -o $@ build/intro.o $(LDFLAGS)

# Windowed needs Cocoa (NSWindow, NSOpenGLContext) driven through objc_msgSend,
# which CGL cannot do on its own.  That is ~700 bytes, so it is a separate
# build and never part of the packed intro.
windowed: build/east-windowed
build/east-windowed: $(SRC) | build
	$(NASM) $(NASMFLAGS) -DWINDOWED=1 src/intro.asm -o build/win.o
	$(CC) -o $@ build/win.o $(LDFLAGS) -framework Cocoa

build/shots: $(SRC) | build
	$(NASM) $(NASMFLAGS) -DOFFSCREEN=1 src/intro.asm -o build/shots.o
	$(CC) -o $@ build/shots.o $(LDFLAGS)

# Renders the same thirteen frames from the linked and the packed build and
# compares them.  Only the five effect-1 frames may differ: their background
# lines are seeded from the clock, exactly as in the original.
verify: build/shots | build
	@python3 tools/pack.py --offscreen >/dev/null
	@cd build && rm -rf linked packed && mkdir linked packed
	@cd build && ./shots >/dev/null && mv shot*.ppm song.wav linked/
	@cd build && ./east_shots >/dev/null && mv shot*.ppm song.wav packed/
	@python3 $(COMMON)/tools/compare.py build/linked build/packed
	@python3 $(COMMON)/tools/contact.py 'build/packed/shot*.ppm' build/contact.png
	@echo "contact sheet: build/contact.png   audio: build/packed/song.wav"

# Where the packed bytes actually go: measures each block's incremental cost
# under the packer's own LZMA settings, not its raw size.
heatmap: | build
	@python3 tools/heatmap.py

build:
	@mkdir -p build

clean:
	rm -rf build east
