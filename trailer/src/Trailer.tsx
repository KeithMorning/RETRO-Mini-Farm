import React from "react";
import {
  AbsoluteFill,
  Audio,
  Img,
  OffthreadVideo,
  Sequence,
  interpolate,
  spring,
  staticFile,
  useCurrentFrame,
  useVideoConfig,
} from "remotion";

const CREAM = "#f5e8c6";
const GOLD = "#e9b765";
const DEEP = "#0d1710";
const INTRO = 120; // 4 seconds of device introduction

const titleFont =
  '"Arial Black", "Helvetica Neue", "PingFang SC", "Hiragino Sans GB", sans-serif';
const cnFont =
  '"PingFang SC", "Hiragino Sans GB", "Microsoft YaHei", sans-serif';
const monoFont = '"Courier New", "SFMono-Regular", monospace';

const scenes = [
  { from: 0, to: 75, zh: "一个小农场。", en: "a little farm." },
  { from: 75, to: 210, zh: "从清晨开始。", en: "wake up." },
  { from: 210, to: 282, zh: "16 种作物，亲手种下。", en: "16 crops. grow." },
  { from: 282, to: 384, zh: "去海边，抛一竿。", en: "cast a line." },
  { from: 384, to: 484, zh: "去认识三个朋友。", en: "three friends." },
  { from: 484, to: 584, zh: "小猫、小鸡、小恐龙。", en: "little friends." },
  { from: 584, to: 703, zh: "下到矿洞第二层。", en: "go deeper." },
  { from: 703, to: 750, zh: "第一年，开始了。", en: "year one." },
  { from: 750, to: 783, zh: "一个小农场，一整年。", en: "one little year." },
];

