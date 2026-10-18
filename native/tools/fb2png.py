#!/usr/bin/env python3
"""Convert raw RGB565 framebuffer dumps from the host simulator to PNG."""

import glob
import os
import struct
import sys
import zlib

W, H = 320, 240


def fb_to_rgb(data: bytes) -> bytearray:
    vals = struct.unpack("<%dH" % (W * H), data)
    rgb = bytearray(W * H * 3)
    # The game stores the framebuffer in panel order: fb[x * H + y].
    for y in range(H):
        for x in range(W):
            v = vals[x * H + y]
            r = ((v >> 11) & 0x1F) * 255 // 31
            g = ((v >> 5) & 0x3F) * 255 // 63
            b = (v & 0x1F) * 255 // 31
            j = (y * W + x) * 3
            rgb[j] = r
            rgb[j + 1] = g
            rgb[j + 2] = b
    return rgb


def write_png(path: str, w: int, h: int, rgb: bytearray) -> None:
    def chunk(tag: bytes, payload: bytes) -> bytes:
        return (
            struct.pack(">I", len(payload))
            + tag
            + payload
            + struct.pack(">I", zlib.crc32(tag + payload) & 0xFFFFFFFF)
        )

    raw = bytearray()
    for y in range(h):
        raw.append(0)
        raw += rgb[y * w * 3 : (y + 1) * w * 3]

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    png += chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(png)


def main() -> None:
    out = sys.argv[1] if len(sys.argv) > 1 else "out"
    files = sorted(glob.glob(os.path.join(out, "*.bin")))
    if not files:
        raise SystemExit("no .bin frames found")

    frames = []
    for path in files:
        data = open(path, "rb").read()
        if len(data) != W * H * 2:
            raise SystemExit(f"{path}: expected {W * H * 2} bytes, got {len(data)}")
        rgb = fb_to_rgb(data)
        png_path = os.path.splitext(path)[0] + ".png"
        write_png(png_path, W, H, rgb)
        frames.append((os.path.basename(path), rgb))
        print(png_path)

    # 2-column contact sheet with 4 px gutters.
    cols = 2
    rows = (len(frames) + cols - 1) // cols
    gap = 4
    sw = cols * W + (cols + 1) * gap
    sh = rows * H + (rows + 1) * gap
    sheet = bytearray(sw * sh * 3)
    for i, (_, rgb) in enumerate(frames):
        cx = gap + (i % cols) * (W + gap)
        cy = gap + (i // cols) * (H + gap)
        for y in range(H):
            src = y * W * 3
            dst = ((cy + y) * sw + cx) * 3
            sheet[dst : dst + W * 3] = rgb[src : src + W * 3]

    write_png(os.path.join(out, "sheet.png"), sw, sh, sheet)
    print(os.path.join(out, "sheet.png"))


if __name__ == "__main__":
    main()
