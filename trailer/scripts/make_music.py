#!/usr/bin/env python3
"""Original 'knowledge-vibe' bed: warm piano, plucks, airy pad, soft beat.

Inspired by the mood of the trending 'vibe knowledge' edits, but generated
from scratch here so the trailer has no third-party audio licensing issue.
"""
import math
import random
import struct
import wave

SR = 44100
DUR = 788 / 30
N = int(SR * DUR)
mix = [0.0] * N
random.seed(7)

def add_note(start, dur, freq, amp, kind="pluck", pan=0.0):
    i0 = int(start * SR)
    i1 = min(N, int((start + dur) * SR))
    for i in range(i0, i1):
        t = i / SR - start
        env = math.exp(-t * (7.0 if kind == "pluck" else 2.2))
        if kind == "pad":
            env = min(1.0, t / 1.2) * max(0.0, 1.0 - t / dur) * 0.8
        w = 2 * math.pi * freq * t
        if kind == "pluck":
            s = math.sin(w) + 0.32 * math.sin(2 * w + 0.02) + 0.10 * math.sin(3 * w)
        elif kind == "piano":
            s = math.sin(w) + 0.55 * math.sin(2 * w) + 0.18 * math.sin(3 * w)
            s += 0.08 * math.sin(4 * w)
            env = math.exp(-t * 2.0) * min(1.0, t / 0.02)
        else:
            s = math.sin(w) + 0.4 * math.sin(2 * w + 0.3)
        # gentle stereo width
        mix[i] += s * env * amp * (1.0 - pan * 0.25)

# A minor / F / C / G — a warm, nostalgic loop.
chords = [
    [220.00, 261.63, 329.63, 392.00],  # Am7
    [174.61, 220.00, 261.63, 329.63],  # Fmaj7
    [130.81, 164.81, 196.00, 246.94],  # Cmaj7
    [196.00, 246.94, 293.66, 349.23],  # G
]
beat = 60.0 / 88.0
bar = beat * 4
chord_len = bar * 2  # 8 beats per chord: spacious

# Pads: quiet, long, wide.
for k in range(int(DUR / chord_len) + 1):
    start = k * chord_len
    ch = chords[k % 4]
    for j, f in enumerate(ch):
        add_note(start, chord_len * 0.98, f, 0.030, "pad", (j - 1.5) * 0.12)
        add_note(start + 0.02, chord_len * 0.96, f * 2, 0.011, "pad", (1.5 - j) * 0.12)

# Piano-ish chord stabs and a simple lead motif.
melody = [
    659.25, 0, 523.25, 0, 493.88, 0, 440.00, 0,
    392.00, 0, 329.63, 0, 293.66, 0, 329.63, 0,
    440.00, 0, 523.25, 0, 659.25, 0, 783.99, 0,
    659.25, 0, 587.33, 0, 523.25, 0, 440.00, 0,
]
step = beat / 2
for k in range(int(DUR / step)):
    start = k * step
    f = melody[k % len(melody)]
    if f > 0 and start > 2.0:
        add_note(start, 0.85, f, 0.095, "pluck")

# Sparse piano chord hits on the bar.
for k in range(int(DUR / bar) + 1):
    start = k * bar
    if start < 2.0:
        continue
    for j, f in enumerate(chords[k % 4]):
        add_note(start + j * 0.035, 1.9, f * 2, 0.055, "piano")

# Soft kick / brushed snare / hat.
for k in range(int(DUR / beat) + 1):
    start = k * beat
    if start < 4.0:
        continue
    # kick on 1 and 3
    if k % 4 in (0, 2):
        for i in range(int(start * SR), min(N, int((start + 0.16) * SR))):
            t = i / SR - start
            f = 90 - 55 * min(1.0, t / 0.12)
            mix[i] += math.sin(2 * math.pi * f * t) * math.exp(-t * 22) * 0.22
    # brushed snare on 2 and 4
    if k % 4 in (1, 3):
        for i in range(int(start * SR), min(N, int((start + 0.12) * SR))):
            t = i / SR - start
            mix[i] += (random.random() * 2 - 1) * math.exp(-t * 30) * 0.035
    # quiet hat on 8ths
    if k % 2 == 0:
        for i in range(int(start * SR), min(N, int((start + 0.035) * SR))):
            t = i / SR - start
            mix[i] += (random.random() * 2 - 1) * math.exp(-t * 120) * 0.018

# A little vinyl crackle and air.
for _ in range(650):
    i = random.randrange(N)
    mix[i] += (random.random() * 2 - 1) * 0.012

# Simple 0.29s feedback delay for air.
delay = int(0.29 * SR)
for i in range(delay, N):
    mix[i] += mix[i - delay] * 0.18

# Fade in/out and normalize.
peak = max(1e-9, max(abs(v) for v in mix))
scale = 0.72 / peak
frames = []
for i, v in enumerate(mix):
    t = i / SR
    fade = min(1.0, t / 1.2) * min(1.0, max(0.0, (DUR - t) / 1.4))
    frames.append(int(max(-1.0, min(1.0, v * scale * fade)) * 32767))

with wave.open("public/music.wav", "wb") as w:
    w.setnchannels(1)
    w.setsampwidth(2)
    w.setframerate(SR)
    w.writeframes(b"".join(struct.pack("<h", s) for s in frames))
print("wrote public/music.wav", DUR, "sec")
