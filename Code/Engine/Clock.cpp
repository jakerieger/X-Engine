#include "Clock.hpp"

namespace x {
    Clock::Clock() {
        QueryPerformanceFrequency(&_frequency);
        QueryPerformanceCounter(&_lastTime);

        _deltaTime              = 0.0;
        _totalTime              = 0.0;
        _frameCount             = 0;
        _framesPerSecond        = 0.0;
        _fpsUpdateInterval      = 1.0; // In seconds
        _timeSinceLastFpsUpdate = 0.0;
    }

    void Clock::Tick() {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);

        _deltaTime = CAST<f64>(currentTime.QuadPart - _lastTime.QuadPart) / CAST<f64>(_frequency.QuadPart);
        _totalTime += _deltaTime;
        _frameCount++;
        _timeSinceLastFpsUpdate += _deltaTime;

        if (_timeSinceLastFpsUpdate >= _fpsUpdateInterval) {
            _framesPerSecond        = CAST<f64>(_frameCount) / _timeSinceLastFpsUpdate;
            _frameCount             = 0;
            _timeSinceLastFpsUpdate = 0.0;
        }

        _lastTime = currentTime;
    }

    f64 Clock::GetDeltaTime() const {
        return _deltaTime;
    }

    f64 Clock::GetTotalTime() const {
        return _totalTime;
    }

    f64 Clock::GetFramesPerSecond() const {
        return _framesPerSecond;
    }

    u64 Clock::GetRawCounter() const {
        LARGE_INTEGER current;
        QueryPerformanceCounter(&current);
        return current.QuadPart;
    }

    u64 Clock::GetCounterFrequency() const {
        return _frequency.QuadPart;
    }
}