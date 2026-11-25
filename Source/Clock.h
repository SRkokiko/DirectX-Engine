#pragma once

#include "Timer.h"

#include <array>

/**
 * @brief All of Clock's timing state. `dt` is what drives the simulation, so Clock is the only one that
 * writes it; everyone else reads it through getDeltaTime() or the const getState() (the Timers view does the latter).
 */
struct TimeState
{
    float    dt = 0.0f;         // Game dt in seconds: the real frame time while playing, 0 while stopped, STEP_DT on a step
    float    realDt = 0.0f;     // Last frame's real duration in seconds, whatever the play state
    uint64_t frameUs = 0;       // Last frame's real duration in microseconds
    float    fps = 0.0f;        // Average over the last FPS_SAMPLES frames
    uint64_t totalUs = 0;       // Real time since the module was created, in microseconds
    uint64_t gameTotalUs = 0;   // Sum of the game dt's: it stops advancing while stopped
    uint64_t frameCount = 0;
    uint64_t initUs = 0;        // How long Application::init took, in microseconds
    uint32_t maxFps = 0;        // Frame rate cap; 0 means unlimited
};

/**
 * @brief Owns the engine clock and the frame timing: dt, FPS, total time and init time.
 *
 * Not a Module: Application owns it as a member and drives it explicitly. Application::update calls beginFrame() before
 * the modules' update, and Application::init reports the init duration through markInitDone().
 *
 * dt is read-only for everyone else: use getDeltaTime() (or getState()).
 */
class Clock
{
public:

    /** @brief Starts the engine clock and asks Windows for 1 ms timer resolution, so the limiter can sleep accurately. */
    Clock();
    ~Clock();

    /** @brief Measures the time since the previous call and updates dt, FPS and the counters. */
    void beginFrame();

    /**
     * @brief Waits until the frame has lasted at least 1/maxFps seconds since beginFrame(). Does nothing if maxFps is 0.
     * Sleeps for most of the wait and spins for the last stretch, because Sleep is too coarse to hit the target alone.
     * Note: ModuleD3D12 presents with vsync, so a cap above the monitor's refresh rate has no visible effect.
     */
    void endFrame();

    /** @brief Caps the frame rate. 0 removes the cap. */
    void setMaxFps(uint32_t fps) { state.maxFps = fps; }

    /**
     * @brief Records how long Application::init took and restarts the frame clock,
     * so the first frame's dt doesn't include the init time.
     */
    void markInitDone(uint64_t initUs);

    /**
     * @brief Game dt in seconds: what every module's update() receives.
     * It is the last frame's duration while playing, 0 while stopped, and STEP_DT for the frame of a step().
     */
    float getDeltaTime() const { return state.dt; }

    /** @brief Resumes the simulation. */
    void play() { playing = true; stepRequested = false; }

    /** @brief Freezes the simulation: dt is 0, but rendering and the UI keep running. */
    void stop() { playing = false; }

    /** @brief While stopped, makes the next frame's dt STEP_DT, then stops again. Ignored while playing. */
    void step() { if (!playing) stepRequested = true; }

    bool isPlaying() const { return playing; }

    const TimeState& getState() const { return state; }

    static constexpr float STEP_DT = 1.0f / 60.0f;  // dt of a single step, in seconds

private:
    enum { FPS_SAMPLES = 30 };

    bool playing = true;
    bool stepRequested = false;

    Timer clock;                // Runs for the whole life of the engine
    uint64_t lastUs = 0;        // clock value at the previous beginFrame()

    std::array<uint64_t, FPS_SAMPLES> samples = {};
    uint64_t sampleSum = 0;
    uint64_t sampleIndex = 0;

    TimeState state;
};
