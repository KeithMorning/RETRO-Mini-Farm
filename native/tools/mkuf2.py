#!/usr/bin/env python3
"""Pack a raw binary into UF2 for the ARCADE-F4 bootloader."""

import struct
import sys

BASE = 0x08010000
FAMILY = 0x57755A57
MAGIC0 = 0x0A324655
MAGIC1 = 0x9E5D5157
MAGIC_END = 0x0AB16F30
FLAG_FAMILY = 0x2000


def main() -> None:
    if len(sys.argv) != 3:
        raise SystemExit(f"usage: {sys.argv[0]} app.bin app.uf2")

    data = bytearray(open(sys.argv[1], "rb").read())
    while len(data) % 256:
        data.append(0xFF)

    nblocks = len(data) // 256
    out = bytearray()
    for i in range(nblocks):
        block = bytearray(512)
        struct.pack_into(
            "<IIIIIIII",
            block,
            0,
            MAGIC0,
            MAGIC1,
            FLAG_FAMILY,
            BASE + i * 256,
            256,
            i,
            nblocks,
            FAMILY,
        )
        block[32:288] = data[i * 256 : (i + 1) * 256]
        struct.pack_into("<I", block, 508, MAGIC_END)
        out += block

    open(sys.argv[2], "wb").write(out)
    print(f"{sys.argv[2]}: {nblocks} blocks, {len(data)} bytes payload, base 0x{BASE:08X}")


if __name__ == "__main__":
    main()
