#pragma once
#include "imgui.h"      
#ifndef IMGUI_DISABLE

IMGUI_IMPL_API bool     ImGui_ImplOpenGL3_Init(const char* glsl_version = nullptr);
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);

IMGUI_IMPL_API bool     ImGui_ImplOpenGL3_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_DestroyDeviceObjects();

IMGUI_IMPL_API void     ImGui_ImplOpenGL3_UpdateTexture(ImTextureData* tex);

IMGUI_IMPL_API void     ImGui_ImplOpenGL3_SetClipOrigin(unsigned int gl_clip_origin);

#if !defined(IMGUI_IMPL_OPENGL_ES2) \
 && !defined(IMGUI_IMPL_OPENGL_ES3)

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV)) || (defined(__ANDROID__))
#define IMGUI_IMPL_OPENGL_ES3               
#elif defined(__EMSCRIPTEN__) || defined(__amigaos4__)
#define IMGUI_IMPL_OPENGL_ES2               
#else

#endif

#endif

struct ImGui_ImplOpenGL3_RenderState
{
    bool            UseBindSampler;
    bool            UseTexParameterFilter;
    unsigned int    CurrentSampler;                 
    unsigned int    CurrentTexParameterFilter;      
};
IMGUI_IMPL_API ImGui_ImplOpenGL3_RenderState* ImGui_ImplOpenGL3_GetRenderState();

#endif
