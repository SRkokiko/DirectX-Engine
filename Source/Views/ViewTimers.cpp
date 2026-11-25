#include "Globals.h"
#include "ViewTimers.h"

#include "Modules/ModuleTime.h"

#include "imgui.h"

ViewTimers::ViewTimers(ModuleTime* time) : time(time)
{
    // Pick up the current cap, so reopening the view doesn't change it
    uint32_t maxFps = time->getState().maxFps;
    limitFps = maxFps != 0;
    if (limitFps)
        targetFps = int(maxFps);
}

void ViewTimers::render()
{
    if (!ImGui::Begin("Timers", &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }

    const TimeState& state = time->getState();

    // Plain history of the real frame time, in ms; it keeps recording while the simulation is stopped
    frameMsHistory[historyIndex] = float(state.frameUs) * 1e-3f;
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;

    ImGui::SeparatorText("Simulation");

    bool playing = time->isPlaying();
    if (ImGui::Button("Play") && !playing)
        time->play();
    ImGui::SameLine();
    if (ImGui::Button("Stop") && playing)
        time->stop();
    ImGui::SameLine();
    ImGui::BeginDisabled(playing);
    if (ImGui::Button("Step"))
        time->step();
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::TextUnformatted(playing ? "Playing" : "Stopped");

    ImGui::SeparatorText("Frame rate");

    bool changed = ImGui::Checkbox("Limit FPS", &limitFps);
    ImGui::BeginDisabled(!limitFps);
    ImGui::SameLine();
    changed |= ImGui::SliderInt("##targetFps", &targetFps, 10, 240);
    ImGui::EndDisabled();
    if (changed)
        time->setMaxFps(limitFps ? uint32_t(targetFps) : 0);

    ImGui::Text("FPS: %.1f", state.fps);
    ImGui::PlotLines("##frameMs", frameMsHistory.data(), HISTORY_SIZE, historyIndex, "Frame time (ms)", 0.0f, FLT_MAX, ImVec2(260, 60));

    if (ImGui::BeginTable("timers", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Timer");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        auto row = [](const char* name, const char* fmt, auto... values)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(name);
            ImGui::TableNextColumn();
            ImGui::Text(fmt, values...);
        };

        row("dt (game)",         "%.6f s", state.dt);
        row("dt (real)",         "%.6f s", state.realDt);
        row("Frame time",        "%.3f ms (%llu us)", state.frameUs * 1e-3, (unsigned long long)state.frameUs);
        row("Frame count",       "%llu", (unsigned long long)state.frameCount);
        row("Real time",         "%.3f s", state.totalUs * 1e-6);
        row("Game time",         "%.3f s", state.gameTotalUs * 1e-6);
        row("Engine init time",  "%.3f ms (%llu us)", state.initUs * 1e-3, (unsigned long long)state.initUs);

        ImGui::EndTable();
    }

    ImGui::End();
}
