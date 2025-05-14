#pragma once

#include "Common/Platform.hpp"
#include "Common/Typedefs.hpp"

namespace x {
    class Clock {
        LARGE_INTEGER mFrequency;
        LARGE_INTEGER mLastTime;
        f64 mDeltaTime;
        f64 mTotalTime;
        f64 mFramesPerSecond;
        f64 mFpsUpdateInterval;
        f64 mTimeSinceLastFpsUpdate;
        u32 mFrameCount;

    public:
        Clock();
        void Tick();

        f64 GetDeltaTimePrecise() const;
        f32 GetDeltaTime() const;
        f64 GetTotalTime() const;
        f64 GetFramesPerSecond() const;
        u64 GetRawCounter() const;
        u64 GetCounterFrequency() const;
    };
}  // namespace x