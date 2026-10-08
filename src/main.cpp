#include "AppState.h"
#include "Apps.h"
#include "Desktop.h"
#include "Taskbar.h"
#include "TaskManager.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>

void reportGlfwError(int code, const char* description)
{
    // Report GLFW initialization, window, and runtime errors to the terminal.
    std::cerr << "GLFW error " << code << ": " << description << '\n';
}

void drawFrame(GLFWwindow* window)
{
    // The window user pointer also gives resize/refresh callbacks this state.
    auto& state = *static_cast<AppState*>(glfwGetWindowUserPointer(window));
    // Do not begin another frame after a close or shared shutdown request.
    if (glfwWindowShouldClose(window) || state.shutdownRequested) {
        return;
    }

    // Framebuffer dimensions are in pixels, including Windows display scaling.
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    if (width <= 0 || height <= 0 || glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
        return;
    }

    // Start one ImGui frame using both backends.
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Call components each rendered frame. Desktop comes first as the base layer;
    // its UI is still a TODO in Desktop.cpp. Task Manager are also stubs.
    // The taskbar reserves its strip of the desktop first so the clock/PWR stay clear of it.
    ApplyTaskbarLayout();
    RenderDesktop(state);
    RenderTaskbar(state);
    // Apps currently draws the demo; the two required app screens belong there.
    RenderApps(state);
    RenderTaskManager(state);

    // Finish the UI, clear the framebuffer, draw ImGui, and present the frame.
    ImGui::Render();

    glViewport(0, 0, width, height);
    // Existing solid background; Desktop.cpp will own the wallpaper UI.
    glClearColor(0.12f, 0.14f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

void onFramebufferResize(GLFWwindow* window, int, int)
{
    // Reuse the full frame to redraw while Windows handles a resize operation.
    drawFrame(window);
}

int main()
{
    // Initialize GLFW and install error reporting before creating the window.
    glfwSetErrorCallback(reportGlfwError);

    if (!glfwInit()) {
        std::cerr << "Could not initialize GLFW.\n";
        return EXIT_FAILURE;
    }

    // Request desktop OpenGL 3.0 for ImGui's OpenGL3 renderer (GLSL 130).
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(1280, 800, "CSOPESY - Desktop OS Mock-up", nullptr, nullptr);
    if (!window) {
        std::cerr << "Could not create the window.\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Activate this window's OpenGL context and enable synchronized buffer swaps.
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Keep shared UI state alive until after the window and callbacks are removed.
    AppState state;
    glfwSetWindowUserPointer(window, &state);

    // Create the ImGui context, enable keyboard navigation, and use the dark style.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    // Install GLFW input callbacks so the demo responds to mouse and keyboard.
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cerr << "Could not initialize the ImGui GLFW backend.\n";
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }
    // Initialize ImGui's OpenGL renderer; release earlier resources if it fails.
    if (!ImGui_ImplOpenGL3_Init("#version 130")) {
        std::cerr << "Could not initialize the ImGui OpenGL3 backend.\n";
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Register redraw callbacks only after both ImGui backends are ready.
    glfwSetFramebufferSizeCallback(window, onFramebufferResize);
    glfwSetWindowRefreshCallback(window, drawFrame);

    // Run until the window closes or a component requests shutdown (future PWR).
    while (!glfwWindowShouldClose(window) && !state.shutdownRequested) {
        // Process input and window events before starting the next frame.
        glfwPollEvents();
        if (glfwWindowShouldClose(window) || state.shutdownRequested) {
            break;
        }
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
            // Keep processing events while minimized without continuously drawing.
            glfwWaitEventsTimeout(0.1);
            continue;
        }
        drawFrame(window);
    }

    // Both the window close button and shared shutdown request use this cleanup.
    // Release ImGui's GPU resources while the OpenGL context still exists.
    glfwSetFramebufferSizeCallback(window, nullptr);
    glfwSetWindowRefreshCallback(window, nullptr);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
