#include "Globals.h"
#include "Clock.h"

#include <algorithm>

#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

Clock::Clock()
{
    timeBeginPeriod(1);
    clock.start();
}

Clock::~Clock()
{
    timeEndPeriod(1);
}

void Clock::endFrame()
{
    if (state.maxFps == 0)
        return;

    const uint64_t targetUs = 1000000ull / state.maxFps;
    const uint64_t spinUs = 2000;   // Margin left for spinning, to absorb Sleep overshoot

    for (;;)
    {
        uint64_t elapsedUs = clock.readUs() - lastUs;
        if (elapsedUs >= targetUs)
            break;

        uint64_t remainingUs = targetUs - elapsedUs;
        if (remainingUs > spinUs)
            Sleep(DWORD((remainingUs - spinUs) / 1000));
    }
}

void Clock::beginFrame()
{
    uint64_t nowUs = clock.readUs();

    state.frameUs = nowUs - lastUs;
    state.realDt = float(state.frameUs) * 1e-6f;
    state.totalUs = nowUs;
    lastUs = nowUs;

    // Game dt: follows the real time only while playing
    if (playing)
        state.dt = state.realDt;
    else if (stepRequested)
        state.dt = STEP_DT;
    else
        state.dt = 0.0f;

    stepRequested = false;
    state.gameTotalUs += uint64_t(state.dt * 1e6f);

    // Rolling average, so the FPS doesn't jitter from frame to frame
    sampleSum -= samples[sampleIndex];
    sampleSum += state.frameUs;
    samples[sampleIndex] = state.frameUs;
    sampleIndex = (sampleIndex + 1) % FPS_SAMPLES;

    // Until the window is full, average only the frames we actually have
    uint64_t count = std::min<uint64_t>(state.frameCount + 1, FPS_SAMPLES);
    state.fps = sampleSum > 0 ? 1e6f * float(count) / float(sampleSum) : 0.0f;

    ++state.frameCount;
}

void Clock::markInitDone(uint64_t initUs)
{
    state.initUs = initUs;
    lastUs = clock.readUs();
}
