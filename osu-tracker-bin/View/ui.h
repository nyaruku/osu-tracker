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
#include <Core/Config.h>
#include <Resources/Fonts/droid_sans.h>

// Forward-declarations for symbols defined in Session.h (which includes this file).
extern std::atomic<bool> fetch;
extern std::thread       fetchThread;
namespace Core::Session {
    extern std::chrono::system_clock::time_point lastFetchCompleted;
    extern std::chrono::milliseconds             lastFetchDuration;
    extern std::atomic<int>                      fetchCount;
    void restartFetch();
}

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

// Layout ratios
static float ratio[]       = { 0.3f, 0.35f, 0.35f };
static float ratioSingle[] = { 0.3f, 0.7f };

void ui_mainTerminate() {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static constexpr ImVec4 COL_BG       = { 0.051f, 0.067f, 0.090f, 1.0f }; // #0d1117
static constexpr ImVec4 COL_SURFACE  = { 0.086f, 0.106f, 0.133f, 1.0f }; // #161b22
static constexpr ImVec4 COL_SURFACE2 = { 0.118f, 0.145f, 0.188f, 1.0f }; // #1e2530
static constexpr ImVec4 COL_PRIMARY  = { 0.169f, 0.490f, 0.914f, 1.0f }; // #2b7de9
static constexpr ImVec4 COL_LINK     = { 0.302f, 0.624f, 1.000f, 1.0f }; // #4d9fff
static constexpr ImVec4 COL_TEXT     = { 0.902f, 0.929f, 0.953f, 1.0f }; // #e6edf3
static constexpr ImVec4 COL_MUTED    = { 0.545f, 0.580f, 0.620f, 1.0f }; // #8b949e
static constexpr ImVec4 COL_BORDER   = { 1.0f,   1.0f,   1.0f,   0.07f };
static constexpr ImVec4 COL_GREEN    = { 0.20f,  0.80f,  0.40f,  1.0f };
static constexpr ImVec4 COL_RED      = { 1.0f,   0.35f,  0.35f,  1.0f };

static void applyTheme()
{
    ImGuiStyle& s = ImGui::GetStyle();

    // Geometry
    s.WindowPadding      = ImVec2(12, 12);
    s.FramePadding       = ImVec2(8, 5);
    s.ItemSpacing        = ImVec2(6, 6);
    s.ItemInnerSpacing   = ImVec2(6, 4);
    s.ScrollbarSize      = 8.0f;
    s.WindowBorderSize   = 0.0f;
    s.FrameBorderSize    = 1.0f;
    s.FrameRounding      = 6.0f;
    s.ScrollbarRounding  = 4.0f;
    s.GrabRounding       = 4.0f;
    s.PopupRounding      = 6.0f;
    s.WindowRounding     = 0.0f;
    s.SeparatorTextPadding = ImVec2(0, 6);

    // Colours
    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]             = COL_BG;
    c[ImGuiCol_ChildBg]              = COL_SURFACE;
    c[ImGuiCol_PopupBg]              = COL_SURFACE;
    c[ImGuiCol_Border]               = COL_BORDER;
    c[ImGuiCol_FrameBg]              = COL_SURFACE;
    c[ImGuiCol_FrameBgHovered]       = COL_SURFACE2;
    c[ImGuiCol_FrameBgActive]        = COL_SURFACE2;
    c[ImGuiCol_TitleBg]              = COL_PRIMARY;
    c[ImGuiCol_TitleBgActive]        = COL_PRIMARY;
    c[ImGuiCol_TitleBgCollapsed]     = COL_PRIMARY;
    c[ImGuiCol_ScrollbarBg]          = COL_BG;
    c[ImGuiCol_ScrollbarGrab]        = COL_SURFACE2;
    c[ImGuiCol_ScrollbarGrabHovered] = { 0.4f, 0.5f, 0.6f, 1.0f };
    c[ImGuiCol_ScrollbarGrabActive]  = COL_LINK;
    c[ImGuiCol_CheckMark]            = COL_LINK;
    c[ImGuiCol_SliderGrab]           = COL_PRIMARY;
    c[ImGuiCol_SliderGrabActive]     = COL_LINK;
    c[ImGuiCol_Button]               = COL_PRIMARY;
    c[ImGuiCol_ButtonHovered]        = COL_LINK;
    c[ImGuiCol_ButtonActive]         = COL_PRIMARY;
    c[ImGuiCol_Header]               = { COL_PRIMARY.x, COL_PRIMARY.y, COL_PRIMARY.z, 0.4f };
    c[ImGuiCol_HeaderHovered]        = { COL_PRIMARY.x, COL_PRIMARY.y, COL_PRIMARY.z, 0.6f };
    c[ImGuiCol_HeaderActive]         = COL_PRIMARY;
    c[ImGuiCol_Separator]            = COL_BORDER;
    c[ImGuiCol_SeparatorHovered]     = COL_LINK;
    c[ImGuiCol_SeparatorActive]      = COL_LINK;
    c[ImGuiCol_Text]                 = COL_TEXT;
    c[ImGuiCol_TextDisabled]         = COL_MUTED;
    c[ImGuiCol_NavHighlight]         = COL_LINK;
}

