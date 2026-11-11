#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
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

int windowWidth = 1366;
int windowHeight = 768;

const glm::vec3 DEFAULT_CAM_POS(0.0f, 85.0f, 135.0f);
const float DEFAULT_YAW   = -90.0f;
const float DEFAULT_PITCH = -30.0f;

Camera camera(DEFAULT_CAM_POS, glm::vec3(0.0f, 1.0f, 0.0f), DEFAULT_YAW, DEFAULT_PITCH);

float lastX = 1366 / 2.0f;
float lastY = 768 / 2.0f;
bool firstMouse = true;
bool isDraggingCamera = false;
bool fpsMode = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool wireframeMode = false;
int colorMode = 0;
bool renderWater = true;
bool rotateSun = true;
float sunAngle = 45.0f;
float sunSpeed = 6.0f;
bool showControlPanel = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);
void setupImGuiTheme();

// Ham main: khoi tao GLFW, OpenGL va chay chuong trinh
int main() {
    // Khoi tao GLFW va tao cua so hien thi
    if (!glfwInit()) {
        std::cerr << "Lỗi: Không khởi tạo được GLFW!" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Mô Phỏng Sinh Địa Hình 3D - Perlin Noise", NULL, NULL);
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

    // Nap cac con tro ham OpenGL qua thu vien GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Lỗi: Không nạp được GLAD!" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Cau hinh Dear ImGui va phông chu tieng Viet
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

    // Khoi tao Shader, dia hinh 3D va mat nuoc bien
    Shader terrainShader("shaders/terrain.vert", "shaders/terrain.frag");
    Shader waterShader("shaders/water.vert", "shaders/water.frag");

    PerlinNoise perlin(2026);
    Terrain terrain(180, 120.0f);
    terrain.applyPreset(PRESET_ISLANDS, perlin);

    Water water(420.0f, terrain.waterLevel * terrain.heightScale);
    glm::vec3 skyColor(0.60f, 0.73f, 0.88f);

    // Vong lap render chinh
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (deltaTime > 0.1f) deltaTime = 0.1f;

        processInput(window);
        camera.Update(deltaTime);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (rotateSun) {
            sunAngle += deltaTime * sunSpeed;
            if (sunAngle > 360.0f) sunAngle -= 360.0f;
        }
        float sunRad = glm::radians(sunAngle);
        glm::vec3 lightDir = glm::normalize(glm::vec3(cos(sunRad), -0.85f, sin(sunRad)));

        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (windowHeight > 0) ? static_cast<float>(windowWidth) / static_cast<float>(windowHeight) : (16.0f / 9.0f);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 1200.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // Ve dia hinh 3D
        terrainShader.use();
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view", view);
        terrainShader.setMat4("model", model);

        terrainShader.setVec3("uViewPos", camera.Position);
        terrainShader.setVec3("uLightDir", lightDir);
        terrainShader.setVec3("uLightColor", glm::vec3(1.0f, 0.98f, 0.92f));
        terrainShader.setVec3("uAmbientColor", glm::vec3(0.40f, 0.46f, 0.54f));

        terrainShader.setFloat("uWaterLevel", terrain.waterLevel);
        terrainShader.setInt("uColorMode", colorMode);

        terrain.draw(terrainShader);

        // Ve mat nuoc bien dai duong
        if (renderWater && !wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            waterShader.use();
            waterShader.setMat4("projection", projection);
            waterShader.setMat4("view", view);
            waterShader.setMat4("model", model);

            waterShader.setVec3("uViewPos", camera.Position);
            waterShader.setVec3("uLightDir", lightDir);
            waterShader.setVec3("uLightColor", glm::vec3(1.0f, 0.98f, 0.92f));

            water.waterHeight = terrain.waterLevel * terrain.heightScale;
            water.draw(waterShader, currentFrame);
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // Bang thong so dia hinh goc trai man hinh
        ImGui::SetNextWindowPos(ImVec2(16, 16), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.85f);
        ImGui::Begin("Thống Số##StatsCard", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
        {
            ImGui::TextColored(ImVec4(0.35f, 0.80f, 1.0f, 1.0f), "MÔ PHỎNG ĐỊA HÌNH 3D (PERLIN NOISE)");
            ImGui::Separator();
            
            float landPct = terrain.getLandPercentage();

            ImGui::Text("Số lớp chi tiết (Octaves):"); ImGui::SameLine(220);
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "%d tầng", terrain.octaves);

            ImGui::Text("Độ cao núi tối đa:");         ImGui::SameLine(220);
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "%.1f m", terrain.heightScale);

            ImGui::Text("Mực nước đại dương:");        ImGui::SameLine(220);
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 1.0f, 1.0f), "%.0f %%", terrain.waterLevel * 100.0f);

            ImGui::Text("Tỷ lệ đất liền:");            ImGui::SameLine(220);
            ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.7f, 1.0f), "%.0f %%", landPct);

            ImGui::Text("Tốc độ khung hình (FPS):");   ImGui::SameLine(220);
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%.0f FPS", io.Framerate);
        }
        ImGui::End();

        float topBarWidth = showControlPanel ? 360.0f : 240.0f;
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - topBarWidth - 16, 16), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.85f);
        ImGui::Begin("Tác Vụ Nhanh##TopBar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
        {
            if (ImGui::Button(showControlPanel ? "Thu gọn bảng [H]" : "⚙ Mở bảng điều khiển [H]")) {
                showControlPanel = !showControlPanel;
            }
            ImGui::SameLine();
            if (ImGui::Button("Góc nhìn chuẩn [R]")) {
                camera.Reset(DEFAULT_CAM_POS, DEFAULT_YAW, DEFAULT_PITCH);
            }
        }
        ImGui::End();

        // Bang dieu khien thong so chinh ben phai man hinh
        if (showControlPanel) {
            float panelWidth = 410.0f;
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - panelWidth - 16, 62), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(panelWidth, io.DisplaySize.y - 78), ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.88f);

            ImGui::Begin("BẢNG ĐIỀU KHIỂN ĐỊA HÌNH", nullptr, ImGuiWindowFlags_None);

            if (fpsMode) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), ">>> ĐANG KHÓA CHUỘT FPS (Nhấn F hoặc ESC để mở chuột) <<<");
            } else {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.6f, 1.0f), "Kéo giữ Chuột để xoay camera | Buông để bấm nút");
            }
            ImGui::Separator();

            ImGui::TextColored(ImVec4(0.45f, 0.75f, 1.0f, 1.0f), "1. CÁC DẠNG ĐỊA HÌNH MẪU (PRESETS):");
            
            float btnW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
            float btnH = 30.0f;

            if (ImGui::Button("Quần đảo tự nhiên", ImVec2(btnW, btnH))) {
                terrain.applyPreset(PRESET_ISLANDS, perlin);
            }
            ImGui::SameLine();
            if (ImGui::Button("Đồi núi thoai thoải", ImVec2(btnW, btnH))) {
                terrain.applyPreset(PRESET_ROLLING_HILLS, perlin);
            }

            if (ImGui::Button("Dãy núi tuyết hùng vĩ", ImVec2(btnW, btnH))) {
                terrain.applyPreset(PRESET_ALPINE_RIDGES, perlin);
            }
            ImGui::SameLine();
            if (ImGui::Button("Hẻm núi bậc thang", ImVec2(btnW, btnH))) {
                terrain.applyPreset(PRESET_TERRACED, perlin);
            }

            if (ImGui::Button("Miệng núi lửa sâu", ImVec2(btnW, btnH))) {
                terrain.applyPreset(PRESET_VOLCANO, perlin);
            }
            ImGui::SameLine();
            if (ImGui::Button("Sóng đơn giản", ImVec2(btnW, btnH))) {
                terrain.applyPreset(PRESET_SINGLE_OCTAVE, perlin);
            }

            ImGui::Spacing();
            ImGui::Separator();

            ImGui::TextColored(ImVec4(0.45f, 0.75f, 1.0f, 1.0f), "2. TÙY CHỈNH THÔNG SỐ ĐỊA HÌNH:");
            
            bool needRegen = false;

            ImGui::TextUnformatted("Tần số sóng (Frequency)");
            ImGui::SameLine(ImGui::GetWindowWidth() - 85.0f);
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), "%.1f", terrain.freq);
            ImGui::SetNextItemWidth(-1.0f);
            needRegen |= ImGui::SliderFloat("##Frequency", &terrain.freq, 1.0f, 6.0f, "");

            ImGui::TextUnformatted("Số tầng chi tiết (Octaves)");
            ImGui::SameLine(ImGui::GetWindowWidth() - 85.0f);
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), "%d tầng", terrain.octaves);
            ImGui::SetNextItemWidth(-1.0f);
            needRegen |= ImGui::SliderInt("##Octaves", &terrain.octaves, 1, 9, "");

            ImGui::TextUnformatted("Độ biến thiên (Lacunarity)");
            ImGui::SameLine(ImGui::GetWindowWidth() - 85.0f);
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), "%.2f", terrain.lacunarity);
            ImGui::SetNextItemWidth(-1.0f);
            needRegen |= ImGui::SliderFloat("##Lacunarity", &terrain.lacunarity, 1.4f, 3.0f, "");

            ImGui::TextUnformatted("Độ sắc nét (Persistence)");
            ImGui::SameLine(ImGui::GetWindowWidth() - 85.0f);
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), "%.2f", terrain.persistence);
            ImGui::SetNextItemWidth(-1.0f);
            needRegen |= ImGui::SliderFloat("##Persistence", &terrain.persistence, 0.2f, 0.85f, "");

            ImGui::TextUnformatted("Chiều cao núi (Height Scale)");
            ImGui::SameLine(ImGui::GetWindowWidth() - 85.0f);
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), "%.1f m", terrain.heightScale);
            ImGui::SetNextItemWidth(-1.0f);
            needRegen |= ImGui::SliderFloat("##HeightScale", &terrain.heightScale, 10.0f, 60.0f, "");

            ImGui::TextUnformatted("Mực nước biển (Sea Level)");
            ImGui::SameLine(ImGui::GetWindowWidth() - 85.0f);
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), "%.0f %%", terrain.waterLevel * 100.0f);
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::SliderFloat("##WaterLevel", &terrain.waterLevel, 0.05f, 0.85f, "");

            if (needRegen) {
                terrain.generate(perlin);
            }

            if (ImGui::Button("Sinh hạt giống mới (New Seed)", ImVec2(-1, 28))) {
                std::random_device rd;
                perlin.setSeed(rd());
                terrain.generate(perlin);
            }

            ImGui::Spacing();
            ImGui::Separator();

            ImGui::TextColored(ImVec4(0.45f, 0.75f, 1.0f, 1.0f), "3. CHẾ ĐỘ HIỂN THỊ & MÔI TRƯỜNG:");
            const char* viewOptions[] = {
                "Địa hình tự nhiên (Biomes sinh thái)",
                "Bản đồ nhiệt độ cao (Elevation Heatmap)",
                "Thang xám viễn thám (Grayscale Heightmap)"
            };

            ImGui::Text("Bảng màu hiển thị:");
            ImGui::PushItemWidth(-1);
            ImGui::Combo("##ColorMode", &colorMode, viewOptions, IM_ARRAYSIZE(viewOptions));
            ImGui::PopItemWidth();

            ImGui::Checkbox("Lưới khung dây 3D (Wireframe)", &wireframeMode);
            ImGui::SameLine(200);
            ImGui::Checkbox("Mặt nước đại dương", &renderWater);

            if (ImGui::Button(rotateSun ? "Tạm dừng quay mặt trời" : "Tiếp tục quay mặt trời", ImVec2(-1, 28))) {
                rotateSun = !rotateSun;
            }

            ImGui::Spacing();
            ImGui::Separator();

            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.6f, 1.0f), "Thao tác chuột & bàn phím:");
            ImGui::BulletText("Nhấn giữ Chuột (Trái/Phải): Rê để xoay 360°");
            ImGui::BulletText("Buông chuột: Chuột tự do bấm nút / thanh trượt");
            ImGui::BulletText("W, A, S, D: Lướt tới / lùi / trái / phải");
            ImGui::BulletText("Space / Ctrl: Bay lên cao / Hạ xuống thấp");
            ImGui::BulletText("Giữ Shift: Tăng tốc bay nhanh (Turbo x2.3)");
            ImGui::BulletText("Cuộn chuột: Thu phóng mượt tầm nhìn (FoV Zoom)");
            ImGui::BulletText("Phím F: Khóa/Mở chuột liên tục kiểu Game FPS");
            ImGui::BulletText("Phím H: Ẩn/Hiện nhanh bảng điều khiển");
            ImGui::BulletText("Phím R: Đặt lại góc nhìn toàn cảnh");

            ImGui::End();
        }

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

