#include "raylib.h"
#include "rlgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include "WzGuiCore.h"
#include "WzLayoutScalar.h"
#define __ISPC_STRUCT_WzChunk__
#include "WzLayout_ispc.h"
#include "WzEngine.h"
#ifdef _WIN32
#include "WzResizeHook.h"
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

// HiDPI / device pixel ratio helpers.
// On displays with Windows scaling > 100% (e.g. 150%), devicePixelRatio = 1.5.
// The canvas drawing buffer must be sized in physical pixels (css * dpr) so the
// GPU renders at full resolution; the CSS display size stays at logical pixels so
// the browser doesn't stretch it a second time.
EM_JS(double, get_device_pixel_ratio, (void), {
    return window.devicePixelRatio || 1.0;
});
EM_JS(int, get_browser_width, (void), {
    return window.innerWidth;   // logical (CSS) pixels
});
EM_JS(int, get_browser_height, (void), {
    return window.innerHeight;  // logical (CSS) pixels
});

// Mouse tracker — bypasses raylib's broken mouse coords on HiDPI web builds.
//
// Root cause: raylib uses Emscripten's USE_GLFW=3 backend, which calls
// glfwGetCursorPos(). On HiDPI displays that function returns coordinates
// already multiplied by devicePixelRatio, putting them in physical-pixel space.
// But the canvas drawing buffer is also in physical-pixel space, so raylib's
// GetMousePosition() ends up returning values scaled by DPR a second time —
// effectively (clientX * dpr * dpr), which places the hit-test area far to the
// bottom-right of the actual cursor.
//
// Fix: attach a 'mousemove' listener directly to the canvas DOM element and
// compute the correct canvas-buffer coordinates ourselves:
//   canvasX = (clientX - canvasRect.left) * (canvas.width / canvasRect.width)
// canvas.width is the physical-pixel drawing-buffer width.
// canvasRect.width is the CSS display width (logical pixels).
// The ratio converts logical mouse coords into drawing-buffer coords exactly.
// These values are stored in Module._trueMouseX/Y and read each frame via
// get_true_mouse_x() / get_true_mouse_y() instead of GetMousePosition().
EM_JS(void, install_mouse_tracker, (void), {
    Module._trueMouseX = 0;
    Module._trueMouseY = 0;
    var canvas = Module.canvas;
    canvas.addEventListener('mousemove', function(e) {
        var rect = canvas.getBoundingClientRect();
        Module._trueMouseX = (e.clientX - rect.left) * (canvas.width / rect.width);
        Module._trueMouseY = (e.clientY - rect.top) * (canvas.height / rect.height);
    });
});
EM_JS(double, get_true_mouse_x, (void), { return Module._trueMouseX || 0; });
EM_JS(double, get_true_mouse_y, (void), { return Module._trueMouseY || 0; });

// Force nearest-neighbor rendering on the canvas and log diagnostic info.
// Without image-rendering: pixelated, the browser may bilinearly interpolate
// the canvas output even at (or near) 1:1 device pixel mapping.
EM_JS(void, fix_canvas_display, (void), {
    var c = Module.canvas;
    c.style.imageRendering = 'pixelated';
});

// Returns 1 if WebGL antialias is on, 0 if off, -1 if unknown
EM_JS(int, get_webgl_antialias, (void), {
    var c = Module.canvas;
    var gl = c.getContext('webgl2');
    if (!gl) gl = c.getContext('webgl');
    if (gl) {
        var a = gl.getContextAttributes();
        return a.antialias ? 1 : 0;
    }
    return -1;
});

#endif

#ifdef __EMSCRIPTEN__
// Current device pixel ratio and logical canvas size, updated every frame.
// The GUI runs in logical pixels; the renderer scales vertices × g_dpr so the
// output fills the physical-pixel canvas — replicating what Windows does when
// it bilinearly upscales a DPI-unaware app's framebuffer to the display.
static float g_dpr   = 1.0f;
static int   g_css_w = 960;
static int   g_css_h = 540;
#endif

