#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <iomanip>
#include <random>
#include <string>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Shader.h"
#include "Camera.h"
#include "PerlinNoise.h"
#include "Terrain.h"
#include "Water.h"

// Kích thước cửa sổ
const unsigned int SCR_WIDTH = 1366;
const unsigned int SCR_HEIGHT = 768;

// Vị trí mặc định Camera
const glm::vec3 DEFAULT_CAM_POS(0.0f, 45.0f, 125.0f);
Camera camera(DEFAULT_CAM_POS);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool mouseCaptured = false; // Bật/tắt khóa chuột xoay góc nhìn 360 độ

// Quản lý thời gian
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Trạng thái ứng dụng
bool wireframeMode = false;
bool fogEnabled = true;
int colorMode = 0; // 0: Biomes, 1: Heatmap, 2: Grayscale
bool renderWater = true;
bool rotateSun = true;
float sunAngle = 55.0f;
float sunSpeed = 8.0f;

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);
void setupImGuiTheme();

int main() {
    // 1. Khởi tạo GLFW
    if (!glfwInit()) {
        std::cerr << "Lỗi: Không khởi tạo được GLFW!" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. Tạo cửa sổ GLFW
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Perlin Noise Terrain Lab 3D (C++/OpenGL)", NULL, NULL);
    if (!window) {
        std::cerr << "Lỗi: Không tạo được cửa sổ GLFW!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // 3. Nạp GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Lỗi: Không nạp được GLAD!" << std::endl;
        return -1;
    }

    // 4. Cấu hình OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 5. Khởi tạo Dear ImGui & Phông chữ Tiếng Việt Unicode
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    setupImGuiTheme();

    ImFontConfig font_cfg;
    font_cfg.OversampleH = 2;
    font_cfg.OversampleV = 2;
    font_cfg.PixelSnapH = true;
    const ImWchar* viRanges = io.Fonts->GetGlyphRangesVietnamese();

    ImFont* viFont = nullptr;
    viFont = io.Fonts->AddFontFromFileTTF("assets/fonts/OpenSans-Regular.ttf", 17.0f, &font_cfg, viRanges);

    if (!viFont) viFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 17.0f, &font_cfg, viRanges);
    if (!viFont) viFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 17.0f, &font_cfg, viRanges);
    if (!viFont) viFont = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/liberation-sans-fonts/LiberationSans-Regular.ttf", 17.0f, &font_cfg, viRanges);

    if (!viFont) {
        io.Fonts->AddFontDefault();
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 6. Shader & Đối tượng
    Shader terrainShader("shaders/terrain.vert", "shaders/terrain.frag");
    Shader waterShader("shaders/water.vert", "shaders/water.frag");

    PerlinNoise perlin(2026);
    Terrain terrain(220, 220, 1.0f);
    terrain.generate(perlin);

    Water water(240.0f, 6.0f);

    glm::vec3 skyColor(0.66f, 0.78f, 0.90f);

    // 7. Vòng lặp render chính
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Giới hạn deltaTime tránh hiện tượng nhảy khung hình đột ngột khi kéo cửa sổ
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        // Xử lý phím bấm & Cập nhật Camera mượt mà với quán tính lướt
        processInput(window);
        camera.Update(deltaTime);

        // Khởi động Frame ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Chu kỳ quay mặt trời
        if (rotateSun) {
            sunAngle += deltaTime * sunSpeed;
            if (sunAngle > 360.0f) sunAngle -= 360.0f;
        }
        float sunRad = glm::radians(sunAngle);
        glm::vec3 lightDir = glm::normalize(glm::vec3(cos(sunRad), -0.72f, sin(sunRad)));

        // Xóa màn hình
        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Ma trận Projection & View
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 600.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        // ================= VẼ ĐỊA HÌNH =================
        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        terrainShader.use();
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view", view);
        terrainShader.setMat4("model", model);

        terrainShader.setVec3("uViewPos", camera.Position);
        terrainShader.setVec3("uLightDir", lightDir);
        terrainShader.setVec3("uLightColor", glm::vec3(1.0f, 0.98f, 0.92f));
        terrainShader.setVec3("uAmbientColor", glm::vec3(0.24f, 0.27f, 0.32f));

        terrainShader.setFloat("uMinHeight", terrain.getMinHeight());
        terrainShader.setFloat("uMaxHeight", terrain.getMaxHeight());
        terrainShader.setInt("uColorMode", colorMode);

        terrainShader.setBool("uFogEnabled", fogEnabled);
        terrainShader.setVec3("uFogColor", skyColor);
        terrainShader.setFloat("uFogStart", 70.0f);
        terrainShader.setFloat("uFogEnd", 280.0f);

        terrain.draw(terrainShader);

        // ================= VẼ MẶT NƯỚC =================
        if (renderWater && !wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            waterShader.use();
            waterShader.setMat4("projection", projection);
            waterShader.setMat4("view", view);
            waterShader.setMat4("model", model);

            waterShader.setVec3("uViewPos", camera.Position);
            waterShader.setVec3("uLightDir", lightDir);
            waterShader.setVec3("uLightColor", glm::vec3(1.0f, 0.98f, 0.90f));

            waterShader.setBool("uFogEnabled", fogEnabled);
            waterShader.setVec3("uFogColor", skyColor);
            waterShader.setFloat("uFogStart", 70.0f);
            waterShader.setFloat("uFogEnd", 280.0f);

            water.draw(waterShader, currentFrame);
        }

        // ================= BẢNG ĐIỀU KHIỂN TIẾNG VIỆT =================
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // 1. Thẻ thông số nổi (Badges)
        ImGui::SetNextWindowPos(ImVec2(18, 18), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.88f);
        ImGui::Begin("Thẻ Thống Số##Badges", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
        {
            ImGui::TextColored(ImVec4(0.45f, 0.70f, 1.0f, 1.0f), "Phòng Thí Nghiệm Địa Hình Perlin Noise 3D");
            ImGui::Separator();
            
            float landPct = terrain.getLandPercentage(renderWater ? water.waterHeight : -999.0f);

            ImGui::Text("Số tầng (Octaves): "); ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "%d", terrain.octaves);
            ImGui::SameLine(180);
            ImGui::Text("Khoảng cao: "); ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "%.1f .. %.1f m", terrain.getMinHeight(), terrain.getMaxHeight());
            ImGui::SameLine(360);
            ImGui::Text("Đất liền: "); ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 1.0f, 1.0f), "%.0f %%", landPct);
        }
        ImGui::End();

        // 2. Bảng điều khiển chính (Control Panel)
        ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH - 440, 18), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(420, SCR_HEIGHT - 36), ImGuiCond_FirstUseEver);

        ImGui::Begin("BẢNG ĐIỀU KHIỂN ĐỊA HÌNH", nullptr, ImGuiWindowFlags_None);

        // Nút chuyển nhanh chế độ Chuột
        if (mouseCaptured) {
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), ">>> ĐANG XOAY CAMERA (Nhấn Chuột Phải để mở chuột) <<<");
        } else {
            if (ImGui::Button("BẬT XOAY CAMERA 360 ĐỘ (Chuột Phải)", ImVec2(-1, 32))) {
                mouseCaptured = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true;
            }
        }
        ImGui::Separator();

        // --- MỤC 1: CÁC DẠNG ĐỊA HÌNH MẪU (PRESETS) ---
        ImGui::TextColored(ImVec4(0.45f, 0.65f, 1.0f, 1.0f), "CÁC DẠNG ĐỊA HÌNH MẪU (PRESETS):");
        
        float btnWidth = 192.0f;
        float btnHeight = 32.0f;

        if (ImGui::Button("Đồi núi nhấp nhô", ImVec2(btnWidth, btnHeight))) {
            terrain.applyPreset(PRESET_ROLLING_HILLS, perlin);
        }
        ImGui::SameLine();
        if (ImGui::Button("Dãy núi nhọn (Alpine)", ImVec2(btnWidth, btnHeight))) {
            terrain.applyPreset(PRESET_ALPINE_RIDGES, perlin);
        }

        if (ImGui::Button("Quần đảo đại dương", ImVec2(btnWidth, btnHeight))) {
            terrain.applyPreset(PRESET_ISLANDS, perlin);
        }
        ImGui::SameLine();
        if (ImGui::Button("Đơn tầng (không gồ ghề)", ImVec2(btnWidth, btnHeight))) {
            terrain.applyPreset(PRESET_SINGLE_OCTAVE, perlin);
        }

        if (ImGui::Button("Vách đá gồ ghề", ImVec2(btnWidth, btnHeight))) {
            terrain.applyPreset(PRESET_ROUGH_CRAGS, perlin);
        }
        ImGui::SameLine();
        if (ImGui::Button("Hẻm núi bậc thang", ImVec2(btnWidth, btnHeight))) {
            terrain.applyPreset(PRESET_TERRACED, perlin);
        }

        ImGui::Spacing();
        ImGui::Separator();

        // --- MỤC 2: ĐIỀU KHIỂN HOẠT HỌA (PLAYBACK) ---
        ImGui::TextColored(ImVec4(0.45f, 0.65f, 1.0f, 1.0f), "ĐIỀU KHIỂN HOẠT HỌA (PLAYBACK):");
        
        if (ImGui::Button(rotateSun ? "Tạm dừng xoay mặt trời" : "Tiếp tục xoay mặt trời", ImVec2(192, 28))) {
            rotateSun = !rotateSun;
        }
        ImGui::SameLine();
        if (ImGui::Button("Đổi hạt giống (Seed)", ImVec2(192, 28))) {
            std::random_device rd;
            perlin.setSeed(rd());
            terrain.generate(perlin);
        }

        if (ImGui::Button("Đặt lại góc nhìn Camera (Reset view)", ImVec2(-1, 28))) {
            camera.Reset(DEFAULT_CAM_POS);
        }

        ImGui::Spacing();
        ImGui::Separator();

        // --- MỤC 3: CHẾ ĐỘ HIỂN THỊ (VIEW) ---
        ImGui::TextColored(ImVec4(0.45f, 0.65f, 1.0f, 1.0f), "CHẾ ĐỘ HIỂN THỊ (VIEW):");
        const char* viewOptions[] = {
            "Địa hình tự nhiên (Biomes thực vật/đá/tuyết)",
            "Bản đồ nhiệt độ cao (Elevation Heatmap)",
            "Thang xám viễn thám (Grayscale Heightmap)"
        };
        ImGui::Combo("Bảng màu", &colorMode, viewOptions, IM_ARRAYSIZE(viewOptions));

        ImGui::Checkbox("Chế độ Khung dây (Wireframe Mesh)", &wireframeMode);
        ImGui::SameLine();
        ImGui::Checkbox("Sương mù (Fog)", &fogEnabled);

        ImGui::Checkbox("Hiển thị mặt nước biển", &renderWater);
        if (renderWater) {
            ImGui::SliderFloat("Mực nước biển", &water.waterHeight, 0.0f, 25.0f, "%.1f m");
        }

        ImGui::Spacing();
        ImGui::Separator();

        // --- MỤC 4: THIẾT LẬP THÔNG SỐ NHIỄU (NOISE SETTINGS) ---
        ImGui::TextColored(ImVec4(0.45f, 0.65f, 1.0f, 1.0f), "THIẾT LẬP THÔNG SỐ NHIỄU (NOISE SETTINGS):");
        
        bool needRegen = false;

        needRegen |= ImGui::SliderFloat("Tần số lưới (Frequency)", &terrain.noiseScale, 0.005f, 0.040f, "%.4f");
        needRegen |= ImGui::SliderInt("Số tầng chi tiết (Octaves)", &terrain.octaves, 1, 8);
        needRegen |= ImGui::SliderFloat("Hệ số tần số (Lacunarity)", &terrain.lacunarity, 1.5f, 3.5f, "%.2f");
        needRegen |= ImGui::SliderFloat("Hệ số biên độ (Persistence)", &terrain.persistence, 0.2f, 0.8f, "%.2f");
        needRegen |= ImGui::SliderFloat("Độ cao núi (Height Scale)", &terrain.heightScale, 5.0f, 65.0f, "%.1f");
        needRegen |= ImGui::SliderFloat("Độ dốc lũy thừa (Redistribution)", &terrain.redistribution, 0.8f, 2.5f, "%.2f");

        if (needRegen) {
            terrain.generate(perlin);
        }

        ImGui::Spacing();
        ImGui::Separator();

        // --- HƯỚNG DẪN THAO TÁC NHANH ---
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.6f, 1.0f), "Hướng dẫn điều khiển mượt mà:");
        ImGui::BulletText("Nhấn Chuột Phải: Bật / Tắt xoay camera 360 độ");
        ImGui::BulletText("W, A, S, D: Lướt tới / lùi / trái / phải (có quán tính)");
        ImGui::BulletText("Space: Bay lên | Ctrl hoặc C: Hạ xuống");
        ImGui::BulletText("Giữ Shift: Tăng tốc bay nhanh (Sprint Turbo x2.3)");
        ImGui::BulletText("Cuộn chuột: Thu phóng mượt tầm nhìn (FoV Zoom)");

        ImGui::End();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (mouseCaptured) {
            mouseCaptured = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetWindowShouldClose(window, true);
        }
    }

    // Tăng tốc bay nhanh khi giữ Shift
    camera.SetSprint(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);

    // Di chuyển Camera (W, A, S, D, Space, Ctrl/C)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN);

    // Phím M: Bật/tắt nhanh chế độ xoay camera
    static bool mPressed = false;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !mPressed) {
        mouseCaptured = !mouseCaptured;
        glfwSetInputMode(window, GLFW_CURSOR, mouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        firstMouse = true;
        mPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) mPressed = false;

    // Phím TAB: Đổi nhanh Wireframe
    static bool tabPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabPressed) {
        wireframeMode = !wireframeMode;
        tabPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) tabPressed = false;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        // Nhấp chuột phải -> Bật/Tắt khóa chuột xoay camera
        mouseCaptured = !mouseCaptured;
        glfwSetInputMode(window, GLFW_CURSOR, mouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        firstMouse = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Nếu nhấp chuột trái vào vùng 3D (không bấm trúng cửa sổ ImGui) -> Bật xoay camera
        if (!io.WantCaptureMouse && !mouseCaptured) {
            mouseCaptured = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!mouseCaptured) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void setupImGuiTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 5.0f;
    style.GrabRounding = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.Alpha = 0.94f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]           = ImVec4(0.09f, 0.11f, 0.16f, 0.94f);
    colors[ImGuiCol_Header]             = ImVec4(0.18f, 0.23f, 0.35f, 0.90f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.24f, 0.32f, 0.48f, 1.00f);
    colors[ImGuiCol_Button]             = ImVec4(0.16f, 0.22f, 0.34f, 0.90f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.28f, 0.38f, 0.58f, 1.00f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.35f, 0.48f, 0.72f, 1.00f);
    colors[ImGuiCol_FrameBg]            = ImVec4(0.13f, 0.16f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.20f, 0.25f, 0.36f, 1.00f);
    colors[ImGuiCol_TitleBg]            = ImVec4(0.10f, 0.13f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.15f, 0.22f, 0.35f, 1.00f);
    colors[ImGuiCol_SliderGrab]         = ImVec4(0.45f, 0.55f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]   = ImVec4(0.60f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_CheckMark]          = ImVec4(0.45f, 0.75f, 1.00f, 1.00f);
}
