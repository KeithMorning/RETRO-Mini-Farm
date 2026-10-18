#!/usr/bin/env bash
# Build and run the host regression tests. No ARM toolchain needed.
set -euo pipefail

cd "$(dirname "$0")/.."   # native/
CC="${CC:-cc}"
CFLAGS="-O2 -std=gnu11 -Isrc -Wall -Wextra"

tests="save_test gameplay_test menu_test upgrade_test season_test beach_test pet_test story_test mine_test easter_test"

for t in $tests; do
    case "$t" in
        save_test|menu_test) save_backend="sim/save_file.c" ;;
        *)                   save_backend="sim/save_stub.c" ;;
    esac
    "$CC" $CFLAGS -o "sim/$t" "sim/$t.c" src/game.c src/font8.c \
        "$save_backend" sim/sound_stub.c
    (cd sim && "./$t" >/dev/null)
    echo "ok $t"
done

echo "all host tests passed"
