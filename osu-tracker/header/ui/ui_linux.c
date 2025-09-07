/* nuklear - v1.32.0 - public domain */
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
#include <nuklear/glfw3.h>

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


enum gameMode {
    osu = 0
    , taiko = 1
    , fruits = 2
    , mania = 3
};

enum server {
    bancho = 0
    , titanic = 1
};


struct appC {
    int osuId;
    enum gameMode gameMode;
    enum server server;
};

struct userC {
    const char* username;
    const char* avatar;
};

struct dataEntryC {
    const char* key;
    const char* name;
    int sort;
    const char* init;
    const char* current;
    const char* change;
    bool positive;
    bool display;
    bool single;
    bool banchoSupport;
    bool titanicSupport;
};

// Internal storage
static struct appC _app;
static struct userC _user;
static struct dataEntryC _entries[MAX_ENTRIES];
static size_t _entry_count = 0;

// Track allocated strings for cleanup
static char* _user_str_copies[2] = { NULL, NULL }; // username, avatar
static char* _entries_str_copies[MAX_ENTRIES * 5]; // 5 strings per entry
static size_t _entries_str_copies_count = 0;

static char* strdup_safe(const char* src) {
    if (!src) return NULL;
    size_t len = strlen(src);
    char* dst = malloc(len + 1);
    if (!dst) return NULL;
    memcpy(dst, src, len + 1);
    return dst;
}

static void free_internal_copies() {
    for (size_t i = 0; i < _entries_str_copies_count; i++) {
        free(_entries_str_copies[i]);
    }
    _entries_str_copies_count = 0;

    if (_user_str_copies[0]) { free(_user_str_copies[0]); _user_str_copies[0] = NULL; }
    if (_user_str_copies[1]) { free(_user_str_copies[1]); _user_str_copies[1] = NULL; }
}

static void copy_data_entry_deep(const struct dataEntryC* src, struct dataEntryC* dst) {
    dst->sort = src->sort;
    dst->positive = src->positive;
    dst->display = src->display;
    dst->single = src->single;
    dst->banchoSupport = src->banchoSupport;
    dst->titanicSupport = src->titanicSupport;

    // List of string fields to copy
    const char* src_fields[] = { src->key, src->name, src->init, src->current, src->change };
    const char** dst_fields[] = { &dst->key, &dst->name, &dst->init, &dst->current, &dst->change };

    for (int i = 0; i < 5; i++) {
        char* copy = strdup_safe(src_fields[i]);
        if (!copy) {
            fprintf(stderr, "Out of memory copying string field\n");
            exit(1);
        }
        _entries_str_copies[_entries_str_copies_count++] = copy;
        *dst_fields[i] = copy;
    }
}

void copyArrayData(const struct appC* app, const struct userC* user, const struct dataEntryC* entries, size_t count) {
    if (count > MAX_ENTRIES) count = MAX_ENTRIES;

    free_internal_copies();

    _app = *app;

    _user_str_copies[0] = strdup_safe(user->username);
    _user_str_copies[1] = strdup_safe(user->avatar);
    _user.username = _user_str_copies[0];
    _user.avatar = _user_str_copies[1];

    _entry_count = count;

    for (size_t i = 0; i < count; i++) {
        copy_data_entry_deep(&entries[i], &_entries[i]);
    }
}

bool show_debug_layout = false;
bool data_debug_layout = false;


#include <nuklear/nuklear.h>
#include <nuklear/nuklear_glfw_gl2.h>

#define STB_IMAGE_IMPLEMENTATION
#include <nuklear/stb_image.h>

static void error_callback(int e, const char *d)
{printf("Error %d: %s\n", e, d);}
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

    /* GUI */
    struct nk_context *ctx;
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

    /* GUI */
    ctx = nk_glfw3_init(win, NK_GLFW3_INSTALL_CALLBACKS);
    
    // Fonts
    struct nk_font_atlas *atlas;
    nk_glfw3_font_stash_begin(&atlas);
    struct nk_font *fontDefault;
    struct nk_font *fontHeader;
    struct nk_font *fontSmall;
    // Load fonts with different sizes
    fontHeader  = nk_font_atlas_add_default(atlas, 22.0f, 0);
    fontDefault = nk_font_atlas_add_default(atlas, 16.0f, 0);
    fontSmall   = nk_font_atlas_add_default(atlas, 16.0f, 0);

    nk_glfw3_font_stash_end();    

    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

    nk_font_atlas_add_default(atlas, 20.0f, 0);

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
        glfwGetWindowSize(win, &width, &height);
        glViewport(0, 0, width, height);
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
