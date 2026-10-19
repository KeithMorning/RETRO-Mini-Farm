import React from "react";
import {
  AbsoluteFill,
  Audio,
  OffthreadVideo,
  interpolate,
  spring,
  staticFile,
  useCurrentFrame,
  useVideoConfig,
} from "remotion";

const CREAM = "#f5e8c6";
const GOLD = "#e9b765";
const DEEP = "#0d1710";

const titleFont =
  '"Arial Black", "Helvetica Neue", "PingFang SC", "Hiragino Sans GB", sans-serif';
const cnFont =
  '"PingFang SC", "Hiragino Sans GB", "Microsoft YaHei", sans-serif';
const monoFont = '"Courier New", "SFMono-Regular", monospace';

const scenes = [
  { from: 0, to: 75, zh: "一个小农场。", en: "a little farm." },
  { from: 75, to: 210, zh: "从清晨开始。", en: "wake up." },
  { from: 210, to: 260, zh: "种下第一颗种子。", en: "grow." },
  { from: 260, to: 327, zh: "去小镇打个招呼。", en: "say hello." },
  { from: 327, to: 384, zh: "把一年装进背包。", en: "carry it with you." },
  { from: 384, to: 477, zh: "去海边钓鱼。", en: "cast a line." },
  { from: 477, to: 534, zh: "下到矿洞第二层。", en: "go deeper." },
  { from: 534, to: 585, zh: "交三个朋友。", en: "make friends." },
  { from: 585, to: 630, zh: "第一年，开始了。", en: "year one." },
];

const captionStyle: React.CSSProperties = {
  position: "absolute",
  left: 80,
  right: 80,
  top: 1315,
  textAlign: "center",
};

const SceneCaption: React.FC = () => {
  const frame = useCurrentFrame();
  const scene = scenes.find((s) => frame >= s.from && frame < s.to) ?? scenes[0];
  const local = frame - scene.from;
  const len = scene.to - scene.from;
  const opacity =
    interpolate(local, [0, 10, len - 12, len], [0, 1, 1, 0], {
      extrapolateLeft: "clamp",
      extrapolateRight: "clamp",
    }) * interpolate(frame, [620, 635], [1, 0], {
      extrapolateLeft: "clamp",
      extrapolateRight: "clamp",
    });
  const y = interpolate(local, [0, 14], [24, 0], {
    extrapolateLeft: "clamp",
    extrapolateRight: "clamp",
  });

  return (
    <div style={{ ...captionStyle, opacity, translate: `0 ${y}px` }}>
      <div
        style={{
          color: CREAM,
          fontFamily: cnFont,
          fontSize: 64,
          fontWeight: 800,
          letterSpacing: 2,
          textShadow: "0 4px 24px rgba(0,0,0,.5)",
        }}
      >
        {scene.zh}
      </div>
      <div
        style={{
          color: GOLD,
          fontFamily: monoFont,
          fontSize: 31,
          letterSpacing: 4,
          marginTop: 10,
          textTransform: "lowercase",
        }}
      >
        {scene.en}
      </div>
    </div>
  );
};

const EndCard: React.FC = () => {
  const frame = useCurrentFrame();
  const { fps } = useVideoConfig();
  const opacity = interpolate(frame, [620, 640], [0, 1], {
    extrapolateLeft: "clamp",
    extrapolateRight: "clamp",
  });
  const scale = spring({
    frame: frame - 620,
    fps,
    config: { damping: 14, stiffness: 90 },
  });
  return (
    <div
      style={{
        position: "absolute",
        left: 40,
        right: 40,
        top: 1280,
        height: 500,
        opacity,
        display: "flex",
        flexDirection: "column",
        alignItems: "center",
        justifyContent: "flex-start",
        paddingTop: 42,
        background:
          "linear-gradient(180deg, rgba(9,17,11,.15), rgba(9,17,11,.92))",
        borderRadius: 28,
      }}
    >
      <div
        style={{
          color: GOLD,
          fontFamily: monoFont,
          fontSize: 28,
          letterSpacing: 8,
          marginBottom: 20,
        }}
      >
        AVAILABLE NOW
      </div>
      <div
        style={{
          color: CREAM,
          fontFamily: titleFont,
          fontSize: 92,
          lineHeight: 0.95,
          textAlign: "center",
          letterSpacing: 2,
          scale,
          textShadow: "0 8px 30px rgba(0,0,0,.6)",
        }}
      >
        RETRO
        <br />
        MINI FARM
      </div>
      <div
        style={{
          marginTop: 30,
          color: "#bcd1b7",
          fontFamily: cnFont,
          fontSize: 34,
          letterSpacing: 2,
        }}
      >
        免费下载 · 链接在简介
      </div>
      <div
        style={{
          marginTop: 12,
          color: "#8eaa8b",
          fontFamily: monoFont,
          fontSize: 22,
          letterSpacing: 2,
        }}
      >
        GITHUB RELEASES
      </div>
    </div>
  );
};

