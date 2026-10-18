#!/bin/bash
# Wait for the ARCADE-F4 bootloader drive and copy the current firmware.
# The bootloader only exposes the drive for ~3 seconds after reset/power-on.
set -e
cd "$(dirname "$0")"
test -f build/app.uf2 || make

for i in $(seq 1 1800); do
    if [ -d /Volumes/ARCADE-F4 ]; then
        for t in 1 2 3 4 5; do
            if cp build/app.uf2 /Volumes/ARCADE-F4/app.uf2 2>/dev/null; then
                echo "flashed (attempt $i)"
                exit 0
            fi
            sleep 0.15
        done
    fi
    sleep 0.15
done

echo "timeout: ARCADE-F4 never appeared"
exit 1
