#include "Clock.hpp"

namespace x {
    Clock::Clock() {
        QueryPerformanceFrequency(&mFrequency);
        QueryPerformanceCounter(&mLastTime);

        mDeltaTime              = 0.0;
        mTotalTime              = 0.0;
        mFrameCount             = 0;
        mFramesPerSecond        = 0.0;
        mFpsUpdateInterval      = 0.5;  // In seconds
        mTimeSinceLastFpsUpdate = 0.0;
    }

    void Clock::Tick() {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);

        mDeltaTime = CAST<f64>(currentTime.QuadPart - mLastTime.QuadPart) / CAST<f64>(mFrequency.QuadPart);
        mTotalTime += mDeltaTime;
        mFrameCount++;
        mTimeSinceLastFpsUpdate += mDeltaTime;

        if (mTimeSinceLastFpsUpdate >= mFpsUpdateInterval) {
            mFramesPerSecond        = CAST<f64>(mFrameCount) / mTimeSinceLastFpsUpdate;
            mFrameCount             = 0;
            mTimeSinceLastFpsUpdate = 0.0;
        }

        mLastTime = currentTime;
    }

    f64 Clock::GetDeltaTimePrecise() const {
        return mDeltaTime;
    }

    f32 Clock::GetDeltaTime() const {
        return CAST<f32>(mDeltaTime);
    }

    f64 Clock::GetTotalTime() const {
        return mTotalTime;
    }

    f64 Clock::GetFramesPerSecond() const {
        return mFramesPerSecond;
    }

    u64 Clock::GetRawCounter() const {
        LARGE_INTEGER current;
        QueryPerformanceCounter(&current);
        return current.QuadPart;
    }

    u64 Clock::GetCounterFrequency() const {
        return mFrequency.QuadPart;
    }
}  // namespace x