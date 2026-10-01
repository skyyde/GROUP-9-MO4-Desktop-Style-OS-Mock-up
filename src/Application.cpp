#include "Application.h"
#include "AppState.h"
#include "Apps.h"
#include "Desktop.h"
#include "Taskbar.h"
#include "TaskManager.h"
#include "Theme.h"

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cstdio>
#include <cstring>

namespace
{
    void OnGlfwError(int code, const char* description)
    {
        std::fprintf(stderr, "GLFW error %d: %s\n", code, description);
    }
}

int RunApplication(int argc, char* argv[])
{
    // Optional development check: render 120 starter frames, then shut down.
    bool smokeTest = false;
    if (argc == 2 && std::strcmp(argv[1], "--smoke-test") == 0) smokeTest = true;
    else if (argc != 1)
    {
        std::fprintf(stderr, "Usage: mo4 [--smoke-test]\n");
        return 1;
    }

    glfwSetErrorCallback(OnGlfwError);
    if (!glfwInit()) return 1;
#ifdef __APPLE__
    const char* glslVersion = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 800, "MO4 - CSOPESY Desktop OS", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }
    glfwSetWindowSizeLimits(window, static_cast<int>(Theme::MinimumWidth),
        static_cast<int>(Theme::MinimumHeight), GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync: rendering is paced by display refresh.

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // Predictable initial layout on each launch.
    Theme::Apply();
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    if (!ImGui_ImplOpenGL3_Init(glslVersion))
    {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    AppState state;
    int renderedFrames = 0;

    while (!glfwWindowShouldClose(window))
    {
        // 1. Receive keyboard/mouse/window events without a separate polling thread.
        glfwPollEvents();
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        if (width == 0 || height == 0)
        {
            glfwWaitEventsTimeout(0.05); // Avoid a busy loop while minimized.
            continue;
        }

        // 2. Begin a new ImGui frame.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 3. Call each module. These are scaffolds for the later deadlines.
        Desktop::Render(state);
        Apps::Render(state);
        TaskManager::Render(state);
        Taskbar::Render(state);

        // Temporary setup panel: proves that Dear ImGui renders successfully.
        // Remove this panel when the desktop module is implemented.
        ImGui::SetNextWindowPos(ImVec2(40.0f, 40.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(510.0f, 185.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("MO4 - October 1 Starter"))
        {
            ImGui::TextColored(Theme::Accent, "CSOPESY / Group 9");
            ImGui::Separator();
            ImGui::TextUnformatted("Dear ImGui + GLFW + OpenGL are running.");
            ImGui::TextWrapped("The basic render loop is ready. Module functions are connected as placeholders.");
            ImGui::TextUnformatted("Close the application using the window X.");
        }
        ImGui::End();

        // 4. Draw the frame using OpenGL and present it.
        ImGui::Render();
        glViewport(0, 0, width, height);
        glClearColor(0.02f, 0.03f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        ++renderedFrames;
        if (smokeTest && renderedFrames >= 120) state.requestShutdown = true;
        if (state.requestShutdown) glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    // 5. Release resources. Future PWR uses the same shutdown/cleanup path.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    if (smokeTest) std::printf("Smoke check: rendered %d frames and shut down cleanly.\n", renderedFrames);
    return 0;
}
