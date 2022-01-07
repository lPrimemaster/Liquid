#include "overlay.h"

#include "../../imgui/imgui.h"
#include "../../imgui/imgui_internal.h"
#include "../../imgui/imgui_impl_glfw.h"
#include "../../imgui/imgui_impl_opengl3.h"

#include "../../thread/threadpool.h"
#include "../raycaster/caster.h"

// Loading samples
#include "../samples/samples.h"

#include <thread>
#include <string>
#include <future>

internal Overlay::OverlaySettings overlaySettings;
internal Overlay::RenderSettings renderSettings;
internal Image* rtRenderTarget = nullptr;
internal std::atomic<i32> loadingBarPct = -1;
internal std::atomic<i32>  renderBarPct = -1;
internal std::mutex renderBarMtx;
internal f32 renderBarPctFloating = 0.0;
internal std::chrono::time_point<std::chrono::steady_clock> loadStart;
internal std::future<Scene> sceneHandle;

internal i32 lastRenderRes = 0;
internal i32 lastRenderSpp = 0;
internal i64 lastRenderTimeMs = 0;

#define RENDER_SETTINGS_STORE(x) renderSettings.x.store(x)
#define RENDER_SETTINGS_LOAD(x) renderSettings.x.load()
#define ITEM_SIZE 150

Overlay::RenderSettings& Overlay::GetRenderSettings()
{
    return renderSettings;
}

Image* Overlay::GetRenderTarget()
{
    return rtRenderTarget;
}

void Overlay::DeleteRenderTarget()
{
    if(rtRenderTarget)
        delete rtRenderTarget;
}

namespace ImGui
{    
    bool BufferingBar(const char* label, float value,  const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;
        
        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = size_arg;
        size.x -= style.FramePadding.x * 2;
        
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(bb, id))
            return false;
        
        // Render
        const float circleStart = size.x - style.FramePadding.x * 2;
        
        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart*value, bb.Max.y), fg_col);
        return true;
    }
}

internal void LoadingWindow(const std::string& title, std::atomic<i32>* flag, void (*onFinish)(), bool add_cond = false)
{
    const ImGuiWindowFlags flags = 
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoTitleBar;
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(
        ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
        ImGuiCond_Always,
        ImVec2(0.5f,0.5f)
    );

    ImGui::SetNextWindowSize(ImVec2(200, 40));
    ImGui::Begin("##loading", nullptr, flags);

    const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);

    i32 progress = flag->load();
    ImGui::Text("%s... (%llds)",
        title.c_str(), 
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - loadStart
        ).count()
    );
    ImGui::BufferingBar("##buffer_bar", progress / 100.0f, ImVec2(200, 6), bg, col);
    ImGui::End();

    if(progress >= 100.0f || add_cond)
    {
        onFinish();
        flag->store(-1);
    }
}

internal void StartAsyncSceneLoad(Scene (*loader)(std::atomic<i32>* progress))
{
    if(renderSettings.world.top)
        Scene::FreeScene(&renderSettings.world);
    loadingBarPct.store(0);
    loadStart = std::chrono::steady_clock::now();
    sceneHandle = std::async(std::launch::async, loader, &loadingBarPct);
}

