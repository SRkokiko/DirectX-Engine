#include "Globals.h"
#include "ViewAbout.h"

#include "imgui.h"

namespace
{
    constexpr const char* ENGINE_NAME = "DirectX Engine";
    constexpr const char* ENGINE_DESCRIPTION = "A small DirectX 12 rendering engine with an ImGui-based editor";
    constexpr const char* AUTHOR = "Pau Bermudez (aka Koko)";
    constexpr const char* LICENSE_NAME = "MIT License";

    struct Library
    {
        const char* name;
        const char* version;
        const char* license;
    };

    // Versions come from the headers in Source/3rdParty
    const Library LIBRARIES[] = {
        { "Dear ImGui (docking branch)", IMGUI_VERSION, "MIT" },
        { "ImGuizmo", "1.91.3 WIP", "MIT" },
        { "DirectXTex", "Oct 2025 (release 194)", "MIT" },
        { "tinygltf", "2.8.10", "MIT" },
        { "DirectX Tool Kit (Keyboard, Mouse, GamePad, SimpleMath)", "-", "MIT" },
        { "WinPixEventRuntime", "-", "Microsoft" },
    };
}

void ViewAbout::render()
{
    if (!ImGui::Begin("About", &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }

    ImGui::TextUnformatted(ENGINE_NAME);
    ImGui::TextWrapped("%s", ENGINE_DESCRIPTION);
    ImGui::Text("Author: %s", AUTHOR);
    ImGui::Text("License: %s", LICENSE_NAME);

    ImGui::SeparatorText("Libraries");
    if (ImGui::BeginTable("libraries", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Version");
        ImGui::TableSetupColumn("License");
        ImGui::TableHeadersRow();

        for (const Library& lib : LIBRARIES)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(lib.name);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(lib.version);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(lib.license);
        }
        ImGui::EndTable();
    }

    ImGui::End();
}
