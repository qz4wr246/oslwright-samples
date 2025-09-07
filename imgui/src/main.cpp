#include <imgui.h>
#include <implot.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <cstdlib>

void ShowPlotDemo() {
    static float frequency = 1.0f;
    static bool addNoise = false;
    static std::vector<float> x(1000), y(1000);

    ImGui::Begin("Control Panel");
    ImGui::SliderFloat("Frequency", &frequency, 0.1f, 10.0f);
    ImGui::Checkbox("Add Noise", &addNoise);
    ImGui::End();

    for (int i = 0; i < x.size(); ++i) {
        x[i] = i * 0.01f;
        y[i] = std::sin(frequency * x[i]);
        if (addNoise) {
            y[i] += ((std::rand() % 100) / 100.0f - 0.5f) * 0.2f;
        }
    }

    ImGui::Begin("Dynamic Plot");
    if (ImPlot::BeginPlot("Sine Wave")) {
        ImPlot::PlotLine("y = sin(f * x)", x.data(), y.data(), x.size());
        ImPlot::EndPlot();
    }
    ImGui::End();
}

int main() {
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 800, "ImGui + ImPlot Demo", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ShowPlotDemo();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImPlot::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
