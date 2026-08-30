# Looking for the East — macOS port

A port of the Fuzzion 4k intro *Looking for the East* (2003) from 32-bit Win32
assembly to native 64-bit assembly for Intel macOS.

The original was NASM x86-32 talking to `CreateWindowEx` + WGL + DirectSound,
packed with aPack into a 4094-byte `.com`. This is the same intro — same
effects, same procedurally generated instruments, same module — rebuilt for
x86-64 Mach-O on top of CGL and CoreAudio.

    make            # build the packed intro -> ./east
    ./east          # fullscreen; Esc quits, and it ends with the tune

    make windowed   # -> build/east-windowed, a 1280x960 window

Requires an Intel Mac. Tested on macOS 15.7.4, Radeon Pro 575X.

Fullscreen is the packed 4k build. Windowed goes through Cocoa - NSWindow and
NSOpenGLContext driven from assembly via `objc_msgSend`, because CGL can capture
a display but cannot make a window - which costs about 700 bytes, so it is a
separate build rather than a runtime flag.

## What had to change

| | Win32 original | macOS port |
|---|---|---|
| window | `CreateWindowEx` + `ChangeDisplaySettings` | `CGDisplayCapture` + `CGLSetFullScreenOnDisplay` |
| GL context | `wglCreateContext` / `wglMakeCurrent` | `CGLCreateContext` / `CGLSetCurrentContext` |
| audio | 8 DirectSound buffers, pitch and volume per buffer | one pre-rendered AudioQueue buffer |
| player | thread waking every 114 ms | tune mixed up-front, `order` derived from the clock |
| calls | stdcall, arguments on the stack | System V AMD64, arguments in registers |
| imports | PE import table | `dlopen`/`dlsym` at start-up |
| exit key | `WM_KEYDOWN` | `CGEventSourceKeyState` |
| resolution | forced 640x480 mode | renders at 640x480, blitted up to the display |

Everything above the platform layer is the original code. The effects are the
same x87 arithmetic — `fsin`, `fprem`-based `fexp`, the resonant low-pass in the
sample generator — because x86-64 still has the x87 unit, and its results differ
from a C `sin()` in ways the intro's sound depends on.

Two things genuinely had to be redesigned:

**Audio.** DirectSound gave every sample buffer its own playback rate and
volume, and the original just drove eight of them from a thread. CoreAudio has
no equivalent, so the mixing that DirectSound did is now explicit
(`src/player.inc`): eight voices with position, step and gain, rendered
one row at a time into a single AudioQueue buffer before the intro starts.
That also removes the player thread — visuals and music both come off one
monotonic clock.

**Aspect ratio.** The original forced a 640×480 mode and hard-coded 4:3. A
modern Mac keeps its own resolution, so the ratio is taken from the display and
fed to `gluPerspective`; vertical framing matches the original and a wide
display simply sees more.

## Size

The Win32 binary was 8862 bytes, aPack'd to 4094. This one:

```
header + load commands    536
loader stub               168
intro, packed            4148   (8306 raw, 49.9%)
dyld bind opcodes          25
------------------------------
east                     4877 bytes
```

It does not fit in 4096, and the gap is compression, not code. Two costs are
structural on macOS:

* **4096 bytes is the floor.** The kernel refuses to exec a Mach-O whose file is
  shorter than the page its first segment maps - a 404-byte binary is killed
  before dyld runs. The page is paid for whether or not it is used.
* **536 of it are mandatory.** `__PAGEZERO`, `__TEXT`, `__LINKEDIT`,
  `LC_DYLD_INFO_ONLY`, `LC_SYMTAB`, `LC_DYSYMTAB`, `LC_LOAD_DYLINKER`,
  `LC_MAIN`, `LC_BUILD_VERSION` and `LC_LOAD_DYLIB` are each load-bearing - drop
  `LC_DYSYMTAB` and dyld refuses the binary. The PE header the original used was
  smaller, and the intro hid strings in the unused parts of the DOS header.

Two tricks claw some of it back. `__LINKEDIT` is declared as an *empty* segment
based at the image address, which makes dyld resolve `LC_DYLD_INFO` offsets
against the first page - otherwise the bind opcodes need a second page and the
file jumps to 8 KB. And the intro unpacks into the zero-filled tail of `__TEXT`
(`vmsize` simply exceeds `filesize`), so there is no `mmap` and no second
segment.

That leaves 3367 bytes for the payload, against 4148 today: the compressor would
have to reach 40% where LZMA reaches 49.9%.

### What was tried

Compressors, on the 7839-byte payload:

| approach | result |
|---|---|
| LZMA via `compression_decode_buffer`, lc/lp/pb swept | **51.4%** - used |
| brotli -q11 | 51.8% |
| zstd --ultra -22 | 53.5% |
| bzip2 -9 | 60.1% |
| raw LZMA1, no container | 50.4%, but needs ~400 bytes of decoder - a net loss |
| context mixing, orders 1-4 | 60.9% |
| context mixing + match model | 62.5% |
| x86 BCJ prefilter | 12 bytes |
| separate code and data streams | 76 bytes *worse* |

