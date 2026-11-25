#include "Globals.h"
#include "ModuleTime.h"

#include <algorithm>

ModuleTime::ModuleTime()
{
    clock.start();
}

void ModuleTime::beginFrame()
{
    uint64_t nowUs = clock.readUs();

    info.frameUs = nowUs - lastUs;
    info.dt = float(info.frameUs) * 1e-6f;
    info.totalUs = nowUs;
    lastUs = nowUs;

    // Rolling average, so the FPS doesn't jitter from frame to frame
    sampleSum -= samples[sampleIndex];
    sampleSum += info.frameUs;
    samples[sampleIndex] = info.frameUs;
    sampleIndex = (sampleIndex + 1) % FPS_SAMPLES;

    // Until the window is full, average only the frames we actually have
    uint64_t count = std::min<uint64_t>(info.frameCount + 1, FPS_SAMPLES);
    info.fps = sampleSum > 0 ? 1e6f * float(count) / float(sampleSum) : 0.0f;

    ++info.frameCount;
}

void ModuleTime::markInitDone(uint64_t initUs)
{
    info.initUs = initUs;
    lastUs = clock.readUs();
}