const Background: React.FC = () => {
  const frame = useCurrentFrame();
  const glow = interpolate(frame, [0, 668], [0.35, 0.7], {
    extrapolateRight: "clamp",
  });
  return (
    <AbsoluteFill
      style={{
        background:
          "radial-gradient(ellipse at 50% 24%, #365a3b 0%, #1a2c1e 42%, #0a120c 100%)",
        overflow: "hidden",
      }}
    >
      <div
        style={{
          position: "absolute",
          width: 760,
          height: 760,
          borderRadius: "50%",
          left: 160,
          top: -300,
          background: "rgba(233,183,101,.16)",
          filter: "blur(74px)",
          opacity: glow,
        }}
      />
      {[
        [120, 260], [930, 220], [180, 500], [860, 470], [540, 120],
        [90, 850], [990, 760], [760, 920], [280, 960],
      ].map(([x, y], i) => (
        <div
          key={i}
          style={{
            position: "absolute",
            left: x,
            top: y,
            width: i % 2 ? 5 : 7,
            height: i % 2 ? 5 : 7,
            borderRadius: "50%",
            background: "#f4dfaa",
            opacity: 0.35 + ((i * 7) % 30) / 100,
          }}
        />
      ))}
      <div
        style={{
          position: "absolute",
          left: -100,
          right: -100,
          bottom: -110,
          height: 420,
          background:
            "linear-gradient(180deg, transparent, rgba(8,15,10,.75) 55%, #08100a)",
        }}
      />
      <div
        style={{
          position: "absolute",
          left: 0,
          right: 0,
          bottom: 0,
          height: 5,
          background: `linear-gradient(90deg, #6c4b28, ${GOLD}, #6c4b28)`,
          opacity: 0.6,
        }}
      />
    </AbsoluteFill>
  );
};

export const Trailer: React.FC = () => {
  const frame = useCurrentFrame();
  const { fps } = useVideoConfig();
  const titleIn = spring({
    frame: frame - 8,
    fps,
    config: { damping: 15, stiffness: 80 },
  });
  const titleOpacity = interpolate(frame, [8, 28], [0, 1], {
    extrapolateLeft: "clamp",
    extrapolateRight: "clamp",
  });

  return (
    <AbsoluteFill style={{ backgroundColor: DEEP, color: CREAM }}>
      <Background />
      <Audio src={staticFile("music.wav")} volume={0.42} />
      <OffthreadVideo
        src={staticFile("gameplay.mp4")}
        muted
        style={{
          position: "absolute",
          left: 52,
          top: 528,
          width: 976,
          height: 736,
          borderRadius: 26,
          border: "8px solid #d8af68",
          boxShadow: "0 22px 70px rgba(0,0,0,.6), 0 0 0 2px rgba(255,240,200,.12)",
          imageRendering: "pixelated",
          background: "#0b120c",
        }}
      />
      <div
        style={{
          position: "absolute",
          left: 60,
          top: 536,
          width: 960,
          height: 720,
          borderRadius: 20,
          pointerEvents: "none",
          boxShadow: "inset 0 0 100px rgba(0,0,0,.35)",
        }}
      />
      <div
        style={{
          position: "absolute",
          left: 60,
          top: 536,
          width: 960,
          height: 720,
          borderRadius: 20,
          pointerEvents: "none",
          background:
            "repeating-linear-gradient(0deg, rgba(0,0,0,.10) 0px, rgba(0,0,0,.10) 2px, transparent 2px, transparent 5px)",
          opacity: 0.22,
        }}
      />

      <div style={{ position: "absolute", left: 80, right: 80, top: 96, textAlign: "center" }}>
        <div
          style={{
            color: GOLD,
            fontFamily: monoFont,
            fontSize: 24,
            letterSpacing: 7,
            opacity: 0.95,
          }}
        >
          ELECFREAKS RETRO ARCADE
        </div>
        <div
          style={{
            color: "#b7cdb1",
            fontFamily: cnFont,
            fontSize: 28,
            letterSpacing: 4,
            marginTop: 12,
            opacity: 0.9,
          }}
        >
          一台掌机 · 一小块地 · 一整年
        </div>
      </div>

      <div
        style={{
          position: "absolute",
          left: 40,
          right: 40,
          top: 198,
          textAlign: "center",
          opacity: titleOpacity,
          scale: titleIn,
        }}
      >
        <div
          style={{
            color: "#d5e4cf",
            fontFamily: monoFont,
            fontSize: 34,
            letterSpacing: 12,
            marginBottom: 2,
          }}
        >
          RETRO
        </div>
        <div
          style={{
            color: CREAM,
            fontFamily: titleFont,
            fontSize: 138,
            lineHeight: 0.92,
            letterSpacing: 2,
            textShadow: "0 10px 50px rgba(0,0,0,.45)",
          }}
        >
          MINI FARM
        </div>
        <div
          style={{
            color: GOLD,
            fontFamily: cnFont,
            fontSize: 48,
            fontWeight: 700,
            letterSpacing: 10,
            marginTop: 14,
          }}
        >
          迷你农场
        </div>
      </div>

      <SceneCaption />
      <EndCard />

      <div
        style={{
          position: "absolute",
          left: 80,
          right: 80,
          bottom: 92,
          display: "flex",
          justifyContent: "center",
          gap: 26,
          color: "#91ab8e",
          fontFamily: monoFont,
          fontSize: 20,
          letterSpacing: 2,
          opacity: interpolate(frame, [0, 20], [0, 1], {
            extrapolateLeft: "clamp",
            extrapolateRight: "clamp",
          }),
        }}
      >
        <span>16 CROPS</span>
        <span>·</span>
        <span>3 STORIES</span>
        <span>·</span>
        <span>2 MINE FLOORS</span>
        <span>·</span>
        <span>1 LITTLE LIFE</span>
      </div>
    </AbsoluteFill>
  );
};
