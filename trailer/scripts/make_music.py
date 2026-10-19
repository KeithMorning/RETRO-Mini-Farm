#!/usr/bin/env python3
"""Generate the trailer's royalty-free chiptune bed."""
import math
import struct
import wave

SR = 44100
DUR = 668 / 30
CHORDS = [
    [261.63, 329.63, 392.00],
    [196.00, 246.94, 293.66],
    [220.00, 261.63, 329.63],
    [174.61, 220.00, 261.63],
]
MEL = [0, 2, 1, 2, 0, 2, 1, 2, 0, 2, 1, 2, 0, 2, 1, 2]

out = []
for i in range(int(SR * DUR)):
    t = i / SR
    bar = int(t / 2.0) % 4
    step = int((t % 2.0) / 0.5) % 4
    f = CHORDS[bar][MEL[step] % 3] * 2
    env = 0.16 if t < 2 else (0.22 if t < 16 else 0.18)
    lead = (math.sin(2 * math.pi * f * t) + 0.25 * math.sin(4 * math.pi * f * t)) * env
    bass = math.sin(2 * math.pi * (CHORDS[bar][0] / 2) * t) * 0.12
    pad = sum(math.sin(2 * math.pi * cf * t) for cf in CHORDS[bar]) * 0.025
    hat = 0.0
    if (t % 0.5) < 0.025:
        hat = (1 - (t % 0.5) / 0.025) * 0.035 * math.sin(2 * math.pi * 7000 * t)
    v = (lead + bass + pad + hat) * min(1, t) * min(1, (DUR - t) / 1.2)
    out.append(int(max(-0.8, min(0.8, v)) * 32767))

with wave.open("public/music.wav", "wb") as w:
    w.setnchannels(1)
    w.setsampwidth(2)
    w.setframerate(SR)
    w.writeframes(b"".join(struct.pack("<h", s) for s in out))
print("wrote public/music.wav")