internal void DisplayMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if(ImGui::BeginMenu("Load Examples"))
            {
                if(ImGui::MenuItem("SimpleSpaceEarth"))
                {
                    StartAsyncSceneLoad(Samples::BasicSphere);
                }
                if(ImGui::MenuItem("SingleEarth"))
                {
                    StartAsyncSceneLoad(Samples::SingleSphere);
                }
                ImGui::EndMenu();
            }
            
            if(ImGui::MenuItem("Save Render..."))
            {
                overlaySettings.displayRenderSave = !overlaySettings.displayRenderSave;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::MenuItem("Render Settings"))
            {
                overlaySettings.displaySettings = !overlaySettings.displaySettings;
            }
            ImGui::EndMenu();
        }

        bool rtRender = RENDER_SETTINGS_LOAD(rtRender);
        if(ImGui::MenuItem("Render", nullptr, false, !rtRender && renderSettings.world.top))
        {
            rtRender = !rtRender;
            RENDER_SETTINGS_STORE(rtRender);
            if(rtRender)
            {
                if(renderSettings.workingPool != nullptr)
                    delete renderSettings.workingPool;

                if(rtRenderTarget != nullptr)
                    delete rtRenderTarget;
                
                rtRenderTarget = new Image(
                    RENDER_SETTINGS_LOAD(rtImageW),
                    RENDER_SETTINGS_LOAD(rtImageH),
                    4
                );
                
                renderSettings.workingPool = new ThreadPool(
                    RENDER_SETTINGS_LOAD(rtBlocksX),
                    RENDER_SETTINGS_LOAD(rtBlocksY),
                    &renderSettings.world,
                    rtRenderTarget,
                    RENDER_SETTINGS_LOAD(rtSamples),
                    calculateChunk,
                    &renderSettings.rtRender,
                    &renderBarPctFloating,
                    &renderBarMtx
                );

                loadStart = std::chrono::steady_clock::now();
                renderBarPct.store(0);
                renderBarPctFloating = 0.0f; // NOTE: No lock is okay here
                renderSettings.workingPool->run();
            }
        }

        ImGui::Separator();
        // ImGui::Dummy(ImVec2(100.0f, 0.0f));

        if(renderSettings.world.top)
            ImGui::TextColored(
                ImVec4(1.0f, 1.0f, 1.0f, 0.7f), 
                "Loaded scene: %s", renderSettings.world.name.c_str()
            );
        else
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 0.7f), "Loaded scene: [None]");

        ImGui::Separator();

        if(lastRenderRes)
            ImGui::TextColored(
                ImVec4(1.0f, 1.0f, 1.0f, 0.7f), 
                "Last render: %dp %dspp (%.2fs)",
                lastRenderRes,
                lastRenderSpp,
                lastRenderTimeMs / 1000.0f
            );

        ImGui::EndMainMenuBar();
    }
}

