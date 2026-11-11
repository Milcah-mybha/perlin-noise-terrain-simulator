#pragma once
#include "imgui.h"      // IMGUI_IMPL_API
#ifndef IMGUI_DISABLE

// Follow "Getting Started" link and check examples/ folder to learn about using backends!
IMGUI_IMPL_API bool     ImGui_ImplOpenGL3_Init(const char* glsl_version = nullptr);
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);

// (Optional) Called by Init/NewFrame/Shutdown
IMGUI_IMPL_API bool     ImGui_ImplOpenGL3_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_DestroyDeviceObjects();

// (Advanced) Use e.g. if you need to precisely control the timing of texture updates (e.g. for staged rendering), by setting ImDrawData::Textures = nullptr to handle this manually.
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_UpdateTexture(ImTextureData* tex);

// (Advanced) Specify clip origin, same value as glClipControl() or 0 for auto-detection.
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_SetClipOrigin(unsigned int gl_clip_origin);

// Configuration flags to add in your imconfig file:
//#define IMGUI_IMPL_OPENGL_ES2     // Enable ES 2 (Auto-detected on Emscripten)
//#define IMGUI_IMPL_OPENGL_ES3     // Enable ES 3 (Auto-detected on iOS/Android)

// You can explicitly select GLES2 or GLES3 API by using one of the '#define IMGUI_IMPL_OPENGL_LOADER_XXX' in imconfig.h or compiler command-line.
#if !defined(IMGUI_IMPL_OPENGL_ES2) \
 && !defined(IMGUI_IMPL_OPENGL_ES3)

// Try to detect GLES on matching platforms
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV)) || (defined(__ANDROID__))
#define IMGUI_IMPL_OPENGL_ES3               // iOS, Android  -> GL ES 3, "#version 300 es"
#elif defined(__EMSCRIPTEN__) || defined(__amigaos4__)
#define IMGUI_IMPL_OPENGL_ES2               // Emscripten    -> GL ES 2, "#version 100"
#else
// Otherwise imgui_impl_opengl3_loader.h will be used.
#endif

#endif

// [BETA] Selected render state data shared with callbacks.
// This is temporarily stored in GetPlatformIO().Renderer_RenderState during the ImGui_ImplOpenGL3_RenderDrawData() call.
// (Please open an issue if you feel you need access to more data)
struct ImGui_ImplOpenGL3_RenderState
{
    bool            UseBindSampler;
    bool            UseTexParameterFilter;
    unsigned int    CurrentSampler;                 // (GLuint) Used if UseBindSampler == true (and are available), otherwise always 0
    unsigned int    CurrentTexParameterFilter;      // (GLuint) Used if UseTexParameterFilter == true
};
IMGUI_IMPL_API ImGui_ImplOpenGL3_RenderState* ImGui_ImplOpenGL3_GetRenderState();

#endif // #ifndef IMGUI_DISABLE
