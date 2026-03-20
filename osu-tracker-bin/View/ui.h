#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl2.h>
#include <stdio.h>
#include <thread>
#include <chrono>
#include <GLFW/glfw3.h>
#ifndef GL_SHADING_LANGUAGE_VERSION
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#endif
#include <Core/Config.h>
#include <Resources/Fonts/droid_sans.h>
#include "ui_theme.h"
#include "ui_debug.h"

static void openUrl(const std::string& url) {
#ifdef _WIN32
    system(("start \"\" \"" + url + "\"").c_str());
#elif defined(__APPLE__)
    system(("open \"" + url + "\" 2>/dev/null &").c_str());
#else
    system(("xdg-open \"" + url + "\" 2>/dev/null &").c_str());
#endif
}

static GLFWwindow* window;
#define WINDOW_WIDTH  480
#define WINDOW_HEIGHT 800

static float ratio[]       = { 0.3f, 0.35f, 0.35f };
static float ratioSingle[] = { 0.3f, 0.7f };

void ui_mainTerminate() {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int ui_main()
{
#ifdef __linux__
    setenv("XDG_SESSION_TYPE", "x11", 1);
    setenv("WAYLAND_DISPLAY", "", 1);
#endif

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, OSU_TRACKER_TITLE, nullptr, nullptr);
    if (window == nullptr) return 1;

    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, 400, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename  = "imgui.ini";

    ImGui::StyleColorsDark();
    applyTheme();

    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;

    ImFont* fontDefault = io.Fonts->AddFontFromMemoryTTF((void*)DroidSans_ttf, sizeof(DroidSans_ttf), 18.0f, &font_cfg);
    ImFont* fontHeader  = io.Fonts->AddFontFromMemoryTTF((void*)DroidSans_ttf, sizeof(DroidSans_ttf), 22.0f, &font_cfg);
    ImFont* fontSmall   = io.Fonts->AddFontFromMemoryTTF((void*)DroidSans_ttf, sizeof(DroidSans_ttf), 15.0f, &font_cfg);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    int display_w = WINDOW_WIDTH, display_h = WINDOW_HEIGHT;
    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll;
    ImGuiWindowFlags    beginFlags = ImGuiWindowFlags_NoResize   | ImGuiWindowFlags_NoMove     |
                                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    GLuint avatarTex = 0;
    int    avatarW   = 0;
    int    avatarH   = 0;

    if (!Core::Config::user.avatarBytes.empty())
        Core::Config::user.avatarDirty = true;

    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &display_w, &display_h);
        bool window_active = !(glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0 ||
                               glfwGetWindowAttrib(window, GLFW_FOCUSED)   == 0);
        glfwPollEvents();
        if (!window_active) {
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }

        if (Core::Config::user.avatarDirty.load()) {
            if (avatarTex) { glDeleteTextures(1, &avatarTex); avatarTex = 0; }
            std::lock_guard<std::mutex> lk(Core::Config::user.avatarMutex);
            const auto& b = Core::Config::user.avatarBytes;
            if (!b.empty()) {
                int n;
                unsigned char* px = stbi_load_from_memory(b.data(), (int)b.size(), &avatarW, &avatarH, &n, 4);
                if (px) {
                    glGenTextures(1, &avatarTex);
                    glBindTexture(GL_TEXTURE_2D, avatarTex);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, avatarW, avatarH, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    stbi_image_free(px);
                }
            }
            Core::Config::user.avatarDirty = false;
        }

        static bool debug    = false;
        static bool f10_last = false;
        bool f10 = glfwGetKey(window, GLFW_KEY_F10) == GLFW_PRESS;
        if (f10 && !f10_last) debug = !debug;
        f10_last = f10;

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)display_w, (float)display_h));

        if (ImGui::Begin(
            Core::Config::user.username.empty() ? "osu! Tracker" : Core::Config::user.username.c_str(),
            nullptr, beginFlags))
        {
            // -- Profile header --------------------------------------------
            constexpr float AV_SIZE = 52.0f;

            ImDrawList* dl   = ImGui::GetWindowDrawList();
            ImVec2      pos  = ImGui::GetCursorScreenPos();
            ImVec2      pos2 = ImVec2(pos.x + AV_SIZE, pos.y + AV_SIZE);

            if (avatarTex) {
                dl->AddImage((ImTextureID)(intptr_t)avatarTex, pos, pos2);
                dl->AddRect(pos, pos2, IM_COL32(43, 125, 233, 200), 0.0f, 0, 1.5f);
            } else {
                dl->AddRectFilled(pos, pos2, IM_COL32(30, 37, 48, 255));
                dl->AddRect(pos, pos2, IM_COL32(43, 125, 233, 160), 0.0f, 0, 1.5f);
            }

            ImGui::Dummy(ImVec2(AV_SIZE, AV_SIZE));
            ImGui::SameLine(0, 10);
            ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, pos.y));

            ImGui::BeginGroup();

            ImGui::PushFont(fontHeader);
            ImGui::TextUnformatted(Core::Config::user.username.empty() ? "No User" : Core::Config::user.username.c_str());
            ImGui::PopFont();

            {
                ImGui::PushFont(fontSmall);
                const float spcX      = ImGui::GetStyle().ItemSpacing.x;
                const float availW    = ImGui::GetContentRegionAvail().x;
                const float settingsW = 70.0f;
                const float comboW    = (availW - settingsW - spcX * 2.0f) / 2.0f;

                static bool pendingSwitch    = false;
                static int  switchFetchCount = -1;
                if (pendingSwitch && Core::Session::fetchCount.load() > switchFetchCount)
                    pendingSwitch = false;

                static const char* srvItems[] = { "Bancho", "Titanic" };
                int srvIdx = (int)Core::Config::application.server;
                ImGui::SetNextItemWidth(comboW);
                if (ImGui::Combo("##server", &srvIdx, srvItems, 2)) {
                    Core::Config::application.server = (Core::Config::server)srvIdx;
                    Core::Config::write();
                    Core::Session::restartFetch();
                    pendingSwitch    = true;
                    switchFetchCount = Core::Session::fetchCount.load();
                }

                ImGui::SameLine();

                static const char* modeItems[] = { "osu!", "taiko", "fruits", "mania" };
                int modeIdx = (int)Core::Config::application.gameMode;
                ImGui::SetNextItemWidth(comboW);
                if (ImGui::Combo("##mode", &modeIdx, modeItems, 4)) {
                    Core::Config::application.gameMode = (Core::Config::gameMode)modeIdx;
                    Core::Config::write();
                    Core::Session::restartFetch();
                    pendingSwitch    = true;
                    switchFetchCount = Core::Session::fetchCount.load();
                }

                ImGui::SameLine();

                ImGui::PushStyleColor(ImGuiCol_Button,        COL_SURFACE);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COL_SURFACE2);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  COL_SURFACE2);
                ImGui::PushStyleColor(ImGuiCol_Text,          COL_MUTED);
                if (ImGui::Button("Settings", ImVec2(settingsW, 0))) {
                    openUrl("http://" OSU_TRACKER_WEBSERVER_HOST ":" + std::to_string(OSU_TRACKER_WEBSERVER_PORT)+ "/settings");
                }
                ImGui::PopStyleColor(4);

                if (pendingSwitch) {
                    const char* anim[] = { "updating.", "updating..", "updating..." };
                    ImGui::TextColored(COL_MUTED, "%s", anim[(int)(ImGui::GetTime() * 4.0) % 3]);
                }

                ImGui::PopFont();
            }

            ImGui::EndGroup();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // -- Stats list ------------------------------------------------
            ImGui::PushFont(fontDefault);
            {
                std::shared_lock<std::shared_mutex> dataLock(Core::Config::data::dataMutex);
                int i = -1;
                for (auto& entry : Core::Config::data::arrFormatted)
                {
                    i++;
                    if (!entry.display) continue;

                    switch (Core::Config::application.server) {
                        case Core::Config::server::bancho:  if (!entry.banchoSupport)  continue; break;
                        case Core::Config::server::titanic: if (!entry.titanicSupport) continue; break;
                    }

                    float* ratioPtr;
                    int    nCols;
                    if (entry.single) { ratioPtr = ratioSingle; nCols = 2; }
                    else              { ratioPtr = ratio;        nCols = 3; }

                    ImGui::Columns(nCols, nullptr, false);
                    float totalWidth = (float)display_w - ImGui::GetStyle().WindowPadding.x * 2.0f;
                    for (int c = 0; c < nCols; ++c)
                        ImGui::SetColumnWidth(c, totalWidth * ratioPtr[c]);

                    ImGui::PushStyleColor(ImGuiCol_Text, COL_MUTED);
                    ImGui::TextUnformatted(entry.name.empty() ? "##ERR" : entry.name.c_str());
                    ImGui::PopStyleColor();
                    ImGui::NextColumn();

                    ImGui::PushItemWidth(-1);
                    ImGui::InputText(("##cur" + std::to_string(i)).c_str(), &entry.current, inputFlags);
                    ImGui::PopItemWidth();
                    ImGui::NextColumn();

                    if (!entry.single) {
                        const ImVec4 chgCol = entry.change.empty()
                            ? COL_MUTED
                            : (entry.positive ? COL_GREEN : COL_RED);
                        ImGui::PushStyleColor(ImGuiCol_Text, chgCol);
                        ImGui::PushItemWidth(-1);
                        ImGui::InputText(("##chg" + std::to_string(i)).c_str(), &entry.change, inputFlags);
                        ImGui::PopItemWidth();
                        ImGui::PopStyleColor();
                        ImGui::NextColumn();
                    }

                    ImGui::Columns(1);
                }
            }
            ImGui::PopFont();
        }
        ImGui::End();

        if (debug)
            drawDebugOverlay(fontSmall, io, display_w, display_h, avatarTex, avatarW, avatarH);

        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(COL_BG.x, COL_BG.y, COL_BG.z, COL_BG.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    if (avatarTex) glDeleteTextures(1, &avatarTex);

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    glfwDestroyWindow(window);
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}