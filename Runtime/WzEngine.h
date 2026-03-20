#pragma once
#include <SDL3/SDL.h>
#include "WzGuiCore.h"

// ---- Render context passed to app->render() each frame ----
// The user draws their 3D scene into `target` using their own pipelines.
// `cmd` is the engine's command buffer for this frame — begin a render pass
// on `target`, issue draw calls, end the pass. Do not submit or present.
typedef struct WzRenderContext {
    SDL_GPUDevice*        device;
    SDL_GPUCommandBuffer* cmd;
    SDL_GPUTexture*       target;   // offscreen render target, NOT the swapchain
    int                   width;
    int                   height;
} WzRenderContext;

// ---- App callbacks — user fills this in and passes to wz_run() ----
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
// its texture and displays it here. Returns the layout rect so callers
// can position it — size is whatever the layout gives it.
//
// Usage:
//   WzRect vp = wz_viewport(parent, WIDGET_VIEWPORT);
//
// The engine automatically calls app->render() with a texture sized to
// match this rect each frame.
WzRect wz_viewport(WzWidget parent, WzID id);

// ---- Engine entry point ----
// Call this from SDL_AppInit. The engine takes ownership of the frame loop.
void wz_engine_set_app(WzApp* app);

// Internal — called by the engine each frame to resize/return viewport texture.
// Not for user code.
SDL_GPUTexture* wz_engine_get_viewport_texture(SDL_GPUDevice* device, int w, int h);
