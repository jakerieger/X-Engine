// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include <chrono>
#include <iomanip>
#include <sstream>

class DateTime {
public:
    using Timepoint = std::chrono::time_point<std::chrono::system_clock>;

    static DateTime Now() {
        return DateTime(std::chrono::system_clock::now());
    }

    // Should return a string in the following format:
    // YYYY-MM-DD HH:MM:SS AM/PM
    [[nodiscard]] std::string UTCString() const {
        const auto time_t = std::chrono::system_clock::to_time_t(_time);
        std::tm utcTm {};
        gmtime_s(&utcTm, &time_t);
        return FormatDateTimeString(utcTm);
    }

    // Should return a string in the following format:
    // YYYY-MM-DD HH:MM:SS AM/PM
    [[nodiscard]] std::string LocalString() const {
        const auto time_t = std::chrono::system_clock::to_time_t(_time);
        std::tm localTm {};
        localtime_s(&localTm, &time_t);
        return FormatDateTimeString(localTm);
    }

    // Should return a string in the following format:
    // YYYY-MM-DD
    [[nodiscard]] std::string DateString() const {
        const auto time_t = std::chrono::system_clock::to_time_t(_time);
        std::tm dateTm {};
        localtime_s(&dateTm, &time_t);
        std::ostringstream oss;
        oss << std::put_time(&dateTm, "%Y-%m-%d");
        return oss.str();
    }

    // Should return a string in the following format:
    // HH:MM:SS AM/PM
    [[nodiscard]] std::string TimeString() const {
        const auto time_t = std::chrono::system_clock::to_time_t(_time);
        std::tm timeTm {};
        localtime_s(&timeTm, &time_t);
        return FormatTimeString(timeTm);
    }

private:
    DateTime() {
        _time = std::chrono::system_clock::now();
    }

    explicit DateTime(Timepoint time) : _time(time) {}

    [[nodiscard]] static std::string FormatDateTimeString(const std::tm& tm) {
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d");
        oss << " ";
        oss << FormatTimeString(tm);
        return oss.str();
    }

    [[nodiscard]] static std::string FormatTimeString(const std::tm& tm) {
        std::ostringstream oss;

        int hour        = tm.tm_hour;
        const bool isPM = tm.tm_hour >= 12;
        hour            = hour % 12;
        if (hour == 0) hour = 12;

        oss << std::setfill('0') << std::setw(2) << hour << ":" << std::setw(2) << tm.tm_min << ":" << std::setw(2)
            << tm.tm_sec << (isPM ? " PM" : " AM");

        return oss.str();
    }

    Timepoint _time;
};