// ---- Widget IDs ----
#define CONCAT(a, b) a##b
#define ITEM(name, num) CONCAT(name, num),
#define GEN_2(n, o0, o1) ITEM(n, o0) ITEM(n, o1)
#define GEN_4(n, o0, o1, o2, o3) GEN_2(n, o0, o1) GEN_2(n, o2, o3)
#define B000(n, p) ITEM(n, p##0) ITEM(n, p##1) ITEM(n, p##2) ITEM(n, p##3) ITEM(n, p##4) ITEM(n, p##5) ITEM(n, p##6) ITEM(n, p##7)
#define B00(n, p)  B000(n, p##0) B000(n, p##1) B000(n, p##2) B000(n, p##3) B000(n, p##4) B000(n, p##5) B000(n, p##6) B000(n, p##7)
#define GEN_512_SAFE(n) B00(n,0) B00(n,1) B00(n,2) B00(n,3) B00(n,4) B00(n,5) B00(n,6) B00(n,7)

enum {
    WIDGET_INPUT_BOX1 = 1,
    WIDGET_INPUT_BOX2,
    WIDGET_INPUT_BOX3,
    WIDGET_DROPDOWN1,
    WIDGET_VBOX1,
    WIDGET_LABEL1,
    GEN_512_SAFE(WIDGET_LABEL_LIST)
};

// ---- Engine context ----
typedef struct {
    unsigned int  white_tex_id;
    WzGui         gui;
} WzContext;

static WzContext ctx;

// ---- Engine app ----
static WzApp* _wz_app = NULL;
void wz_engine_set_app(WzApp* app) { _wz_app = app; }

// ---- Viewport render texture ----
static RenderTexture2D _viewport_rt = { 0 };
static int             _viewport_rt_w = 0;
static int             _viewport_rt_h = 0;

static RenderTexture2D get_or_resize_viewport_rt(int w, int h)
{
    if (_viewport_rt.id > 0 && _viewport_rt_w == w && _viewport_rt_h == h)
        return _viewport_rt;
    if (_viewport_rt.id > 0)
        UnloadRenderTexture(_viewport_rt);
    _viewport_rt   = LoadRenderTexture(w, h);
    _viewport_rt_w = w;
    _viewport_rt_h = h;
    return _viewport_rt;
}

WzWidget wz_viewport(WzWidget parent, int w, int h, unsigned id)
{
    RenderTexture2D rt = get_or_resize_viewport_rt(w, h);
    WzTexture wz_tex = {
        .data  = (void*)(uintptr_t)rt.texture.id,
        .w     = (float)w,  .h     = (float)h,
        .tex_w = (float)w,  .tex_h = (float)h,
    };
    return wz_texture_raw(parent, wz_tex, (unsigned)w, (unsigned)h, __FILE__, __LINE__);
}

// ---- Font atlas upload ----
static unsigned int upload_alpha_atlas(const unsigned char* bitmap, int w, int h)
{
    unsigned char* rgba = (unsigned char*)malloc(w * h * 4);
    for (int i = 0; i < w * h; i++) {
        rgba[i*4+0] = rgba[i*4+1] = rgba[i*4+2] = 255;
        rgba[i*4+3] = bitmap[i];
    }
    Image img = {
        .data    = rgba,
        .width   = w,
        .height  = h,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1,
    };
    Texture2D tex = LoadTextureFromImage(img);
    // POINT (nearest-neighbor): the font atlas is baked at physical pixel size
    // and glyph quads map exactly 1:1 to device pixels after vertex scaling,
    // so no interpolation is needed. POINT gives sharp, aliasing-free sampling.
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    free(rgba);
    return tex.id;
}

// ---- Timing shim (WzGui expects uint64_t function pointers) ----
static unsigned long wz_ticks_ns(void) { return (unsigned long)(GetTime() * 1e9); }
static unsigned long wz_ticks_ms(void) { return (unsigned long)(GetTime() * 1e3); }

// ---- Performance averaging ----
static float perf_avg(float* samples, unsigned* index, float new_val)
{
    samples[*index % 60] = new_val;
    (*index)++;
    float sum = 0.0f;
    unsigned count = *index < 60 ? *index : 60;
    for (unsigned i = 0; i < count; ++i) sum += samples[i];
    return sum / (float)count;
}

