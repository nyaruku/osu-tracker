#pragma once
#include <imgui/imgui.h>
#include <GLFW/glfw3.h>
#include <Core/Config.h>
#include <Api/Status.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <shared_mutex>
#include "ui_theme.h"

extern std::atomic<bool> fetch;
extern std::thread       fetchThread;

namespace Core::Session {
    extern std::chrono::system_clock::time_point lastFetchCompleted;
    extern std::chrono::milliseconds             lastFetchDuration;
    extern std::atomic<int>                      fetchCount;
    void restartFetch();
}

inline void drawDebugOverlay(ImFont* fontSmall, const ImGuiIO& io, int display_w, int display_h,
                              GLuint avatarTex, int avatarW, int avatarH)
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)display_w, (float)display_h));
    ImGui::PushStyleColor(ImGuiCol_WindowBg,      ImVec4(0.02f, 0.04f, 0.07f, 0.97f));
    ImGui::PushStyleColor(ImGuiCol_Header,        COL_PRIMARY);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, COL_LINK);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  COL_PRIMARY);
    ImGui::PushStyleColor(ImGuiCol_Text,          COL_TEXT);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  ImVec2(12, 12));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,    ImVec2(6, 5));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar  | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove      | ImGuiWindowFlags_NoSavedSettings;

    auto fmtTime = [](std::chrono::system_clock::time_point tp) -> std::string {
        if (tp == std::chrono::system_clock::time_point{}) return "never";
        auto t = std::chrono::system_clock::to_time_t(tp);
        char buf[16]; strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&t));
        return buf;
    };

    auto statusCol = [](int code) -> ImVec4 {
        if (code == 0)   return COL_MUTED;
        if (code == 200) return { 0.20f, 0.80f, 0.40f, 1.0f };
        return { 1.0f, 0.35f, 0.35f, 1.0f };
    };

    float valX = (float)display_w * 0.45f;

    auto row = [&](const char* label, const char* value) {
        ImGui::PushStyleColor(ImGuiCol_Text, COL_MUTED);
        ImGui::TextUnformatted(label);
        ImGui::PopStyleColor();
        ImGui::SameLine(valX);
        ImGui::TextUnformatted(value);
    };

    if (ImGui::Begin("##debug", nullptr, flags))
    {
        ImGui::PushFont(fontSmall);

        // -- RENDER --------------------------------------------------------
        if (ImGui::CollapsingHeader("  RENDER", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Spacing();
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "%.1f", io.Framerate);
            row("FPS", tmp);
            row("ImGui", IMGUI_VERSION);

            int fbW, fbH; glfwGetFramebufferSize(glfwGetCurrentContext(), &fbW, &fbH);
            snprintf(tmp, sizeof(tmp), "%d x %d", fbW, fbH);
            row("Framebuffer", tmp);

            int winW, winH; glfwGetWindowSize(glfwGetCurrentContext(), &winW, &winH);
            snprintf(tmp, sizeof(tmp), "%d x %d", winW, winH);
            row("Window", tmp);

            GLint vp[4]; glGetIntegerv(GL_VIEWPORT, vp);
            snprintf(tmp, sizeof(tmp), "%d,%d  %dx%d", vp[0], vp[1], vp[2], vp[3]);
            row("Viewport", tmp);

            row("GPU Vendor",   (const char*)glGetString(GL_VENDOR));
            row("GPU Renderer", (const char*)glGetString(GL_RENDERER));
            row("OpenGL",       (const char*)glGetString(GL_VERSION));
            row("GLSL",         (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

            snprintf(tmp, sizeof(tmp), "ID=%u  %dx%d", avatarTex, avatarW, avatarH);
            row("Avatar Tex", tmp);
            ImGui::Spacing();
        }

        // -- THREADS -------------------------------------------------------
        if (ImGui::CollapsingHeader("  THREADS", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Spacing();
            bool ftRunning = fetch.load() && fetchThread.joinable();

            ImGui::PushStyleColor(ImGuiCol_Text, COL_MUTED);
            ImGui::TextUnformatted("Fetch");
            ImGui::PopStyleColor();
            ImGui::SameLine(valX);
            ImGui::TextColored(
                ftRunning ? ImVec4{0.2f, 0.8f, 0.4f, 1} : ImVec4{1, 0.35f, 0.35f, 1},
                ftRunning ? "running" : "stopped");

            if (Core::Session::fetchCount.load() > 0) {
                char tmp[128];
                snprintf(tmp, sizeof(tmp), "last %s  (%lldms)",
                    fmtTime(Core::Session::lastFetchCompleted).c_str(),
                    (long long)Core::Session::lastFetchDuration.count());
                row("", tmp);

                auto nextAt = Core::Session::lastFetchCompleted +
                              std::chrono::milliseconds(Core::Config::application.apiInterval);
                auto remMs  = std::chrono::duration_cast<std::chrono::milliseconds>(
                                  nextAt - std::chrono::system_clock::now()).count();
                snprintf(tmp, sizeof(tmp), "cycle #%d  |  next in %lldms",
                    Core::Session::fetchCount.load(), remMs > 0 ? (long long)remMs : 0LL);
                row("", tmp);
            } else {
                row("", "waiting for first fetch...");
            }

            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, COL_MUTED); ImGui::TextUnformatted("UI");  ImGui::PopStyleColor();
            ImGui::SameLine(valX); ImGui::TextColored({0.2f, 0.8f, 0.4f, 1}, "running");

            ImGui::PushStyleColor(ImGuiCol_Text, COL_MUTED); ImGui::TextUnformatted("Web"); ImGui::PopStyleColor();
            ImGui::SameLine(valX); ImGui::TextColored({0.2f, 0.8f, 0.4f, 1}, "running");

            ImGui::Spacing();
        }

        // -- API -----------------------------------------------------------
        if (ImGui::CollapsingHeader("  API", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Spacing();
            row("Server",     Core::Config::application.server == Core::Config::server::titanic ? "Titanic" : "Bancho");
            row("Init fail",  Api::Status::init_api_failed ? "YES" : "no");
            row("Avatar URL", Core::Config::user.avatarUrl.empty() ? "-" : Core::Config::user.avatarUrl.c_str());

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, COL_MUTED);
            ImGui::TextUnformatted("Endpoint");
            ImGui::SameLine(valX);     ImGui::TextUnformatted("Code");
            ImGui::SameLine(valX + 50); ImGui::TextUnformatted("Last Call");
            ImGui::PopStyleColor();
            ImGui::Separator();

            auto apiRow = [&](const char* name, const Api::Status::EndpointStatus& st) {
                ImGui::PushStyleColor(ImGuiCol_Text, COL_MUTED);
                ImGui::TextUnformatted(name);
                ImGui::PopStyleColor();
                ImGui::SameLine(valX);
                if (!st.everCalled)
                    ImGui::TextColored(COL_MUTED, "-");
                else
                    ImGui::TextColored(statusCol(st.code), "%d", st.code);
                ImGui::SameLine(valX + 50);
                ImGui::TextColored(COL_MUTED, "%s",
                    fmtTime(st.everCalled ? st.lastCall : std::chrono::system_clock::time_point{}).c_str());
            };

            if (Core::Config::application.server == Core::Config::server::bancho) {
                apiRow("Auth",       Api::Status::statusAuth);
                apiRow("Bancho",     Api::Status::statusBancho);
                apiRow("Respektive", Api::Status::statusRespektive);
                apiRow("Inspector",  Api::Status::statusInspector);
            } else {
                apiRow("Titanic",    Api::Status::statusTitanic);
            }
            ImGui::Spacing();
        }

        // -- DATA ----------------------------------------------------------
        if (ImGui::CollapsingHeader("  DATA"))
        {
            ImGui::Spacing();
            row("Username", Core::Config::user.username.empty() ? "-" : Core::Config::user.username.c_str());

            std::shared_lock<std::shared_mutex> dataLock(Core::Config::data::dataMutex);
            char tmp[16]; snprintf(tmp, sizeof(tmp), "%d", (int)Core::Config::data::arr.size());
            row("Entries", tmp);
            ImGui::Spacing();

            const int fmtSize = (int)Core::Config::data::arrFormatted.size();
            for (size_t j = 0; j < Core::Config::data::arr.size(); j++) {
                const auto& raw = Core::Config::data::arr[j];
                if (ImGui::CollapsingHeader(raw.key.c_str())) {
                    if ((int)j >= fmtSize) continue;
                    const auto& fmt = Core::Config::data::arrFormatted[j];
                    row("init",          raw.init.empty()    ? "-" : raw.init.c_str());
                    row("init (fmt)",    fmt.init.empty()    ? "-" : fmt.init.c_str());
                    row("current",       raw.current.empty() ? "-" : raw.current.c_str());
                    row("current (fmt)", fmt.current.empty() ? "-" : fmt.current.c_str());
                    row("change",        raw.change.empty()  ? "-" : raw.change.c_str());
                    row("change (fmt)",  fmt.change.empty()  ? "-" : fmt.change.c_str());
                    char flags[64];
                    snprintf(flags, sizeof(flags), "bancho=%s  titanic=%s  display=%s",
                        raw.banchoSupport  ? "y" : "n",
                        raw.titanicSupport ? "y" : "n",
                        raw.display        ? "y" : "n");
                    row("flags", flags);
                }
            }
            ImGui::Spacing();
        }

        ImGui::PopFont();
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(5);
}