const Background: React.FC = () => {
  const frame = useCurrentFrame();
  const glow = interpolate(frame, [0, 903], [0.35, 0.7], {
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

const DeviceIntro: React.FC = () => {
  const frame = useCurrentFrame();
  const fade = interpolate(frame, [0, 10, 104, 120], [0, 1, 1, 0], {
    extrapolateLeft: "clamp",
    extrapolateRight: "clamp",
  });
  const textY = interpolate(frame, [8, 28], [26, 0], {
    extrapolateLeft: "clamp",
    extrapolateRight: "clamp",
  });

  return (
    <AbsoluteFill style={{ opacity: fade, zIndex: 20 }}>
      <div style={{ position: "absolute", left: 80, right: 80, top: 142, textAlign: "center" }}>
        <div style={{ color: GOLD, fontFamily: monoFont, fontSize: 24, letterSpacing: 8 }}>
          ELECFREAKS
        </div>
        <div
          style={{
            color: CREAM,
            fontFamily: titleFont,
            fontSize: 58,
            letterSpacing: 4,
            marginTop: 8,
          }}
        >
          RETRO ARCADE
        </div>
      </div>

      <div
        style={{
          position: "absolute",
          left: 270,
          top: 380,
          width: 540,
          height: 810,
          borderRadius: 82,
          background:
            "linear-gradient(145deg, #4be0ed 0%, #18c4d7 40%, #0aa5bb 100%)",
          border: "6px solid #087f92",
          boxShadow:
            "0 34px 90px rgba(0,0,0,.55), inset 0 3px 0 rgba(255,255,255,.55), inset 0 -10px 30px rgba(0,70,80,.35)",
          scale: 1,
          rotate: "0deg",
        }}
      >
        <div
          style={{
            position: "absolute",
            left: 42,
            top: 42,
            width: 456,
            height: 344,
            background: "#161a25",
            borderRadius: 20,
            padding: 14,
            boxShadow: "inset 0 0 34px rgba(0,0,0,.85)",
          }}
        >
          <div style={{ width: "100%", height: "100%", overflow: "hidden", borderRadius: 10 }}>
            <Img
              src={staticFile("farm.png")}
              style={{
                width: "100%",
                height: "100%",
                objectFit: "cover",
                imageRendering: "pixelated",
              }}
            />
          </div>
        </div>

        <div style={{ position: "absolute", left: 6, top: 8, width: 10, height: 10, borderRadius: "50%", background: "#d8f6f7", boxShadow: "0 0 8px #b9ffff" }} />

        <div style={{ position: "absolute", left: 76, top: 498, width: 150, height: 150 }}>
          <div style={{ position: "absolute", left: 58, top: 0, width: 36, height: 150, borderRadius: 10, background: "#f7f4ea", boxShadow: "inset 0 -4px 0 #d8d3c4, 0 5px 0 #0d8496" }} />
          <div style={{ position: "absolute", left: 0, top: 58, width: 150, height: 36, borderRadius: 10, background: "#f7f4ea", boxShadow: "inset 0 -4px 0 #d8d3c4, 0 5px 0 #0d8496" }} />
          <div style={{ position: "absolute", left: 68, top: 68, width: 16, height: 16, borderRadius: 4, background: "#d9d4c5" }} />
        </div>

        <div style={{ position: "absolute", left: 244, top: 486, width: 108, height: 30, borderRadius: 15, background: "#ece9df", boxShadow: "inset 0 -3px 0 #c9c4b6" }} />
        <div
          style={{
            position: "absolute",
            left: 266,
            top: 493,
            color: "#9b978b",
            fontFamily: monoFont,
            fontSize: 12,
            letterSpacing: 1,
          }}
        >
          menu
        </div>
        <div
          style={{
            position: "absolute",
            left: 370,
            top: 488,
            width: 28,
            height: 28,
            borderRadius: "50%",
            background: "#ece9df",
            boxShadow: "inset 0 -3px 0 #c9c4b6",
          }}
        />

        <div
          style={{
            position: "absolute",
            left: 382,
            top: 538,
            width: 82,
            height: 82,
            borderRadius: "50%",
            background: "#f8f5eb",
            border: "4px solid #dad4c6",
            boxShadow: "0 5px 0 #0d8496",
            color: "#138fa3",
            fontFamily: titleFont,
            fontSize: 36,
            lineHeight: "74px",
            textAlign: "center",
          }}
        >
          A
        </div>
        <div
          style={{
            position: "absolute",
            left: 292,
            top: 590,
            width: 82,
            height: 82,
            borderRadius: "50%",
            background: "#f8f5eb",
            border: "4px solid #dad4c6",
            boxShadow: "0 5px 0 #0d8496",
            color: "#138fa3",
            fontFamily: titleFont,
            fontSize: 36,
            lineHeight: "74px",
            textAlign: "center",
          }}
        >
          B
        </div>

        <div
          style={{
            position: "absolute",
            left: 214,
            top: 724,
            color: "rgba(0,92,104,.55)",
            fontFamily: monoFont,
            fontSize: 13,
            letterSpacing: 4,
          }}
        >
          RETRO ARCADE
        </div>
      </div>

      <div
        style={{
          position: "absolute",
          left: 70,
          right: 70,
          top: 1240,
          textAlign: "center",
          opacity: fade,
          translate: `0 ${textY}px`,
        }}
      >
        <div style={{ color: CREAM, fontFamily: cnFont, fontSize: 54, fontWeight: 800, letterSpacing: 2 }}>
          一台真正的掌上游戏机
        </div>
        <div style={{ color: GOLD, fontFamily: cnFont, fontSize: 32, marginTop: 12 }}>
          不是手机游戏 · 真实按键 · 2.4 英寸屏
        </div>
        <div
          style={{
            marginTop: 22,
            color: "#a9c2a4",
            fontFamily: monoFont,
            fontSize: 23,
            letterSpacing: 3,
          }}
        >
          A REAL HANDHELD · REAL BUTTONS · POCKET SIZE
        </div>
      </div>
    </AbsoluteFill>
  );
};

const SceneCaption: React.FC<{ frame: number }> = ({ frame }) => {
  const scene = scenes.find((s) => frame >= s.from && frame < s.to) ?? scenes[0];
  const local = frame - scene.from;
  const len = scene.to - scene.from;
  const opacity =
    interpolate(local, [0, 10, len - 12, len], [0, 1, 1, 0], {
      extrapolateLeft: "clamp",
      extrapolateRight: "clamp",
    }) * interpolate(frame, [735, 748], [1, 0], {
      extrapolateLeft: "clamp",
      extrapolateRight: "clamp",
    });
  const y = interpolate(local, [0, 14], [24, 0], {
    extrapolateLeft: "clamp",
    extrapolateRight: "clamp",
  });

  return (
    <div
      style={{
        position: "absolute",
        left: 80,
        right: 80,
        top: 1315,
        textAlign: "center",
        opacity,
        translate: `0 ${y}px`,
      }}
    >
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
        }}
      >
        {scene.en}
      </div>
    </div>
  );
};

const EndCard: React.FC<{ frame: number }> = ({ frame }) => {
  const { fps } = useVideoConfig();
  const opacity = interpolate(frame, [750, 770], [0, 1], {
    extrapolateLeft: "clamp",
    extrapolateRight: "clamp",
  });
  const scale = spring({
    frame: frame - 750,
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

export const Trailer: React.FC = () => {
  const frame = useCurrentFrame();
  const { fps } = useVideoConfig();
  const main = frame - INTRO;
  const titleIn = spring({
    frame: Math.max(0, main - 8),
    fps,
    config: { damping: 15, stiffness: 80 },
  });
  const titleOpacity = interpolate(main, [8, 28], [0, 1], {
    extrapolateLeft: "clamp",
    extrapolateRight: "clamp",
  });

  return (
    <AbsoluteFill style={{ backgroundColor: DEEP, color: CREAM }}>
      <Background />
      <Audio src={staticFile("music.wav")} volume={0.44} />
      <DeviceIntro />

      <Sequence from={INTRO}>
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
            boxShadow:
              "0 22px 70px rgba(0,0,0,.6), 0 0 0 2px rgba(255,240,200,.12)",
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
      </Sequence>

      <div
        style={{
          position: "absolute",
          left: 80,
          right: 80,
          top: 96,
          textAlign: "center",
          opacity: titleOpacity,
        }}
      >
        <div style={{ color: GOLD, fontFamily: monoFont, fontSize: 24, letterSpacing: 7 }}>
          ELECFREAKS RETRO ARCADE
        </div>
        <div
          style={{
            color: "#b7cdb1",
            fontFamily: cnFont,
            fontSize: 28,
            letterSpacing: 4,
            marginTop: 12,
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
        <div style={{ color: "#d5e4cf", fontFamily: monoFont, fontSize: 34, letterSpacing: 12 }}>
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

      <SceneCaption frame={Math.max(0, main)} />
      <EndCard frame={Math.max(0, main)} />

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
          opacity: titleOpacity,
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

      {/* Opaque lead-in: hides any first-frame browser/encoder artifacts. */}
      <AbsoluteFill
        style={{
          background: "#050b07",
          zIndex: 99,
          pointerEvents: "none",
          opacity: interpolate(frame, [0, 8, 16], [1, 1, 0], {
            extrapolateLeft: "clamp",
            extrapolateRight: "clamp",
          }),
        }}
      />
    </AbsoluteFill>
  );
};
