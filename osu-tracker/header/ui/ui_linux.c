#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

#define MAX_ENTRIES 100
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 600
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL2_IMPLEMENTATION

#include "ui_global.h"

bool show_debug_layout = false;
bool data_debug_layout = false;


#include <nuklear/nuklear.h>
#include <nuklear/nuklear_glfw_gl2.h>
#include "droid_sans.h"

#define STB_IMAGE_IMPLEMENTATION
#include <nuklear/stb_image.h>

static void error_callback(int e, const char* d)
{
    printf("Error %d: %s\n", e, d);
}
int w = WINDOW_WIDTH;
int h = WINDOW_HEIGHT;

#include "draw.h"

static GLFWwindow* win;

void ui_mainTerminate() {
    glfwSetWindowShouldClose(win, GLFW_TRUE);
}

int ui_main(void)
{
    /* Platform */
    int width = 0, height = 0;
    int fb_width = 0, fb_height = 0;

    /* GUI */
    struct nk_context* ctx;
    struct nk_colorf bg;

#ifdef INCLUDE_CONFIGURATOR
    static struct nk_color color_table[NK_COLOR_COUNT];
    memcpy(color_table, nk_default_color_style, sizeof(color_table));
#endif

    /* GLFW */
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        fprintf(stdout, "[GFLW] failed to init!\n");
        exit(1);
    }
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, OSU_TRACKER_TITLE, NULL, NULL);
    glfwMakeContextCurrent(win);
    glfwGetWindowSize(win, &width, &height);

    glfwGetFramebufferSize(win, &fb_width, &fb_height);

    /* GUI */
    ctx = nk_glfw3_init(win, NK_GLFW3_INSTALL_CALLBACKS);

    // Fonts
    struct nk_font_atlas* atlas;
    nk_glfw3_font_stash_begin(&atlas);

    struct nk_font* fontDefault = nk_font_atlas_add_from_memory(
        atlas,
        DroidSans_ttf,          // pointer to embedded array
        DroidSans_ttf_len,      // length of the array
        18.0f,                  // font size
        0                       // optional nk_font_config*
    );

    struct nk_font* fontHeader = nk_font_atlas_add_from_memory(
        atlas,
        DroidSans_ttf,          // pointer to embedded array
        DroidSans_ttf_len,      // length of the array
        24.0f,                  // font size
        0                       // optional nk_font_config*
    );

    struct nk_font* fontSmall = nk_font_atlas_add_from_memory(
        atlas,
        DroidSans_ttf,          // pointer to embedded array
        DroidSans_ttf_len,      // length of the array
        18.0f,                  // font size
        0                       // optional nk_font_config*
    );

    nk_glfw3_font_stash_end();
    nk_style_set_font(ctx, &fontDefault->handle);

    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

    while (!glfwWindowShouldClose(win))
    {
        /* Input */
        glfwPollEvents();
        nk_glfw3_new_frame();
        static int f10_was_down = 0;
        static int f11_was_down = 0;

        int f10_now = glfwGetKey(win, GLFW_KEY_F10) == GLFW_PRESS;
        int f11_now = glfwGetKey(win, GLFW_KEY_F11) == GLFW_PRESS;

        if (f10_now && !f10_was_down) {
            show_debug_layout = !show_debug_layout;
        }
        if (f11_now && !f11_was_down) {
            data_debug_layout = !data_debug_layout;
        }
        f10_was_down = f10_now;
        f11_was_down = f11_now;
        // ui
        nk_style_default(ctx);
        drawContent(ctx, fontDefault, fontSmall, fontHeader, w, h, _app, _user, _entries, _entry_count, show_debug_layout, data_debug_layout);
        nk_end(ctx);

        /* Draw */
        /* Update framebuffer size each frame */
        glfwGetWindowSize(win, &width, &height);
        glfwGetFramebufferSize(win, &fb_width, &fb_height);
        glViewport(0, 0, fb_width, fb_height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(bg.r, bg.g, bg.b, bg.a);
        /* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
         * with blending, scissor, face culling and depth test and defaults everything
         * back into a default state. Make sure to either save and restore or
         * reset your own state after drawing rendering the UI. */
        nk_glfw3_render(NK_ANTI_ALIASING_OFF);
        glfwSwapBuffers(win);
    }
    nk_glfw3_shutdown();
    glfwTerminate();
    return 0;
}