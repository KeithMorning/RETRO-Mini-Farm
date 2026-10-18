#!/usr/bin/env python3
"""Read a UF2 image from an ARCADE-F4 bootloader drive and report flash usage.

Usage:
    python3 tools/uf2-info.py /Volumes/ARCADE-F4/CURRENT.UF2
    python3 tools/uf2-info.py app.uf2 --app-start 0x08010000
"""

import argparse
import struct
from pathlib import Path

UF2_MAGIC_START0 = 0x0A324655
UF2_MAGIC_START1 = 0x9E5D5157
RAM_BASE = 0x20000000
FLASH_END = 0x08080000


def read_uf2(path: Path) -> bytes:
    raw = path.read_bytes()
    blocks = {}
    for off in range(0, len(raw) - 511, 512):
        block = raw[off : off + 512]
        m0, m1, _flags, addr, psize, _bno, _nblocks, _fam = struct.unpack_from(
            "<IIIIIIII", block, 0
        )
        if m0 != UF2_MAGIC_START0 or m1 != UF2_MAGIC_START1:
            continue
        if psize > 476:
            psize = 256
        blocks[addr] = block[32 : 32 + psize]
    if not blocks:
        raise SystemExit("no UF2 blocks found")
    base = min(blocks)
    data = b"".join(blocks[a] for a in sorted(blocks))
    return base, data


def find_vectors(base: int, data: bytes):
    out = []
    for off in range(0, len(data) - 8, 4):
        sp, reset = struct.unpack_from("<II", data, off)
        if RAM_BASE <= sp <= RAM_BASE + 0x80000 and base <= reset < FLASH_END:
            if reset & 1:
                out.append((base + off, sp, reset))
    return out


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("uf2", type=Path)
    ap.add_argument("--app-start", type=lambda s: int(s, 0), default=None)
    ap.add_argument("--flash-end", type=lambda s: int(s, 0), default=FLASH_END)
    args = ap.parse_args()

    base, data = read_uf2(args.uf2)
    print(f"UF2        : {args.uf2}")
    print(f"image      : 0x{base:08X} .. 0x{base + len(data) - 1:08X} ({len(data) / 1024:.0f} KB)")

    vectors = find_vectors(base, data)
    print("vectors    :")
    page_vectors = [v for v in vectors if v[0] % 0x1000 == 0]
    for addr, sp, reset in page_vectors:
        print(f"  0x{addr:08X}  SP=0x{sp:08X} ({(sp - RAM_BASE) / 1024:.0f} KB)  reset=0x{reset:08X}")

    app_start = args.app_start
    if app_start is None:
        # The application vector table is normally at a 64 KB boundary and has
        # a stack pointer above the bootloader's own 64 KB region.
        candidates = [
            a for a, sp, _ in vectors if a % 0x10000 == 0 and sp >= RAM_BASE + 0x10000
        ]
        app_start = max(candidates) if candidates else base

    if not (base <= app_start < base + len(data)):
        raise SystemExit(f"app start 0x{app_start:08X} outside image")

    app_off = app_start - base
    app = data[app_off:]
    last = max(i for i, c in enumerate(app) if c != 0xFF)
    used = last + 1
    region = args.flash_end - app_start

    print(f"app start  : 0x{app_start:08X}")
    print(f"app end    : 0x{app_start + used - 1:08X}")
    print(f"app size   : {used / 1024:.0f} KB")
    print(f"app region : {region / 1024:.0f} KB")
    print(f"free       : {(region - used) / 1024:.0f} KB ({(region - used) / region * 100:.0f}%)")


if __name__ == "__main__":
    main()