// Xu ly su kien ban phim di chuyen camera va cac phim tat
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (fpsMode) {
            fpsMode = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetWindowShouldClose(window, true);
        }
    }

    camera.SetSprint(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.ProcessKeyboard(UP);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) camera.ProcessKeyboard(DOWN);

    static bool fPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fPressed) {
        fpsMode = !fpsMode;
        glfwSetInputMode(window, GLFW_CURSOR, fpsMode ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        firstMouse = true;
        fPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) fPressed = false;

    static bool hPressed = false;
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hPressed) {
        showControlPanel = !showControlPanel;
        hPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE) hPressed = false;

    static bool rPressed = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rPressed) {
        camera.Reset(DEFAULT_CAM_POS, DEFAULT_YAW, DEFAULT_PITCH);
        rPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) rPressed = false;

    static bool tabPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabPressed) {
        wireframeMode = !wireframeMode;
        tabPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) tabPressed = false;
}

// Xu ly su kien nhan giu chuot trai/phai de xoay goc nhin
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();

    if (io.WantCaptureMouse) {
        isDraggingCamera = false;
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            isDraggingCamera = true;
            firstMouse = true;
        } else if (action == GLFW_RELEASE) {
            isDraggingCamera = false;
        }
    }
}

// Xu ly su kien di chuyen con tro chuot
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
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

    if (isDraggingCamera || fpsMode) {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

// Xu ly su kien cuon chuot de thu phong goc nhin (FoV Zoom)
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

// Xu ly khi nguoi dung thay doi kich thuoc cua so
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (width > 0 && height > 0) {
        windowWidth = width;
        windowHeight = height;
        glViewport(0, 0, width, height);
    }
}