// ---- Key translation: Raylib KEY_* → Wz WZ_KEY_* ----
static unsigned raylib_key_to_wz(int key)
{
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    // Letters — printable letters are handled by GetCharPressed; this covers special keys
    if (key >= KEY_A && key <= KEY_Z) {
        char base = 'a' + (key - KEY_A);
        if (shift) return (unsigned)(base - 32);
        return (unsigned)base;
    }
    // Numbers
    if (key >= KEY_ZERO && key <= KEY_NINE) {
        if (shift) {
            const char sym[] = ")!@#$%^&*(";
            return (unsigned)sym[key - KEY_ZERO];
        }
        return (unsigned)('0' + (key - KEY_ZERO));
    }
    // Printable punctuation
    switch (key) {
    case KEY_SPACE:         return ' ';
    case KEY_ENTER:         return '\n';
    case KEY_ESCAPE:        return 27;
    case KEY_BACKSPACE:     return '\b';
    case KEY_TAB:           return '\t';
    case KEY_MINUS:         return shift ? '_' : '-';
    case KEY_EQUAL:         return shift ? '+' : '=';
    case KEY_LEFT_BRACKET:  return shift ? '{' : '[';
    case KEY_RIGHT_BRACKET: return shift ? '}' : ']';
    case KEY_BACKSLASH:     return shift ? '|' : '\\';
    case KEY_SEMICOLON:     return shift ? ':' : ';';
    case KEY_APOSTROPHE:    return shift ? '"' : '\'';
    case KEY_GRAVE:         return shift ? '~' : '`';
    case KEY_COMMA:         return shift ? '<' : ',';
    case KEY_PERIOD:        return shift ? '>' : '.';
    case KEY_SLASH:         return shift ? '?' : '/';
    // Extended keys
    case KEY_CAPS_LOCK:     return WZ_KEY_CAPSLOCK;
    case KEY_F1:            return WZ_KEY_F1;
    case KEY_F2:            return WZ_KEY_F2;
    case KEY_F3:            return WZ_KEY_F3;
    case KEY_F4:            return WZ_KEY_F4;
    case KEY_F5:            return WZ_KEY_F5;
    case KEY_F6:            return WZ_KEY_F6;
    case KEY_F7:            return WZ_KEY_F7;
    case KEY_F8:            return WZ_KEY_F8;
    case KEY_F9:            return WZ_KEY_F9;
    case KEY_F10:           return WZ_KEY_F10;
    case KEY_F11:           return WZ_KEY_F11;
    case KEY_F12:           return WZ_KEY_F12;
    case KEY_PRINT_SCREEN:  return WZ_KEY_PRINTSCREEN;
    case KEY_SCROLL_LOCK:   return WZ_KEY_SCROLLLOCK;
    case KEY_PAUSE:         return WZ_KEY_PAUSE;
    case KEY_INSERT:        return WZ_KEY_INSERT;
    case KEY_HOME:          return WZ_KEY_HOME;
    case KEY_PAGE_UP:       return WZ_KEY_PAGEUP;
    case KEY_DELETE:        return WZ_KEY_DELETE;
    case KEY_END:           return WZ_KEY_END;
    case KEY_PAGE_DOWN:     return WZ_KEY_PAGEDOWN;
    case KEY_RIGHT:         return WZ_KEY_RIGHT;
    case KEY_LEFT:          return WZ_KEY_LEFT;
    case KEY_DOWN:          return WZ_KEY_DOWN;
    case KEY_UP:            return WZ_KEY_UP;
    case KEY_KP_DIVIDE:     return WZ_KEY_KP_DIVIDE;
    case KEY_KP_MULTIPLY:   return WZ_KEY_KP_MULTIPLY;
    case KEY_KP_SUBTRACT:   return WZ_KEY_KP_MINUS;
    case KEY_KP_ADD:        return WZ_KEY_KP_PLUS;
    case KEY_KP_ENTER:      return WZ_KEY_KP_ENTER;
    case KEY_KP_EQUAL:      return WZ_KEY_KP_EQUALS;
    case KEY_LEFT_CONTROL:  return WZ_KEY_LCTRL;
    case KEY_LEFT_SHIFT:    return WZ_KEY_LSHIFT;
    case KEY_LEFT_ALT:      return WZ_KEY_LALT;
    case KEY_LEFT_SUPER:    return WZ_KEY_LGUI;
    case KEY_RIGHT_CONTROL: return WZ_KEY_RCTRL;
    case KEY_RIGHT_SHIFT:   return WZ_KEY_RSHIFT;
    case KEY_RIGHT_ALT:     return WZ_KEY_RALT;
    case KEY_RIGHT_SUPER:   return WZ_KEY_RGUI;
    default:                return WZ_KEY_UNKNOWN;
    }
}

