#pragma once
#include "raylib.h"
#include "WzGuiCore.h"

// ---- Render context passed to app->render() each frame ----
// The user draws their 3D scene into `target` using BeginTextureMode/EndTextureMode.
// Do NOT call EndDrawing — the engine owns the frame boundary.
typedef struct WzRenderContext {
    RenderTexture2D target;   // offscreen render texture (Raylib)
    int             width;
    int             height;
} WzRenderContext;

// ---- App callbacks — user fills this in and passes to wz_engine_set_app() ----
typedef struct WzApp {
    // Called once after window + GPU are ready. Load assets here.
    void (*init)  (void* userdata);

    // Called every frame before rendering. Run game logic here.
    void (*update)(void* userdata, float dt);

    // Called every frame. Draw your 3D scene into rc->target.
    // The engine composites rc->target into the viewport widget afterward.
    void (*render)(void* userdata, WzRenderContext* rc);

    // Called on shutdown. Free resources here.
    void (*quit)  (void* userdata);

    void* userdata;
} WzApp;

// ---- Viewport widget ----
// Place this in your GUI layout. The engine renders app->render() into
// its texture and displays it here. w/h are the pixel dimensions of the texture.
//
// Usage:
//   wz_viewport(vbox, 800, 600, WIDGET_VIEWPORT);
WzWidget wz_viewport(WzWidget parent, int w, int h, unsigned id);

// ---- Engine entry point ----
// Call this before the main loop. The engine stores the app pointer and
// calls its callbacks each frame.
void wz_engine_set_app(WzApp* app);
