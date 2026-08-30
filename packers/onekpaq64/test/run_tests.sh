#!/bin/bash
# Round-trips the 64-bit decompressor against oneKpaq's own encoder.
#   usage: ONEKPAQ=/path/to/onekpaq ./run_tests.sh
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ONEKPAQ="${ONEKPAQ:-/tmp/onekpaq/onekpaq}"
[ -x "$ONEKPAQ" ] || { echo "set ONEKPAQ to the oneKpaq encoder binary"; exit 2; }
ENCDIR="$(dirname "$ONEKPAQ")"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT

# a spread of inputs: code, text, random, zeros, and something highly repetitive
head -c 512 "$ONEKPAQ"                    > "$TMP/code.bin"
head -c 300 /usr/share/dict/words 2>/dev/null > "$TMP/text.bin" || \
    printf 'the quick brown fox %.0s' {1..20} > "$TMP/text.bin"
head -c 256 /dev/urandom                  > "$TMP/rand.bin"
python3 -c "open('$TMP/zeros.bin','wb').write(bytes(300))"
python3 -c "open('$TMP/rep.bin','wb').write(b'ABCDEFGH'*50)"

pass=0; fail=0
for f in code text rand zeros rep; do
    ( cd "$ENCDIR" && rm -f onekpaq_context.cache && \
      "$ONEKPAQ" 3 1 "$TMP/$f.bin" "$TMP/$f.okp" ) >"$TMP/$f.log" 2>&1
    line=$(grep -o 'offset=[0-9]* shift=[0-9]*' "$TMP/$f.log")
    off=${line#offset=}; off=${off%% *}; sh=${line##*shift=}
    if [ -z "$off" ]; then echo "  $f: encoder failed"; fail=$((fail+1)); continue; fi
    nasm -f macho64 -I"$HERE/.." -DONEKPAQ_DECOMPRESSOR_SHIFT=$sh \
         "$HERE/wrap.asm" -o "$TMP/wrap.o"
    cc -o "$TMP/harness" "$HERE/harness.c" "$TMP/wrap.o" -Wl,-w
    r=$("$TMP/harness" "$TMP/$f.bin" "$TMP/$f.okp" "$off" | tail -1)
    printf "  %-8s shift=%-3s offset=%-3s %s\n" "$f" "$sh" "$off" "$r"
    case "$r" in *"ROUND TRIP OK"*) pass=$((pass+1));; *) fail=$((fail+1));; esac
done
echo; echo "passed $pass, failed $fail"; [ "$fail" -eq 0 ]