And on the code itself:

| change | raw | compressed | |
|---|---|---|---|
| park RBP on a 256-byte constant pool | -936 | large win | kept |
| helper routines for repeated call shapes | -300 | **+88** | reverted |
| inline call descriptors, 131 sites | -352 | **+12** | reverted |

The last two are the interesting result, and they point the same way: **the
payload is already at LZMA's entropy floor.** Every `glColor3f(1,1,1)` assembles
to the same eighteen bytes as every other one, so LZMA encodes the repeats for
almost nothing. Hand-encoding those call sites down to seven bytes each removes
832 bytes of code and *loses*, because what it removes is the redundancy the
compressor was living on. Shortening the code is not the lever; reducing its
information content is.

### oneKpaq

[oneKpaq](https://github.com/temisu/oneKpaq) is the live Crinkler-class packer
for macOS: PAQ-style context mixing, BSD-2. Measured on identical slices of this
payload it beats raw LZMA by a consistent **8%** (1 KB: 628 vs 681; 2 KB: 1165
vs 1267) - the only thing tried here that wins at all.

It ships a **32-bit** decompressor only, and macOS has not run 32-bit code since
Catalina, so `../../packers/onekpaq64/` holds a 64-bit port of it - 171 bytes, round-trip
tested against the real encoder. The build uses it when the encoder is
available:

    ONEKPAQ=/path/to/onekpaq make

and falls back to the system LZMA otherwise, which costs about 300 bytes more.
Compression results are cached under `build/okp-cache/` by payload hash, because
an encoder run takes a long time - the search is O(bits x models x iterations)
and this payload is roughly twice the size oneKpaq is built for. Build the
encoder itself with `-O3 -march=native`: it ships `-Os`, optimised for *size*,
for a compute-bound model search.

Two other macOS options were checked and rejected: **muncho**, a literal Crinkler
clone for OS X, is abandoned and its source was never really released; **iPakk**
is an LZMA packer, i.e. what is already here.

### Import by hash: measured, and not worth it

The obvious next cut is `symnames`, 422 packed bytes spent resolving imports by
name. Replacing it with hashes does not pay off, and the reason is structural: a
macOS process here loads **330 images with ~202,000 exported symbols**, so hashes
must be 32-bit (16-bit gives 153 false hits against 56 targets). That is 224
bytes of *incompressible* data against names that pack to 422 precisely because
they share prefixes - `gl`, `CGL`, `AudioQueue`. Add an export-trie walker, which
on macOS means a recursive walk of a prefix-compressed trie rather than a flat
PE-style export table, and the whole exercise comes out roughly 40 bytes
*worse*. Crinkler's 16-bit hashes work on Windows only because a process there
has a handful of DLLs.

## Verifying it

`make verify` builds the intro twice — linked, and packed — renders thirteen
frames from each into an offscreen framebuffer, and compares them:

    make verify

Only the five effect-1 frames may differ; their background lines are seeded from
the clock, as in the original. The mixed tune is compared byte for byte. This is
how the port was developed without taking over the display, and `build/contact.png`
is a contact sheet of the result.

One difference this caught is worth writing down: with provably identical GL
calls, the packed and linked builds still rendered a few pixels apart. macOS
picks the OpenGL driver plugin from the binary's platform version, and the
hand-built Mach-O had none — it was being handed the legacy *ATI* renderer while
the linked build got the modern *AMD* one. Adding `LC_BUILD_VERSION` (24 bytes)
fixed it.

## Layout

```
src/intro.asm       entry, start-up, texture and display-list build
src/frame.inc       the eight effects and the frame loop
src/draw.inc        cube, rubik grid, glyphs, fexp/rand/frand
src/sgen.inc        instrument synthesis  (ported from the original sgen.inc)
src/player.inc      module playback and mixing  (replaces DirectSound)
src/offscreen.inc   the verification build
src/window.inc      windowed mode (Cocoa via objc_msgSend)
src/data.inc        constant pool, variables, BSS
src/macros.inc      calling conventions and the import table
src/imports.inc     the imported symbols, hot ones first
src/consts.inc      GL / CGL / CoreAudio constants
src/stub.asm        loader stub for the packed build
src/onekpaq_decompressor64.asm   copy of ../../packers/onekpaq64/
tools/pack.py       builds the Mach-O and packs the intro
tools/heatmap.py    where the packed bytes go
tools/font5x7.py    a bitmap font, so the heatmap can label itself

The intro's content - the module, the instrument parameters, the glyph strokes,
the arrangement and the packed cube - is not here: it lives in `../../common/data/`
and is shared with the Win32 original. `../../common/tools/` holds the frame
comparison and contact sheet used by `make verify`, and `../../common/doc/`
describes what the effects do, which is the useful starting point for another
port.
```

## Credits

Original intro by **Fuzzion**, 2003 — code by Raimon Ràfols, music `searching
east`. Released at Bcnparty. This port keeps the original data files intact.
