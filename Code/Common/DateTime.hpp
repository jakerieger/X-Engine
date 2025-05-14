// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "Typedefs.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace x {
    class DateTime {
    public:
        using Timepoint = std::chrono::time_point<std::chrono::system_clock>;

        explicit DateTime(Timepoint time) : mTime(time) {}

        static DateTime Now() {
            return DateTime(std::chrono::system_clock::now());
        }

        // Should return a string in the following format:
        // YYYY-MM-DD HH:MM:SS AM/PM
        [[nodiscard]] str UTCString() const {
            const auto time_t = std::chrono::system_clock::to_time_t(mTime);
            std::tm utcTm {};
            gmtime_s(&utcTm, &time_t);
            return FormatDateTimeString(utcTm);
        }

        // Should return a string in the following format:
        // YYYY-MM-DD HH:MM:SS AM/PM
        [[nodiscard]] str LocalString() const {
            const auto time_t = std::chrono::system_clock::to_time_t(mTime);
            std::tm localTm {};
            localtime_s(&localTm, &time_t);
            return FormatDateTimeString(localTm);
        }

        // Should return a string in the following format:
        // YYYY-MM-DD
        [[nodiscard]] str DateString() const {
            const auto time_t = std::chrono::system_clock::to_time_t(mTime);
            std::tm dateTm {};
            localtime_s(&dateTm, &time_t);
            std::ostringstream oss;
            oss << std::put_time(&dateTm, "%Y-%m-%d");
            return oss.str();
        }

        // Should return a string in the following format:
        // HH:MM:SS AM/PM
        [[nodiscard]] str TimeString() const {
            const auto time_t = std::chrono::system_clock::to_time_t(mTime);
            std::tm timeTm {};
            localtime_s(&timeTm, &time_t);
            return FormatTimeString(timeTm);
        }

    private:
        DateTime() {
            mTime = std::chrono::system_clock::now();
        }

        [[nodiscard]] static str FormatDateTimeString(const std::tm& tm) {
            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y-%m-%d");
            oss << " ";
            oss << FormatTimeString(tm);
            return oss.str();
        }

        [[nodiscard]] static str FormatTimeString(const std::tm& tm) {
            std::ostringstream oss;

            i32 hour        = tm.tm_hour;
            const bool isPM = tm.tm_hour >= 12;
            hour            = hour % 12;
            if (hour == 0) hour = 12;

            oss << std::setfill('0') << std::setw(2) << hour << ":" << std::setw(2) << tm.tm_min << ":" << std::setw(2)
                << tm.tm_sec << (isPM ? " PM" : " AM");

            return oss.str();
        }

        Timepoint mTime;
    };

}  // namespace x