// Cau hinh chu de mau sac hien dai cho giao dien ImGui
void setupImGuiTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding    = 8.0f;
    style.FrameRounding     = 5.0f;
    style.PopupRounding     = 5.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding       = 5.0f;
    style.WindowPadding     = ImVec2(12.0f, 12.0f);
    style.FramePadding      = ImVec2(8.0f, 5.0f);
    style.ItemSpacing       = ImVec2(8.0f, 6.0f);

    colors[ImGuiCol_Text]                  = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.55f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.10f, 0.13f, 0.18f, 0.88f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.12f, 0.15f, 0.20f, 0.94f);
    colors[ImGuiCol_Border]                = ImVec4(0.24f, 0.32f, 0.44f, 0.60f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.16f, 0.20f, 0.28f, 0.70f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.22f, 0.28f, 0.38f, 0.85f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.28f, 0.36f, 0.48f, 1.00f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.12f, 0.16f, 0.24f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.16f, 0.22f, 0.34f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.10f, 0.13f, 0.18f, 0.75f);
    colors[ImGuiCol_Button]                = ImVec4(0.20f, 0.32f, 0.48f, 0.75f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.28f, 0.44f, 0.65f, 0.90f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.34f, 0.52f, 0.78f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.20f, 0.32f, 0.48f, 0.75f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.28f, 0.44f, 0.65f, 0.85f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.34f, 0.52f, 0.78f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.38f, 0.65f, 0.95f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.50f, 0.78f, 1.00f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.38f, 0.85f, 0.55f, 1.00f);
    colors[ImGuiCol_Separator]             = ImVec4(0.24f, 0.32f, 0.44f, 0.60f);
}
