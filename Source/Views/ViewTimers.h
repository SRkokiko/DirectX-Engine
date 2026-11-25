#pragma once

#include "View.h"

#include <array>

class ModuleTime;

/**
 * @brief View that shows every timer of ModuleTime, the Play / Stop / Step buttons and the frame rate cap.
 */
class ViewTimers : public View
{
public:

    /** @param time Source of the timings and target of the buttons; must outlive the view. */
    explicit ViewTimers(ModuleTime* time);

    /** @brief Builds the Timers window; needs the ImGui frame ModuleEditor started in preRender(). */
    void render() override;

private:
    enum { HISTORY_SIZE = 120 };

    ModuleTime* time = nullptr;

    bool limitFps = false;
    int  targetFps = 60;

    std::array<float, HISTORY_SIZE> frameMsHistory = {};    // Circular buffer, oldest entry at historyIndex
    int historyIndex = 0;
};