internal void DisplaySettings()
{
    if(ImGui::Begin("Render Settings", &overlaySettings.displaySettings, 
        ImGuiWindowFlags_NoCollapse))
    {
        ImGui::BeginDisabled(RENDER_SETTINGS_LOAD(rtRender));
        // RT SPP
        {
            ImGui::PushItemWidth(ITEM_SIZE);
            static i32 rtSamples = RENDER_SETTINGS_LOAD(rtSamples);
            if(ImGui::InputInt("Samples", &rtSamples, 1, 100))
            {
                if(rtSamples < 1) rtSamples = 1;
                RENDER_SETTINGS_STORE(rtSamples);
            }
            ImGui::PopItemWidth();
        }

        // RT Threads
        {
            ImGui::PushItemWidth(ITEM_SIZE);
            static int threadConcurrency = std::thread::hardware_concurrency();
            static std::string current = std::to_string(threadConcurrency);
            if(ImGui::BeginCombo("Threads", current.c_str()))
            {
                i32 rtThreads = 1;
                for(i32 i = 0; i < threadConcurrency; i++)
                {
                    std::string str = std::to_string(i + 1);
                    bool is_selected = (current == str);
                    if(ImGui::Selectable(str.c_str(), is_selected))
                    {
                        current = str;
                        rtThreads = i + 1;
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                RENDER_SETTINGS_STORE(rtThreads);

                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
        }

        // RT Grid Blocks
        {
            static i32 rtBlocksX = RENDER_SETTINGS_LOAD(rtBlocksX);
            static i32 rtBlocksY = RENDER_SETTINGS_LOAD(rtBlocksY);

            ImGui::PushItemWidth(25);
            ImGui::PushID(0);
            bool changed = ImGui::InputInt("", &rtBlocksX, 0, 1);
            ImGui::PopID();
            ImGui::SameLine();
            ImGui::Text("x");
            ImGui::SameLine();
            ImGui::PushID(1);
            changed |= ImGui::InputInt("", &rtBlocksY, 0, 1);
            ImGui::PopID();
            ImGui::SameLine();
            ImGui::Text("Blocks");
            ImGui::PopItemWidth();

            if(changed)
            {
                RENDER_SETTINGS_STORE(rtBlocksX);
                RENDER_SETTINGS_STORE(rtBlocksY);
            }
        }

        // RT Image resolution
        {
            ImGui::PushItemWidth(ITEM_SIZE);
            static i32 currentIdx = 3;
            static std::string resolutions[6] = { "144p",  "240p", "360p", "480p", "720p", "1080p" };
            if(ImGui::BeginCombo("Resolution", resolutions[currentIdx].c_str()))
            {
                for(i32 i = 0; i < 6; i++)
                {
                    bool is_selected = (resolutions[currentIdx] == resolutions[i]);
                    if(ImGui::Selectable(resolutions[i].c_str(), is_selected))
                    {
                        currentIdx = i;
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                u32 rtImageH = std::stoul(resolutions[currentIdx]);
                u32 rtImageW = (u32)((16.0f / 9.0f) * rtImageH);

                RENDER_SETTINGS_STORE(rtImageW);
                RENDER_SETTINGS_STORE(rtImageH);

                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
        }
        
        // Raster Enable/Disable
        {
            ImGui::PushItemWidth(ITEM_SIZE);
            i32 currentIdx = RENDER_SETTINGS_LOAD(rasterRender) ? 1 : 0;
            static std::string opt[2] = { "Off",  "On" };
            if(ImGui::BeginCombo("Raster Render", opt[currentIdx].c_str()))
            {
                for(i32 i = 0; i < 2; i++)
                {
                    bool is_selected = (opt[currentIdx] == opt[i]);
                    if(ImGui::Selectable(opt[i].c_str(), is_selected))
                    {
                        currentIdx = i;
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                bool rasterRender = opt[currentIdx] == "On";
                RENDER_SETTINGS_STORE(rasterRender);

                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
        }

        ImGui::EndDisabled();
        ImGui::End();
    }
}

internal void DisplayRenderSave()
{
    if(ImGui::Begin("Save Render", &overlaySettings.displayRenderSave, 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
    {
        static char buffer[512] = { "output.bmp" }; // There's probably a better way of doing this
        ImGui::InputText("Output filename", buffer, 512);

        ImGui::BeginDisabled(RENDER_SETTINGS_LOAD(rtRender));
        if(ImGui::Button("Save"))
        {
            // TODO: Filename and path validation
            if(buffer[0] != '\0' && rtRenderTarget != nullptr)
            {
                rtRenderTarget->saveToBMP(buffer);
            }
        }
        ImGui::EndDisabled();
        ImGui::End();
    }
}

void Overlay::Display()
{
    DisplayMenuBar();
    if(overlaySettings.displaySettings) DisplaySettings();
    if(overlaySettings.displayRenderSave) DisplayRenderSave();

    // Loading bars
    if(loadingBarPct.load() >= 0) LoadingWindow("Loading", &loadingBarPct, [](){ renderSettings.world = sceneHandle.get(); });
    if(renderBarPct.load() >= 0) 
    {
        // FIX: Don't do this dumb stuff... Atomic and mutex ?? Why am I so lazy
        {
            std::lock_guard<std::mutex> lock(renderBarMtx);
            renderBarPct.store((i32)renderBarPctFloating);
        }
        LoadingWindow("Rendering", &renderBarPct, [](){ 
                lastRenderRes = RENDER_SETTINGS_LOAD(rtImageH);
                lastRenderSpp = RENDER_SETTINGS_LOAD(rtSamples);
                lastRenderTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - loadStart
                ).count();
            },
            !RENDER_SETTINGS_LOAD(rtRender)
        );
    }
}
