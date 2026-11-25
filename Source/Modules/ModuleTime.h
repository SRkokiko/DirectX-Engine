#pragma once

#include "Module.h"
#include "Timer.h"

#include <array>

/** @brief Everything ModuleTime measures, in one place so a UI can show it without knowing how it is computed. */
struct TimeInfo
{
    float    dt = 0.0f;         // Last frame's duration in seconds
    uint64_t frameUs = 0;       // Last frame's duration in microseconds
    float    fps = 0.0f;        // Average over the last FPS_SAMPLES frames
    uint64_t totalUs = 0;       // Time since the module was created, in microseconds
    uint64_t frameCount = 0;
    uint64_t initUs = 0;        // How long Application::init took, in microseconds
};

/**
 * @brief Owns the engine clock and the frame timing: dt, FPS, total time and init time.
 *
 * Register it first in Application so nothing runs before it. Application::update calls beginFrame() before
 * the other modules' update, and Application::init reports the init duration through markInitDone().
 *
 * dt is read-only for everyone else: use getDeltaTime() (or getInfo()).
 */
class ModuleTime : public Module
{
public:

    /** @brief Starts the engine clock. */
    ModuleTime();

    /** @brief Measures the time since the previous call and updates dt, FPS and the counters. */
    void beginFrame();

    /**
     * @brief Records how long Application::init took and restarts the frame clock,
     * so the first frame's dt doesn't include the init time.
     */
    void markInitDone(uint64_t initUs);

    /** @brief Duration of the last frame in seconds. */
    float getDeltaTime() const { return info.dt; }

    const TimeInfo& getInfo() const { return info; }

private:
    enum { FPS_SAMPLES = 30 };

    Timer clock;                // Runs for the whole life of the engine
    uint64_t lastUs = 0;        // clock value at the previous beginFrame()

    std::array<uint64_t, FPS_SAMPLES> samples = {};
    uint64_t sampleSum = 0;
    uint64_t sampleIndex = 0;

    TimeInfo info;
};