static unsigned short get_wz_modifiers(void)
{
    unsigned short mod = WZ_KMOD_NONE;
    if (IsKeyDown(KEY_LEFT_SHIFT))    mod |= WZ_KMOD_LSHIFT;
    if (IsKeyDown(KEY_RIGHT_SHIFT))   mod |= WZ_KMOD_RSHIFT;
    if (IsKeyDown(KEY_LEFT_CONTROL))  mod |= WZ_KMOD_LCTRL;
    if (IsKeyDown(KEY_RIGHT_CONTROL)) mod |= WZ_KMOD_RCTRL;
    if (IsKeyDown(KEY_LEFT_ALT))      mod |= WZ_KMOD_LALT;
    if (IsKeyDown(KEY_RIGHT_ALT))     mod |= WZ_KMOD_RALT;
    if (IsKeyDown(KEY_LEFT_SUPER))    mod |= WZ_KMOD_LGUI;
    if (IsKeyDown(KEY_RIGHT_SUPER))   mod |= WZ_KMOD_RGUI;
    return mod;
}

#define MAX_NUM_EVENTS 128

static int build_wz_events(WzEvent* events, int max)
{
    int count = 0;
    unsigned short mod = get_wz_modifiers();

    // Drain key-pressed queue (includes special keys + letters/numbers)
    int rl_key;
    while ((rl_key = GetKeyPressed()) != 0 && count < max) {
        unsigned wz_key = raylib_key_to_wz(rl_key);
        if (wz_key != WZ_KEY_UNKNOWN) {
            events[count++].key = (WzKeyboardEvent){
                .type   = WZ_EVENT_TYPE_KEYBOARD,
                .key    = wz_key,
                .down   = true,
                .repeat = false,
                .mod    = mod,
            };
        }
    }

    // Drain char-pressed queue for printable text (correct unicode with shift/caps)
    int ch;
    while ((ch = GetCharPressed()) != 0 && count < max) {
        // Only add if not already covered by key-pressed above (printable ASCII)
        // GetCharPressed gives us the correct shifted character
        if (ch >= 32 && ch <= 126) {
            events[count++].key = (WzKeyboardEvent){
                .type = WZ_EVENT_TYPE_KEYBOARD,
                .key  = (unsigned)ch,
                .down = true,
                .mod  = mod,
            };
        }
    }

    // Mouse buttons
    Vector2 mp = GetMousePosition();
#ifdef __EMSCRIPTEN__
    // JS tracker returns physical pixel coords; divide by DPR to get logical
    // pixels matching the GUI coordinate space. See install_mouse_tracker().
    mp.x = (float)get_true_mouse_x() / g_dpr;
    mp.y = (float)get_true_mouse_y() / g_dpr;
#endif
    struct { int rl; int wz; } btns[] = {
        { MOUSE_BUTTON_LEFT,   1 },
        { MOUSE_BUTTON_MIDDLE, 2 },
        { MOUSE_BUTTON_RIGHT,  3 },
    };
    for (int b = 0; b < 3 && count < max; b++) {
        if (IsMouseButtonPressed(btns[b].rl)) {
            wz_input_mouse_button_event(btns[b].wz, true, mp.x, mp.y);
            events[count++].button = (WzButtonEvent){
                .type = WZ_EVENT_TYPE_MOUSE, .button = btns[b].wz,
                .down = true, .x = mp.x, .y = mp.y,
            };
        }
        if (IsMouseButtonReleased(btns[b].rl) && count < max) {
            wz_input_mouse_button_event(btns[b].wz, false, mp.x, mp.y);
            events[count++].button = (WzButtonEvent){
                .type = WZ_EVENT_TYPE_MOUSE, .button = btns[b].wz,
                .down = false, .x = mp.x, .y = mp.y,
            };
        }
    }

    return count;
}

