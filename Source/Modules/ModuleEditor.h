#pragma once

#include "Module.h"

class ModuleD3D12;
class ImGuiPass;

/**
 * @brief Owns the Dear ImGui context and its Win32/DX12 backends, and brackets the frame's UI.
 *
 * Must be registered after every module that draws ImGui or scene content: ImGui calls can be made
 * from any module's render() (the context is global), and the UI is recorded last so it ends up
 * on top of the scene. It must also be registered after ModuleD3D12, whose preRender() has to
 * wait on the fence and open the command list first.
 */
class ModuleEditor : public Module
{
public:

    /**
     * @param d3d12 Provides the device and the frame's command list.
     * @param hWnd Window that feeds input to ImGui.
     */
    ModuleEditor(ModuleD3D12* d3d12, HWND hWnd);
    ~ModuleEditor() override;

    /**
     * @brief Creates the ImGui context and its backends.
     * @return false if the pass can't be created.
     */
    bool init() override;

    /** @brief Starts the ImGui frame; from here on, any module can make ImGui calls. */
    void preRender() override;

    /** @brief Ends the ImGui frame and records its draw data into the frame's command list. */
    void render() override;

    /** @brief Waits for the GPU and destroys the ImGui context and backends. */
    bool cleanUp() override;

private:
    ModuleD3D12* d3d12 = nullptr;
    HWND hWnd = nullptr;

    ImGuiPass* imguiPass = nullptr;
};
