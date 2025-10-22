#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl2.h>
#include <stdio.h>
#include <GLFW/glfw3.h>
#include "../header/config.h"
#include "droid_sans.h"

static GLFWwindow* window;
#define WINDOW_WIDTH 480
#define WINDOW_HEIGHT 800

// Layout ratios
static float ratio[]       = { 0.3f, 0.35f, 0.35f };
static float ratioSingle[] = { 0.3f, 0.7f };
static float ratioEqual[]  = { 0.5f, 0.5f };

void ui_mainTerminate() {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int ui_main()
{

#ifdef __linux__
    // Somehow crashes on wayland with wayland support, fallback to x11
    setenv("XDG_SESSION_TYPE", "x11", 1);
    setenv("WAYLAND_DISPLAY", "", 1);   // prevent Wayland backend
#endif

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return 1;
    }

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, OSU_TRACKER_TITLE, nullptr, nullptr);

    if (window == nullptr) {
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, 400, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = "imgui.ini";

    ImGui::StyleColorsDark();

    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;

    ImFont* fontDefault = io.Fonts->AddFontFromMemoryTTF((void*)DroidSans_ttf, sizeof(DroidSans_ttf), 18.0f, &font_cfg);
    ImFont* fontHeader  = io.Fonts->AddFontFromMemoryTTF((void*)DroidSans_ttf, sizeof(DroidSans_ttf), 24.0f, &font_cfg);
    ImFont* fontSmall   = io.Fonts->AddFontFromMemoryTTF((void*)DroidSans_ttf, sizeof(DroidSans_ttf), 14.0f, &font_cfg);

    if (!fontDefault || !fontHeader || !fontSmall) {
        printf("Failed to load one or more fonts!\n");
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    ImVec4 bg = ImVec4(0.10f, 0.18f, 0.24f, 1.0f);

    int display_w = WINDOW_WIDTH, display_h = WINDOW_HEIGHT;
    // Main loop
    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll;
    ImGuiWindowFlags beginFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &display_w, &display_h);
        bool window_active = !(glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0 ||
                               glfwGetWindowAttrib(window, GLFW_FOCUSED) == 0);

        glfwPollEvents();

        if (!window_active)
        {
            // throttle CPU usage when inactive
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Toggle debug overlay once per key press
        static bool debug = false;
        static bool f10_pressed_last = false;
        bool f10_pressed = glfwGetKey(window, GLFW_KEY_F10) == GLFW_PRESS;
        if (f10_pressed && !f10_pressed_last) debug = !debug;
        f10_pressed_last = f10_pressed;

        // Set window size and position (full application window)
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(display_w, display_h));

        // Push fonts and styling
        ImGui::PushFont(fontHeader);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(35/255.f, 35/255.f, 35/255.f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(21/255.f, 133/255.f, 181/255.f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(13/255.f, 13/255.f, 13/255.f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(1, 1, 1, 1));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(1, 1, 1, 1));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0,0,0,1));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(1,1,1,1));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(1,1,1,1));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(1,1,1,1));

        ImGui::GetStyle().WindowBorderSize = 1.0f;
        ImGui::GetStyle().FrameBorderSize = 1.0f;
        //ImGui::GetStyle().FrameRounding = 0.0f;
        ImGui::GetStyle().WindowPadding = ImVec2(10, 10);
        ImGui::GetStyle().ItemSpacing = ImVec2(1, 4);
        ImGui::GetStyle().ItemInnerSpacing = ImVec2(5, 4);
        ImGui::GetStyle().ScrollbarSize = 8.0f;
        ImGui::GetStyle().FrameRounding = 5.0f;
        ImGui::GetStyle().FramePadding = ImVec2(6, 4);
        //ImGui::GetStyle().Colors[]

        // Begin window
        if (ImGui::Begin(
            config::user.username.empty() ? "'No User'" : config::user.username.c_str()
            ,nullptr
            ,beginFlags
        )){
            ImGui::PopFont();
            ImGui::PushFont(fontDefault);

            float* ratioPtr = nullptr;
            int nCols = 0;

            int i = -1;
            for (auto& entry : config::data::arrFormatted)
            {
                i++;
                if (!entry.display) continue;

                switch (config::application.server) {
                    case config::server::bancho: {
                        if (!entry.banchoSupport) continue;
                        break;
                    }
                    case config::server::titanic: {
                        if (!entry.titanicSupport) continue;
                        break;
                    }
                }

                if (entry.single) {
                    ratioPtr = ratioSingle;
                    nCols = 2;
                } else {
                    ratioPtr = ratio;
                    nCols = 3;
                }

                // Setup columns
                ImGui::Columns(nCols, nullptr, false);

                float totalWidth = display_w - ImGui::GetStyle().WindowPadding.x*2;
                for (int c = 0; c < nCols; ++c)
                    ImGui::SetColumnWidth(c, totalWidth * ratioPtr[c]);

                // Name
                ImGui::Text("%s", entry.name.empty() ? "##ERROR" : entry.name.c_str());
                ImGui::NextColumn();

                ImGui::PushItemWidth(-1); // fill column
                ImGui::InputText(("##current" + std::to_string(i)).c_str(), &entry.current, inputFlags);
                ImGui::PopItemWidth();
                ImGui::NextColumn();

                if (!entry.single)
                {
                    ImGui::PushItemWidth(-1);
                    ImGui::InputText(("##change" + std::to_string(i)).c_str(), &entry.change, inputFlags);
                    ImGui::PopItemWidth();
                    ImGui::NextColumn();
                }

                ImGui::Columns(1);
            }

            ImGui::PopFont();
        }

        ImGui::End();
        ImGui::PopStyleColor(10);


        // Render debug overlay if enabled
        static bool showDataDebug   = false; // starts collapsed
        static bool showRenderDebug = false; // starts collapsed

        if (debug) {
            ImGui::SetNextWindowPos(ImVec2(0,0));
            ImGui::SetNextWindowSize(ImVec2(display_w, display_h));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.9f));
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f,0.6f,0.8f,1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10,10));

            ImGuiWindowFlags debugFlags = ImGuiWindowFlags_NoTitleBar |
                                          ImGuiWindowFlags_NoResize |
                                          ImGuiWindowFlags_NoMove |
                                          ImGuiWindowFlags_NoSavedSettings;

            if (ImGui::Begin("Debug Overlay", nullptr, debugFlags))
            {
                // Rendering info section, start collapsed
                if (ImGui::CollapsingHeader("Render Debug"))
                {
                    ImGui::Columns(2, nullptr, false);
                    ImGui::Text("FPS"); ImGui::NextColumn();
                    ImGui::Text("%.1f", io.Framerate); ImGui::NextColumn();

                    ImGui::Text("ImGui Version"); ImGui::NextColumn();
                    ImGui::Text("%s", IMGUI_VERSION); ImGui::NextColumn();

                    // ---- GLFW Info ----
                    int fbWidth, fbHeight;
                    glfwGetFramebufferSize(glfwGetCurrentContext(), &fbWidth, &fbHeight);
                    ImGui::Text("GLFW Framebuffer W"); ImGui::NextColumn();
                    ImGui::Text("%d", fbWidth); ImGui::NextColumn();

                    ImGui::Text("GLFW Framebuffer H"); ImGui::NextColumn();
                    ImGui::Text("%d", fbHeight); ImGui::NextColumn();

                    int winW, winH;
                    glfwGetWindowSize(glfwGetCurrentContext(), &winW, &winH);
                    ImGui::Text("Window W"); ImGui::NextColumn();
                    ImGui::Text("%d", winW); ImGui::NextColumn();

                    ImGui::Text("Window H"); ImGui::NextColumn();
                    ImGui::Text("%d", winH); ImGui::NextColumn();

                    ImGui::Text("Viewport"); ImGui::NextColumn();
                    GLint viewport[4];
                    glGetIntegerv(GL_VIEWPORT, viewport);
                    ImGui::Text("%d,%d - %d x %d", viewport[0], viewport[1], viewport[2], viewport[3]); ImGui::NextColumn();


                    // ---- OpenGL Info ----
                    const GLubyte* renderer = glGetString(GL_RENDERER);
                    const GLubyte* vendor   = glGetString(GL_VENDOR);
                    const GLubyte* version  = glGetString(GL_VERSION);
                    const GLubyte* glslVer  = glGetString(GL_SHADING_LANGUAGE_VERSION);

                    GLint maxViewportDims[2];
                    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxViewportDims);

                    ImGui::Text("GPU Vendor"); ImGui::NextColumn();
                    ImGui::Text("%s", vendor); ImGui::NextColumn();

                    ImGui::Text("GPU Renderer"); ImGui::NextColumn();
                    ImGui::Text("%s", renderer); ImGui::NextColumn();

                    ImGui::Text("OpenGL Version"); ImGui::NextColumn();
                    ImGui::Text("%s", version); ImGui::NextColumn();

                    ImGui::Text("GLSL Version"); ImGui::NextColumn();
                    ImGui::Text("%s", glslVer); ImGui::NextColumn();

                    ImGui::Columns(1);
                }

                // Data debug section, start collapsed
                if (ImGui::CollapsingHeader("Data Debug"))
                {
                    ImGui::Columns(2, nullptr, false);
                    ImGui::Text("Username"); ImGui::NextColumn();
                    ImGui::Text("%s", config::user.username.c_str()); ImGui::NextColumn();

                    ImGui::Text("Entries"); ImGui::NextColumn();
                    ImGui::Text("%d", (int)config::data::arr.size()); ImGui::NextColumn();
                    ImGui::Columns(1);
                    for (size_t i = 0; i < config::data::arr.size(); i++) {
                        if (ImGui::CollapsingHeader(config::data::arr[i].key.c_str()))
                        {
                            config::dataEntry _entry = config::data::arr[i];
                            config::dataEntry _entryFormatted = config::data::arrFormatted[i];

                            ImGui::Columns(2, nullptr, false);
                            ImGui::Text("Key"); ImGui::NextColumn();
                            ImGui::Text(_entry.key.c_str()); ImGui::NextColumn();
                            ImGui::Text("Init"); ImGui::NextColumn();
                            ImGui::Text(_entry.init.c_str()); ImGui::NextColumn();
                            ImGui::Text("Init (Format)"); ImGui::NextColumn();
                            ImGui::Text(_entryFormatted.init.c_str()); ImGui::NextColumn();
                            ImGui::Text("Current"); ImGui::NextColumn();
                            ImGui::Text(_entry.current.c_str()); ImGui::NextColumn();
                            ImGui::Text("Current (Format)"); ImGui::NextColumn();
                            ImGui::Text(_entryFormatted.current.c_str()); ImGui::NextColumn();
                            ImGui::Text("Change"); ImGui::NextColumn();
                            ImGui::Text(_entry.change.c_str()); ImGui::NextColumn();
                            ImGui::Text("Change (Format)"); ImGui::NextColumn();
                            ImGui::Text(_entryFormatted.change.c_str()); ImGui::NextColumn();

                            ImGui::Columns(1);
                            ImGui::Text("Bancho Support: %s", _entry.banchoSupport ? "true" : "false"); ImGui::NextColumn();
                            ImGui::Text("Titanic Support: %s", _entry.titanicSupport ? "true" : "false"); ImGui::NextColumn();

                        }
                    }
                }
            }

            ImGui::End();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(3);
        }


        // Render
        ImGui::Render();
        //glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(bg.x, bg.y, bg.z, bg.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    glfwDestroyWindow(window);   // destroy window first
    ImGui::DestroyContext();     // then free ImGui

    glfwTerminate();             // finally terminate GLFW
    return 0;
}