// ---- GUI draw list submission via rlgl immediate mode ----
// Wz geometry is all quads (4 verts + 6 indices each).
// We emit them as RL_QUADS, matching Raylib's own DrawRectangle pattern.
static void render_wz_draw_list(WzDrawList* dl)
{
    if (dl->vtx_count == 0 || dl->draw_call_count == 0) return;

    for (unsigned i = 0; i < dl->draw_call_count; i++) {
        WzDrawCall* dc = &dl->draw_calls[i];

        unsigned int tex_id = dc->texture
            ? (unsigned int)(uintptr_t)dc->texture
            : ctx.white_tex_id;

        if (dc->has_clip) {
#ifdef __EMSCRIPTEN__
            // Scale clip rect from logical to physical pixels.
            BeginScissorMode((int)(dc->clip_rect.x * g_dpr),
                             (int)(dc->clip_rect.y * g_dpr),
                             (int)(dc->clip_rect.w * g_dpr),
                             (int)(dc->clip_rect.h * g_dpr));
#else
            BeginScissorMode((int)dc->clip_rect.x, (int)dc->clip_rect.y,
                             (int)dc->clip_rect.w, (int)dc->clip_rect.h);
#endif
        }

        rlSetTexture(tex_id);
        rlBegin(RL_QUADS);
        rlNormal3f(0.0f, 0.0f, 1.0f);

        // Every 6 indices = 1 quad with 4 unique vertices
        // Index pattern: base+0, base+1, base+2, base+2, base+3, base+0
        // Wz verts: v0=TL, v1=TR, v2=BR, v3=BL
        // RL_QUADS needs: TL, BL, BR, TR (matching DrawRectangle winding)
        for (unsigned j = 0; j + 5 < dc->idx_count; j += 6) {
            unsigned base = dc->idx_offset + j;
            int vi[4] = {
                dl->indices[base + 0],   // v0 = TL
                dl->indices[base + 4],   // v3 = BL
                dl->indices[base + 2],   // v2 = BR
                dl->indices[base + 1],   // v1 = TR
            };
            for (int k = 0; k < 4; k++) {
                WzVertex* v = &dl->vertices[vi[k]];
                rlColor4f(v->r, v->g, v->b, v->a);
                rlTexCoord2f(v->u, v->v);
#ifdef __EMSCRIPTEN__
                // Scale logical → physical pixels and round to the nearest
                // integer device pixel. Rounding here (rather than in logical
                // space) ensures each vertex lands on an exact device pixel
                // boundary regardless of DPR, giving TEXTURE_FILTER_POINT
                // exact 1:1 atlas-to-device sampling with no edge blending.
                rlVertex2f(roundf(v->x * g_dpr), roundf(v->y * g_dpr));
#else
                rlVertex2f(v->x, v->y);
#endif
            }
        }

        rlEnd();
        rlSetTexture(0);

        if (dc->has_clip) EndScissorMode();
    }
}

// ---- Load file into malloc'd buffer ----
static unsigned char* load_file_data(const char* path, size_t* out_size)
{
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* buf = (unsigned char*)malloc(sz);
    if (buf) fread(buf, 1, sz, f);
    fclose(f);
    *out_size = (size_t)sz;
    return buf;
}

// ---- Init GPU resources (white texture only) ----
static void init_gpu_resources(void)
{
    // White 1x1 texture (fallback for solid color draws)
    unsigned char white[] = { 255, 255, 255, 255 };
    Image img = { .data = white, .width = 1, .height = 1,
                  .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, .mipmaps = 1 };
    Texture2D white_tex = LoadTextureFromImage(img);
    ctx.white_tex_id = white_tex.id;
}

