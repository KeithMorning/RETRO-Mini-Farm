import React from "react";
import { Composition } from "remotion";
import { Trailer } from "./Trailer";

export const RemotionRoot: React.FC = () => {
  return (
    <Composition
      id="RetroMiniFarmTrailer"
      component={Trailer}
      durationInFrames={668}
      fps={30}
      width={1080}
      height={1920}
    />
  );
};
