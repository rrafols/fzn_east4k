# oneKpaq decompressor, x86-64

A 64-bit port of the decompressor from [oneKpaq](https://github.com/temisu/oneKpaq)
by Teemu Suutari — a PAQ-style context-mixing packer for 1k/4k intros, whose
compression lands within a few percent of Crinkler.

oneKpaq ships a 32-bit decompressor only; the README says *"if you need 64-bit,
contact me"*, and none of its eight forks has one. Since macOS dropped 32-bit
support in Catalina — the kernel answers `Bad CPU type in executable` even for a
static i386 binary with no libraries — a 64-bit decompressor is the only way to
use oneKpaq on a modern Mac at all.

**171 bytes**, against 143 for the 32-bit original (mode 3).

## Using it

The encoder is unchanged — build oneKpaq itself and run:

    ./onekpaq 3 1 input output.okp
    ...
    P offset=2 shift=5

Mode 3 is single-section with the fast decoder, which is what this ports. Then:

```asm
    mov     rbx, payload + offset       ; from the encoder's "offset="
    lea     rdi, [destination]
    finit                               ; two free x87 registers, DF clear
%include "onekpaq_decompressor64.asm"
```

Assemble with `-DONEKPAQ_DECOMPRESSOR_SHIFT=<shift>`, or patch it at run time:
the shift lives in a single byte, exported as `onekpaq_decompressor.shift`, so a
packer can write it without re-assembling.

Requirements, all inherited from the original:

* `rbx` points **into** the compressed data, `offset` bytes from its start — the
  bytes before that point are the header, and the decoder walks backwards into
  them.
* The compressed data must be **writable**. It is used as scratch and is
  destroyed.
* The destination must be zero-filled and writable from **-13** bytes to
  length+1.
* The decoder does not know the output length and simply keeps going, so give it
  a generous buffer and take the length from elsewhere.
* Everything is clobbered, including `xmm0`/`xmm1`. `rbx` and `rbp` are
  callee-saved under System V, so a C-callable wrapper must preserve them —
  see `test/wrap.asm`.

## What changed from the 32-bit original

| | |
|---|---|
| `PUSHAD`/`POPAD` | does not exist in 64-bit mode. Rather than expanding to eight pushes, only the registers actually live across each level are saved: three at the context level, three at the model level. `RCX` needs no saving at the model level because the weight loop `NEG`s it twice. |
| `SALC` | invalid in 64-bit mode; `SBB AL,AL` has the same effect for one byte more. |
| pointers | `RBX`/`RSI`/`RDI`/`RDX` carry addresses and take REX.W; `EAX`/`ECX`/`EBP`, and `EDX` while it holds `c0`, stay 32-bit exactly as before. |
| `LOOP` | uses `RCX` rather than `ECX`. Safe here: every write to the counter is a 32-bit operation, which zeroes the upper half. |
| `db 0xc0` | the original turns the following `sar dword [ebx],1` into `rcl cl,0x3b`, a nop. It still works, because `rbx` needs no REX prefix and the encoding is unchanged — but it would break if that register changed. |

## Tests

    ONEKPAQ=/path/to/onekpaq test/run_tests.sh

Compresses code, text, random bytes, zeros and repetitive data with the real
encoder, decompresses each with this decoder, and compares. This matters more
than usual: a context-mixing decoder that is subtly wrong produces plausible
output rather than crashing, so the only real check is a byte-exact round trip
across inputs that exercise different shift values.

Verified on macOS 15.7.4, x86-64: 7/7 inputs, shifts 1 to 16, offsets 0 to 3.

## Licence

BSD-2-Clause, as the original. See `LICENSE`; the copyright on the algorithm and
the assembly it derives from is Teemu Suutari's.
