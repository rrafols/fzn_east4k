# Looking for the East

The Fuzzion 4k intro *Looking for the East* (2003, 2nd place at Breakpoint), and
its ports.

## Layout

    common/          everything that is not specific to one platform
      data/          the demo's content: module, instruments, glyphs, arrangement
      doc/           what each effect does, and how the player works
      reference/     the original 2003 release, and the .it it was made from
      tools/         frame comparison and contact sheets, used by every port
    ports/
      win32/         the original, as released in 2003
      macos-x86_64/  Intel macOS, CGL + CoreAudio
      webgl/         browsers, WebGL 2 + Web Audio
    packers/
      onekpaq64/     64-bit port of the oneKpaq decompressor, for any x86-64 port


## What is genuinely shared

`common/data/` is the source of truth for the intro's *content*, and every port
takes the same files:

| file | what it is |
|---|---|
| `zik.asm` | the module: order list, patterns, channels |
| `samples.inc` | parameters for the eight generated instruments |
| `chin_char.inc` | the glyph strokes |
| `orderstuff.inc` | which effects play during which order |
| `cubedata.inc` | the unit cube, two bits per component |

These are NASM `db`/`dd` tables, so a port that is not assembling them will need
to convert them rather than include them - `ports/webgl/tools/extract_data.py`
does that by running them through nasm and reading the bytes back. The values
are what matters, and they are documented in `common/doc/`.

Everything above that line is per-port: the effects are the same *arithmetic*
everywhere, but the code is not portable — x87 on Intel, and the platform layer
differs completely.

## Building

Each port builds independently, from its own directory.

    cd ports/macos-x86_64 && make        # -> ./east, a 4877-byte Mach-O
    cd ports/win32/src    && build.bat   # needs DOS/Windows and nasm
    cd ports/webgl        && make serve  # no build step; open the URL it prints

## A note on the Win32 tree

`ports/win32/` is the 2003 source, unchanged apart from two things: it now takes
its data tables from `common/data/`, and `R.BAT` became `build.bat` pointing at
them. The shared copies differ from the 2003 originals only in line endings, a
dead `%if 0` block in `samples.inc`, and a vestigial `global` line in `zik.asm`
that named symbols which never existed (`_orderList` against a label of
`orderList`).

**This has not been re-tested on Windows**

## Credits

Original intro by **Fuzzion**, 2003 — code by bp and ufix, music by sml.
Released at Breakpoint 2003. The ports keep the original data intact.

**Port to macOS x86_64 done with heavy usage of Claude**.