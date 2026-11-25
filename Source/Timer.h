#pragma once

#include <chrono>
#include <cstdint>

/**
 * @brief Stopwatch with microsecond precision, built on the monotonic std::chrono::steady_clock.
 *
 * Microseconds are the base unit; milliseconds are derived from them, so no precision is lost to an early cast.
 * After stop() the value is frozen, and start() restarts from zero.
 */
class Timer
{
public:

    /** @brief Starts (or restarts) the timer from zero. */
    void start()
    {
        startTime = Clock::now();
        elapsedUs = 0;
        running = true;
    }

    /** @brief Freezes the timer and returns the elapsed microseconds. Does nothing if it isn't running. */
    uint64_t stop()
    {
        elapsedUs = readUs();
        running = false;
        return elapsedUs;
    }

    /** @brief Microseconds since start(), or the frozen value if stopped. 0 if it was never started. */
    uint64_t readUs() const
    {
        if (!running)
            return elapsedUs;

        return uint64_t(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - startTime).count());
    }

    /** @brief Whole milliseconds since start(), or the frozen value if stopped. */
    uint64_t readMs() const { return readUs() / 1000; }

    bool isRunning() const { return running; }

private:
    using Clock = std::chrono::steady_clock;

    Clock::time_point startTime;
    uint64_t          elapsedUs = 0;
    bool              running = false;
};