// ---- Load fonts and upload atlas ----
static void load_fonts(void)
{
#ifdef __EMSCRIPTEN__
    const char* font_regular = "/fonts/regular.ttf";
    const char* font_bold    = "/fonts/bold.ttf";
#else
    const char* font_regular = "C:\\Windows\\Fonts\\Arial.ttf";
    const char* font_bold    = "C:\\Windows\\Fonts\\ArialBD.ttf";
#endif

    // Fonts are baked at physical pixel sizes (logical × dpr) so each atlas
    // Bake fonts at physical pixel size (base * dpr) so each atlas texel maps
    // 1:1 to a device pixel. Layout metrics are divided by dpr afterwards so
    // the GUI works in logical pixels. screen_scale = 1/dpr converts atlas
    // sizes back to logical when building glyph quads.
#ifdef __EMSCRIPTEN__
    float font_dpr = g_dpr;
#else
    float font_dpr = 1.0f;
#endif

    size_t file_size;
    unsigned char* data = load_file_data(font_regular, &file_size);
    if (data) {
        wz_font_load(0, data, (unsigned)file_size, 18.0f * font_dpr);
        wz_font_load(1, data, (unsigned)file_size, 28.0f * font_dpr);
        free(data);
    }

    data = load_file_data(font_bold, &file_size);
    if (data) {
        wz_font_load(2, data, (unsigned)file_size, 24.0f * font_dpr);
        free(data);
    }

#ifdef __EMSCRIPTEN__
    if (font_dpr != 1.0f) {
        for (unsigned fi = 0; fi < ctx.gui.fonts_count; fi++) {
            WzFont* font = &ctx.gui.fonts[fi];
            font->screen_scale = 1.0f / font_dpr;
            font->pixel_height /= font_dpr;
            font->ascent       /= font_dpr;
            font->descent      /= font_dpr;
            font->line_gap     /= font_dpr;
            for (int ci = font->first_char; ci < font->first_char + font->num_chars; ci++) {
                WzGlyph* g = &font->glyphs[ci];
                g->xoff     /= font_dpr;
                g->yoff     /= font_dpr;
                g->xadvance /= font_dpr;
            }
        }
    }
#endif

    // Pack icons into atlas before upload
    wz_create_dropdown_icon();
    wz_pack_icons_into_atlas();

    // Upload each font atlas to GPU
    for (unsigned fi = 0; fi < ctx.gui.fonts_count; fi++) {
        WzFont* font = &ctx.gui.fonts[fi];
        unsigned int atlas_id = upload_alpha_atlas(font->atlas_bitmap, font->atlas_w, font->atlas_h);
        free(font->atlas_bitmap);
        font->atlas_bitmap = NULL;
        wz_font_set_atlas_texture(fi, (WzTexture){
            .data  = (void*)(uintptr_t)atlas_id,
            .w     = (float)font->atlas_w,
            .h     = (float)font->atlas_h,
        });
    }

    // Dropdown icon bitmap no longer needed (now in atlas)
    free(ctx.gui.dropdown_icon_bitmap);
    ctx.gui.dropdown_icon_bitmap = NULL;

    // X icon: sub-region of font 0 atlas
    WzFont* font0 = &ctx.gui.fonts[0];
    ctx.gui.x_icon = (WzTexture){
        .data  = (void*)(uintptr_t)font0->atlas_texture.data,
        .w     = 16, .h = 16,
        .src_x = ctx.gui.x_icon_atlas_x,
        .src_y = ctx.gui.x_icon_atlas_y,
        .tex_w = (float)font0->atlas_w,
        .tex_h = (float)font0->atlas_h,
    };
}

