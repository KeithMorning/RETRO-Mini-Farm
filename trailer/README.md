# Retro Mini Farm — Douyin trailer

Vertical 1080×1920 launch trailer made with Remotion.

## Preview

```bash
cd trailer
npx remotion studio --browser-executable='/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'
```

If the bundled Chrome download stalls, the installed Chrome path above avoids
the download.

## Assets

The generated media is intentionally not committed:

```bash
# 1. capture the simulator gameplay reel
cd native
cc -O1 -std=gnu11 -Isrc -o sim/record_demo sim/record_demo.c \
  build/host/game.o build/host/font8.o build/host/save_stub.o build/host/sound_stub.o
(cd sim && ./record_demo)
./sim/encode_video sim/out/demo.rgb ../dist/gameplay.mp4

# 2. copy it and generate the music
cd ../trailer
cp ../dist/gameplay.mp4 public/gameplay.mp4
python3 scripts/make_music.py
```

## Render

```bash
npx remotion render RetroMiniFarmTrailer out/retro-mini-farm-douyin.mp4 \
  --browser-executable='/Applications/Google Chrome.app/Contents/MacOS/Google Chrome' \
  --codec=h264 --crf=18 --concurrency=4
```

The final MP4 is 1080×1920, 30 fps, 788 frames (26.27 s), with AAC audio.
