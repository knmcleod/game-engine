#include "GE/GEpch.h"

#ifdef GE_DEBUG

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <imgui/imgui.h>

#include <imgui/misc/cpp/imgui_stdlib.h>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <imgui/backends/imgui_impl_opengl3.cpp>
#include <imgui/backends/imgui_impl_glfw.cpp>
#include <imgui/imgui_internal.h>
#pragma warning(pop)

#endif // GE_DEBUG