// ---- Frame ----
static void do_frame(float* avg_total, float* avg_gui, float* avg_layout, float* avg_render,
                     float* smp_total, float* smp_gui, float* smp_layout, float* smp_render,
                     unsigned* idx_total, unsigned* idx_gui, unsigned* idx_layout, unsigned* idx_render)
{
    double frame_start = GetTime();
    float dt = GetFrameTime() * 1000.0f;

#ifdef __EMSCRIPTEN__
    // Per-frame HiDPI resize handling.
    // We do NOT use FLAG_WINDOW_RESIZABLE on web (see InitWindow below).
    // Instead, every frame we check whether the browser window size changed and,
    // if so, resize the canvas ourselves in two steps:
    //   1. emscripten_set_canvas_element_size  — sets the WebGL drawing-buffer
    //      size to physical pixels (css * dpr). This is what the GPU renders into.
    //   2. emscripten_set_element_css_size     — sets the CSS display size back to
    //      logical pixels so the browser doesn't stretch the canvas a second time.
    // Then SetWindowSize tells raylib the new framebuffer dimensions so its
    // projection matrices and GetScreenWidth/Height() stay correct.
    {
        double dpr = get_device_pixel_ratio();
        int css_w = get_browser_width();
        int css_h = get_browser_height();
        int phys_w = (int)(css_w * dpr);
        int phys_h = (int)(css_h * dpr);
        g_dpr   = (float)dpr;
        g_css_w = css_w;
        g_css_h = css_h;
        if (phys_w != GetScreenWidth() || phys_h != GetScreenHeight()) {
            emscripten_set_canvas_element_size("#canvas", phys_w, phys_h);
            SetWindowSize(phys_w, phys_h);
            // CSS = phys/dpr (not raw css_w) to avoid fractional device-pixel
            // mismatch that causes the browser to bilinearly scale the canvas.
            emscripten_set_element_css_size("#canvas", (double)phys_w / dpr, (double)phys_h / dpr);
        }
    }
#endif

    if (_wz_app && _wz_app->update)
        _wz_app->update(_wz_app->userdata, dt);

    // Build input events from Raylib polling
    WzEvent events[MAX_NUM_EVENTS];
    int events_count = build_wz_events(events, MAX_NUM_EVENTS);

    // Widget phase
    wz_set_gui(&ctx.gui);
    Vector2 mp = GetMousePosition();
#ifdef __EMSCRIPTEN__
    // JS tracker returns physical pixel coords; divide by DPR to get logical
    // pixels matching the GUI coordinate space. See install_mouse_tracker().
    mp.x = (float)get_true_mouse_x() / g_dpr;
    mp.y = (float)get_true_mouse_y() / g_dpr;
#endif
    ctx.gui.mouse_pos.x = mp.x;
    ctx.gui.mouse_pos.y = mp.y;

    double t_gui = GetTime();
    static int    s_selected_text = -1;
    static bool   s_active        = false;
    static WzWidget s_dd_widget   = { 0 };

#ifdef __EMSCRIPTEN__
    // Pass logical (CSS) dimensions so the GUI lays out in logical pixels,
    // matching the native window size. The renderer scales to physical pixels.
    wz_begin((unsigned)g_css_w, (unsigned)g_css_h,
             events, (unsigned)events_count, true);
#else
    wz_begin((unsigned)GetScreenWidth(), (unsigned)GetScreenHeight(),
             events, (unsigned)events_count, true);
#endif
    {
        WzWidget root = (WzWidget){ 0 };
        WzWidget vbox = wz_vbox_id(root, WIDGET_VBOX1);

        WzStr strs[] = { wz_str_create("wow1"), wz_str_create("wow2"), wz_str_create("wow3") };
        s_dd_widget = wz_dropdown(vbox, strs, 3, &s_selected_text, &s_active, WIDGET_DROPDOWN1);
        wz_label_id(vbox, wz_str_create("euaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"), WIDGET_LABEL1);
    }
    *avg_gui = perf_avg(smp_gui, idx_gui, (float)((GetTime() - t_gui) * 1000.0));

    double t_layout = GetTime();
    wz_end();
    *avg_layout = perf_avg(smp_layout, idx_layout, (float)((GetTime() - t_layout) * 1000.0));

    // Clipboard copy
    if (ctx.gui.copied_text[0]) {
        SetClipboardText(ctx.gui.copied_text);
        ctx.gui.copied_text[0] = '\0';
    }

    // Clipboard paste (Ctrl+V) — set pasted_text if not already pending
    if (!ctx.gui.pasted_text &&
        (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
        IsKeyPressed(KEY_V))
    {
        const char* clip = GetClipboardText();
        if (clip && clip[0]) {
            size_t len = strlen(clip) + 1;
            char* copy = (char*)malloc(len);
            if (copy) { memcpy(copy, clip, len); ctx.gui.pasted_text = copy; }
        }
    }

    // Performance overlay (top-right corner)
    {
#ifdef __EMSCRIPTEN__
        int sw = g_css_w;  // GUI is in logical pixels on web
#else
        int sw = GetScreenWidth();
#endif
        const char* labels[] = { "total", "gui", "layout", "render", "draws" };
        float values[] = { *avg_total, *avg_gui, *avg_layout, *avg_render,
                           (float)ctx.gui.draw_list.draw_call_count };
        char buf[64];
        for (int i = 0; i < 5; i++) {
            if (i == 4) sprintf(buf, "%s: %d", labels[i], (int)values[i]);
            else        sprintf(buf, "%s: %.2f ms", labels[i], values[i]);
            float tw = 0, th = 0;
            wz_get_text_size(buf, 0, (unsigned)strlen(buf), 0, &tw, &th);
            wz_draw_text(0, (int)(sw - tw - 8), (int)(8 + i * (th + 4)),
                         buf, strlen(buf), WZ_YELLOW);
        }
    }

    BeginDrawing();
    ClearBackground((Color){ 32, 140, 113, 255 });

    // User 3D render into viewport render texture
    if (_wz_app && _wz_app->render && _viewport_rt.id > 0) {
        WzRenderContext rc = {
            .target = _viewport_rt,
            .width  = _viewport_rt_w,
            .height = _viewport_rt_h,
        };
        _wz_app->render(_wz_app->userdata, &rc);
    }

    // GUI composite
    double t_render = GetTime();
    render_wz_draw_list(&ctx.gui.draw_list);
    *avg_render = perf_avg(smp_render, idx_render, (float)((GetTime() - t_render) * 1000.0));

    EndDrawing();

    *avg_total = perf_avg(smp_total, idx_total, (float)((GetTime() - frame_start) * 1000.0));
}

// ---- Perf state (shared between main loop and resize callback) ----
static float    _smp_total[60], _smp_gui[60], _smp_layout[60], _smp_render[60];
static unsigned _idx_total, _idx_gui, _idx_layout, _idx_render;
static float    _avg_total, _avg_gui, _avg_layout, _avg_render;

static void resize_frame_cb(void)
{
    do_frame(&_avg_total, &_avg_gui, &_avg_layout, &_avg_render,
             _smp_total, _smp_gui, _smp_layout, _smp_render,
             &_idx_total, &_idx_gui, &_idx_layout, &_idx_render);
}

// ---- Main ----
int main(void)
{
// FLAG_WINDOW_RESIZABLE is intentionally omitted on web.
// When that flag is set, raylib's internal GLFW resize callback fires on every
// browser resize and calls SetWindowSize with the CSS (logical) pixel dimensions,
// which resets the canvas drawing buffer to logical size and destroys the HiDPI
// setup. We handle resizing ourselves each frame (see do_frame) so we can always
// keep the drawing buffer at physical-pixel size.
#ifdef __EMSCRIPTEN__
    SetConfigFlags(FLAG_VSYNC_HINT);
#else
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
#endif

// HiDPI InitWindow: create the WebGL context at physical-pixel resolution.
// Passing (css * dpr) as the window size makes Emscripten set canvas.width/height
// to physical pixels from the start, so the WebGL framebuffer is full-resolution.
// emscripten_set_element_css_size then fixes the CSS display size back to logical
// pixels — without this the canvas would appear (dpr) times too large on screen.
#ifdef __EMSCRIPTEN__
    {
        // Disable WebGL antialiasing BEFORE context creation. GLFW/raylib
        // creates the context inside InitWindow, so we patch getContext to
        // force antialias:false. Without this, MSAA blurs the framebuffer.
        EM_ASM({
            var origGetContext = HTMLCanvasElement.prototype.getContext;
            HTMLCanvasElement.prototype.getContext = function(type, attrs) {
                if (type === "webgl" || type === "webgl2") {
                    attrs = attrs || {};
                    attrs.antialias = false;
                }
                return origGetContext.call(this, type, attrs);
            };
        });

        double dpr = get_device_pixel_ratio();
        int css_w = get_browser_width();
        int css_h = get_browser_height();
        int phys_w = (int)(css_w * dpr);
        int phys_h = (int)(css_h * dpr);
        g_dpr   = (float)dpr;
        g_css_w = css_w;
        g_css_h = css_h;
        InitWindow(phys_w, phys_h, "WzEngine");
        emscripten_set_element_css_size("#canvas", (double)phys_w / dpr, (double)phys_h / dpr);
    }
#else
    InitWindow(960, 540, "WzEngine");
#endif

    SetWindowMinSize(320, 240);

    // Init WzGui before window is shown
    wz_set_gui(&ctx.gui);
    ctx.gui.ticks_in_ns = wz_ticks_ns;
    ctx.gui.ticks_in_ms = wz_ticks_ms;

    init_gpu_resources();
    load_fonts();

// Install the JS mousemove listener that gives us correct HiDPI mouse coords.
// Must be called after InitWindow so Module.canvas exists.
// See install_mouse_tracker() above for why GetMousePosition() can't be used.
#ifdef __EMSCRIPTEN__
    install_mouse_tracker();
    fix_canvas_display();
#endif

    if (_wz_app && _wz_app->init)
        _wz_app->init(_wz_app->userdata);

    // Subclass window to render during resize (avoids black bars on Windows)
#ifdef _WIN32
    wz_install_resize_hook(GetWindowHandle(), resize_frame_cb);
#endif

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(resize_frame_cb, 0, 1);
#else
    while (!WindowShouldClose()) {
        do_frame(&_avg_total, &_avg_gui, &_avg_layout, &_avg_render,
                 _smp_total, _smp_gui, _smp_layout, _smp_render,
                 &_idx_total, &_idx_gui, &_idx_layout, &_idx_render);
    }

    if (_wz_app && _wz_app->quit)
        _wz_app->quit(_wz_app->userdata);

    CloseWindow();
    return 0;
#endif
}
