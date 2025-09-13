#define MAX_ENTRIES 100
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 600

int width = WINDOW_WIDTH;
int height = WINDOW_HEIGHT;
int nk_width = WINDOW_WIDTH;
int nk_height = WINDOW_HEIGHT;

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
    if (!src || !dst) return;

    *dst = (struct dataEntryC){ 0 }; // zero all fields

    dst->sort = src->sort;
    dst->positive = src->positive;
    dst->display = src->display;
    dst->single = src->single;
    dst->banchoSupport = src->banchoSupport;
    dst->titanicSupport = src->titanicSupport;

    const char* src_fields[] = { src->key, src->name, src->init, src->current, src->change };
    const char** dst_fields[] = { &dst->key, &dst->name, &dst->init, &dst->current, &dst->change };

    for (int i = 0; i < 5; i++) {
        if (_entries_str_copies_count >= MAX_ENTRIES * 5) {
            fprintf(stderr, "Too many entries for internal copy buffer\n");
            exit(1);
        }
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