#pragma once

#include <chrono>
#include <iostream>

namespace x {
    class Timer {
        std::chrono::time_point<std::chrono::high_resolution_clock> _start;

    public:
        Timer() { Reset(); }

        void Reset() { _start = std::chrono::high_resolution_clock::now(); }

        f32 Elapsed() const {
            return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _start).count() * 0.001f * 0.001f;
        }

        f32 ElapsedMillis() const {
            return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _start).count() * 0.001f;
        }
    };

    class ScopedTimer {
        Timer _timer;
        std::string _name;

    public:
        ScopedTimer(std::string_view name) : _name(name) {}

        ~ScopedTimer() {
            f32 time = _timer.ElapsedMillis();
            std::cout << _name << ": " << time << " ms\n";
        }
    };
}  // namespace x