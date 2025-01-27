#pragma once

#include "Platform.hpp"
#include "Common/Types.hpp"

namespace x {
    class Clock {
        LARGE_INTEGER _frequency;
        LARGE_INTEGER _lastTime;
        f64 _deltaTime;
        f64 _totalTime;
        f64 _framesPerSecond;
        f64 _fpsUpdateInterval;
        f64 _timeSinceLastFpsUpdate;
        u32 _frameCount;

    public:
        Clock();
        void Tick();

        f64 GetDeltaTime() const;
        f64 GetTotalTime() const;
        f64 GetFramesPerSecond() const;
        u64 GetRawCounter() const;
        u64 GetCounterFrequency() const;
    };
}