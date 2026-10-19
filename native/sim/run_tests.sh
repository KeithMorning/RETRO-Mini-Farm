#!/usr/bin/env bash
# Build and run host regression tests. Shared objects are built by `make`.
set -euo pipefail

cd "$(dirname "$0")/.."   # native/
CC="${CC:-cc}"
CFLAGS="-O1 -std=gnu11 -Isrc -Wall -Wextra"
OBJ=build/host
mkdir -p "$OBJ"

# Ensure shared game/font/save/audio objects exist and are up to date.
make -s host-test-objs

tests="save_test gameplay_test menu_test upgrade_test season_test beach_test pet_test story_test mine_test easter_test goal_test"

for t in $tests; do
    case "$t" in
        save_test|menu_test) save_obj="$OBJ/save_file.o" ;;
        *)                   save_obj="$OBJ/save_stub.o" ;;
    esac
    "$CC" $CFLAGS -c "sim/$t.c" -o "$OBJ/$t.o"
    "$CC" $CFLAGS -o "sim/$t" "$OBJ/$t.o" "$OBJ/game.o" "$OBJ/font8.o" \
        "$save_obj" "$OBJ/sound_stub.o"
    (cd sim && "./$t" >/dev/null)
    echo "ok $t"
done

echo "all host tests passed"