// Main code
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
    ImGuiInputTextFlags inputFlags  = ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll;
    ImGuiWindowFlags    beginFlags  = ImGuiWindowFlags_NoResize    | ImGuiWindowFlags_NoMove      |
                                      ImGuiWindowFlags_NoCollapse  | ImGuiWindowFlags_NoTitleBar;

    // Avatar texture state
    GLuint avatarTex = 0;
    int    avatarW   = 0;
    int    avatarH   = 0;

    // If the UI is reopened after a previous session, avatarBytes may already
    // be populated but avatarDirty is false (URL didn't change). Force a reload.
    if (!Core::Config::user.avatarBytes.empty())
        Core::Config::user.avatarDirty = true;

    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &display_w, &display_h);
        bool window_active = !(glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0 ||
                               glfwGetWindowAttrib(window, GLFW_FOCUSED)   == 0);
        glfwPollEvents();
        if (!window_active)
            std::this_thread::sleep_for(std::chrono::milliseconds(33));

        // Reload avatar texture when the API thread signals new bytes
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

        // Debug overlay toggle (F10)
        static bool debug = false;
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
            // -- Profile header ---------------------------------------------
            constexpr float AV_SIZE = 52.0f;

            ImDrawList* dl  = ImGui::GetWindowDrawList();
            ImVec2      pos = ImGui::GetCursorScreenPos();
            ImVec2      pos2 = ImVec2(pos.x + AV_SIZE, pos.y + AV_SIZE);

            if (avatarTex) {
                dl->AddImage((ImTextureID)(intptr_t)avatarTex, pos, pos2);
                // Thin blue border
                dl->AddRect(pos, pos2, IM_COL32(43, 125, 233, 200), 0.0f, 0, 1.5f);
            } else {
                // Placeholder square
                dl->AddRectFilled(pos, pos2, IM_COL32(30, 37, 48, 255));
                dl->AddRect(pos, pos2, IM_COL32(43, 125, 233, 160), 0.0f, 0, 1.5f);
            }

            // Advance past the avatar image
            ImGui::Dummy(ImVec2(AV_SIZE, AV_SIZE));
            ImGui::SameLine(0, 10);

            // Username + server badge stacked vertically
            float textY = pos.y + (AV_SIZE - ImGui::GetTextLineHeightWithSpacing() * 2.0f) * 0.5f;
            ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, textY));

            ImGui::PushFont(fontHeader);
            ImGui::TextUnformatted(Core::Config::user.username.empty() ? "No User" : Core::Config::user.username.c_str());
            ImGui::PopFont();

            // Server badge
            ImGui::PushFont(fontSmall);
            const char* serverLabel = (Core::Config::application.server == Core::Config::server::titanic) ? "Titanic" : "Bancho";
            const ImVec4 badgeCol   = (Core::Config::application.server == Core::Config::server::titanic)
                                        ? ImVec4(0.60f, 0.22f, 0.80f, 1.0f)
                                        : ImVec4(0.169f, 0.490f, 0.914f, 1.0f);
            ImGui::TextColored(badgeCol, "%s", serverLabel);
            ImGui::PopFont();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // -- Toolbar ---------------------------------------------------
            {
                ImGui::PushFont(fontSmall);

                auto btnStyle = [&](bool active) {
                    ImGui::PushStyleColor(ImGuiCol_Button,        active ? COL_PRIMARY  : COL_SURFACE);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, active ? COL_LINK     : COL_SURFACE2);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  active ? COL_PRIMARY  : COL_SURFACE2);
                    ImGui::PushStyleColor(ImGuiCol_Text,          active ? COL_TEXT     : COL_MUTED);
                };

                const float spcX   = ImGui::GetStyle().ItemSpacing.x;
                const float availW = ImGui::GetContentRegionAvail().x;

                // Game-mode row
                const char* modeLabels[] = { "osu!", "taiko", "fruits", "mania" };
                const float modeW = (availW - spcX * 3.0f) / 4.0f;
                for (int m = 0; m < 4; m++) {
                    bool active = (int)Core::Config::application.gameMode == m;
                    btnStyle(active);
                    if (ImGui::Button(modeLabels[m], ImVec2(modeW, 0)) && !active) {
                        Core::Config::application.gameMode = (Core::Config::gameMode)m;
                        Core::Config::write();
                        Core::Session::restartFetch();
                    }
                    ImGui::PopStyleColor(4);
                    if (m < 3) ImGui::SameLine();
                }

                // Server row + Settings
                const float settingsW  = 80.0f;
                const float serverW    = (availW - settingsW - spcX * 3.0f) / 2.0f;
                const char* srvLabels[] = { "Bancho", "Titanic" };
                for (int sv = 0; sv < 2; sv++) {
                    bool active = (int)Core::Config::application.server == sv;
                    btnStyle(active);
                    if (ImGui::Button(srvLabels[sv], ImVec2(serverW, 0)) && !active) {
                        Core::Config::application.server = (Core::Config::server)sv;
                        Core::Config::write();
                        Core::Session::restartFetch();
                    }
                    ImGui::PopStyleColor(4);
                    if (sv == 0) ImGui::SameLine();
                }

                // Settings button
                ImGui::SameLine();
                ImGui::SetCursorPosX(
                    ImGui::GetWindowWidth() - settingsW
                    - ImGui::GetStyle().WindowPadding.x
                    - ImGui::GetStyle().ScrollbarSize);
                ImGui::PushStyleColor(ImGuiCol_Button,        COL_SURFACE);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COL_SURFACE2);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  COL_SURFACE2);
                ImGui::PushStyleColor(ImGuiCol_Text,          COL_MUTED);
                if (ImGui::Button("Settings", ImVec2(settingsW, 0))) {
                    openUrl(
                        "http://" OSU_TRACKER_WEBSERVER_HOST ":"
                        + std::to_string(OSU_TRACKER_WEBSERVER_PORT)
                        + "/settings");
                }
                ImGui::PopStyleColor(4);

                ImGui::PopFont();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Stats list
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

                    // Label (muted)
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

        // Debug overlay
        if (debug) {
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

            ImGuiWindowFlags debugFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                          ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoSavedSettings;

            // Helpers
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
            auto dbgRow = [&](const char* label, const char* value) {
                ImGui::PushStyleColor(ImGuiCol_Text, COL_MUTED);
                ImGui::TextUnformatted(label);
                ImGui::PopStyleColor();
                ImGui::SameLine(valX);
                ImGui::TextUnformatted(value);
            };

            if (ImGui::Begin("##debug", nullptr, debugFlags))
            {
                ImGui::PushFont(fontSmall);

                // RENDER
                if (ImGui::CollapsingHeader("  RENDER", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Spacing();
                    char tmp[128];
                    snprintf(tmp, sizeof(tmp), "%.1f", io.Framerate);      dbgRow("FPS",         tmp);
                    dbgRow("ImGui",  IMGUI_VERSION);
                    int fbW, fbH; glfwGetFramebufferSize(glfwGetCurrentContext(), &fbW, &fbH);
                    snprintf(tmp, sizeof(tmp), "%d × %d", fbW, fbH);       dbgRow("Framebuffer", tmp);
                    int winW, winH; glfwGetWindowSize(glfwGetCurrentContext(), &winW, &winH);
                    snprintf(tmp, sizeof(tmp), "%d × %d", winW, winH);     dbgRow("Window",      tmp);
                    GLint vp[4]; glGetIntegerv(GL_VIEWPORT, vp);
                    snprintf(tmp, sizeof(tmp), "%d,%d  %d×%d", vp[0],vp[1],vp[2],vp[3]); dbgRow("Viewport", tmp);
                    dbgRow("GPU Vendor",   (const char*)glGetString(GL_VENDOR));
                    dbgRow("GPU Renderer", (const char*)glGetString(GL_RENDERER));
                    dbgRow("OpenGL",       (const char*)glGetString(GL_VERSION));
                    dbgRow("GLSL",         (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
                    snprintf(tmp, sizeof(tmp), "ID=%u  %dx%d", avatarTex, avatarW, avatarH);
                    dbgRow("Avatar Tex",   tmp);
                    ImGui::Spacing();
                }

                // THREADS
                if (ImGui::CollapsingHeader("  THREADS", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Spacing();
                    bool ftRunning = fetch.load() && fetchThread.joinable();

                    // Fetch row
                    ImGui::PushStyleColor(ImGuiCol_Text, COL_MUTED); ImGui::TextUnformatted("Fetch"); ImGui::PopStyleColor();
                    ImGui::SameLine(valX);
                    ImGui::TextColored(ftRunning ? ImVec4{0.2f,0.8f,0.4f,1} : ImVec4{1,0.35f,0.35f,1},
                                       ftRunning ? "running" : "stopped");

                    if (Core::Session::fetchCount.load() > 0) {
                        char tmp[128];
                        snprintf(tmp, sizeof(tmp), "last %s  (%lldms)",
                            fmtTime(Core::Session::lastFetchCompleted).c_str(),
                            (long long)Core::Session::lastFetchDuration.count());
                        dbgRow("", tmp);

                        auto nextAt   = Core::Session::lastFetchCompleted + std::chrono::milliseconds(Core::Config::application.apiInterval);
                        auto remMs    = std::chrono::duration_cast<std::chrono::milliseconds>(nextAt - std::chrono::system_clock::now()).count();
                        snprintf(tmp, sizeof(tmp), "cycle #%d  |  next in %lldms",
                            Core::Session::fetchCount.load(), remMs > 0 ? (long long)remMs : 0LL);
                        dbgRow("", tmp);
                    } else {
                        dbgRow("", "waiting for first fetch...");
                    }

                    ImGui::Spacing();

                    // UI thread
                    ImGui::PushStyleColor(ImGuiCol_Text, COL_MUTED); ImGui::TextUnformatted("UI"); ImGui::PopStyleColor();
                    ImGui::SameLine(valX); ImGui::TextColored({0.2f,0.8f,0.4f,1}, "running");

                    // Web / main thread
                    ImGui::PushStyleColor(ImGuiCol_Text, COL_MUTED); ImGui::TextUnformatted("Web"); ImGui::PopStyleColor();
                    ImGui::SameLine(valX); ImGui::TextColored({0.2f,0.8f,0.4f,1}, "running");
                    ImGui::Spacing();
                }

                // API
                if (ImGui::CollapsingHeader("  API", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Spacing();
                    dbgRow("Server",     Core::Config::application.server == Core::Config::server::titanic ? "Titanic" : "Bancho");
                    dbgRow("Init fail",  Api::Status::init_api_failed ? "YES" : "no");
                    dbgRow("Avatar URL", Core::Config::user.avatarUrl.empty() ? "-" : Core::Config::user.avatarUrl.c_str());

                    ImGui::Spacing();
                    // Column header
                    ImGui::PushStyleColor(ImGuiCol_Text, COL_MUTED);
                    ImGui::TextUnformatted("Endpoint");
                    ImGui::SameLine(valX); ImGui::TextUnformatted("Code");
                    ImGui::SameLine(valX + 50); ImGui::TextUnformatted("Last Call");
                    ImGui::PopStyleColor();
                    ImGui::Separator();

                    auto apiRow = [&](const char* name, const Api::Status::EndpointStatus& st) {
                        ImGui::PushStyleColor(ImGuiCol_Text, COL_MUTED);
                        ImGui::TextUnformatted(name);
                        ImGui::PopStyleColor();
                        ImGui::SameLine(valX);
                        if (!st.everCalled) {
                            ImGui::TextColored(COL_MUTED, "-");
                        } else {
                            ImGui::TextColored(statusCol(st.code), "%d", st.code);
                        }
                        ImGui::SameLine(valX + 50);
                        ImGui::TextColored(COL_MUTED, "%s", fmtTime(st.everCalled ? st.lastCall : std::chrono::system_clock::time_point{}).c_str());
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

                // DATA
                if (ImGui::CollapsingHeader("  DATA"))
                {
                    ImGui::Spacing();
                    dbgRow("Username", Core::Config::user.username.empty() ? "-" : Core::Config::user.username.c_str());

                    std::shared_lock<std::shared_mutex> dataLock(Core::Config::data::dataMutex);
                    char tmp[16]; snprintf(tmp, sizeof(tmp), "%d", (int)Core::Config::data::arr.size());
                    dbgRow("Entries", tmp);
                    ImGui::Spacing();

                    const int fmtSize = (int)Core::Config::data::arrFormatted.size();
                    for (size_t j = 0; j < Core::Config::data::arr.size(); j++) {
                        const auto& raw = Core::Config::data::arr[j];
                        if (ImGui::CollapsingHeader(raw.key.c_str())) {
                            if ((int)j >= fmtSize) continue;
                            const auto& fmt = Core::Config::data::arrFormatted[j];
                            dbgRow("init",          raw.init.empty()     ? "-" : raw.init.c_str());
                            dbgRow("init (fmt)",    fmt.init.empty()     ? "-" : fmt.init.c_str());
                            dbgRow("current",       raw.current.empty()  ? "-" : raw.current.c_str());
                            dbgRow("current (fmt)", fmt.current.empty()  ? "-" : fmt.current.c_str());
                            dbgRow("change",        raw.change.empty()   ? "-" : raw.change.c_str());
                            dbgRow("change (fmt)",  fmt.change.empty()   ? "-" : fmt.change.c_str());
                            char flags[64];
                            snprintf(flags, sizeof(flags), "bancho=%s  titanic=%s  display=%s",
                                raw.banchoSupport  ? "y":"n",
                                raw.titanicSupport ? "y":"n",
                                raw.display        ? "y":"n");
                            dbgRow("flags", flags);
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

        // Render
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