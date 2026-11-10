# MÔ PHỎNG PHÒNG THÍ NGHIỆM ĐỊA HÌNH PERLIN NOISE 3D (C++ / OPENGL)
> **Đề tài Môn học:** Đồ họa Máy tính (Computer Graphics)  
> **Giao diện:** Bảng điều khiển Tiếng Việt chuẩn Unicode (chống lỗi phông chữ)  
> **Ngôn ngữ & Công nghệ:** C++17, Modern OpenGL 3.3 Core Profile, GLSL, GLM, GLFW, GLAD, Dear ImGui

---

## 1. TỔNG QUAN BẢNG ĐIỀU KHIỂN TIẾNG VIỆT

Giao diện được thiết kế hiện đại bám sát mô hình **"Perlin Noise Terrain Lab 3D"**:

1. **Thẻ thông số nổi (Badges):**
   * **Số tầng (Octaves):** Số lớp nhiễu xếp chồng.
   * **Khoảng cao:** Độ cao tối thiểu đến tối đa của địa hình ($m$).
   * **Tỷ lệ đất liền (%):** Tỷ lệ diện tích đất nhô lên trên mặt nước biển thời gian thực.
2. **Các dạng địa hình mẫu (Presets) - Bấm 1 chạm:**
   * **Đồi núi nhấp nhô (Rolling hills):** Dạng đồi núi tự nhiên mượt mà kinh điển.
   * **Dãy núi nhọn (Alpine ridges):** Dãy núi đá tuyết nhọn hiểm trở (Ridged Multifractal).
   * **Quần đảo đại dương (Islands):** Địa hình đảo thoai thoải bao bọc bởi biển cả.
   * **Đơn tầng (Single octave):** Địa hình chỉ có 1 tầng nhiễu nguyên thủy không gồ ghề.
   * **Vách đá gồ ghề (Rough crags):** Địa hình tai mèo đá sỏi hiểm trở độ nhám cao.
   * **Hẻm núi bậc thang (Terraced canyon):** Các tầng bậc thang trầm tích địa chất.
3. **Điều khiển hoạt họa (Playback):**
   * Tạm dừng / Tiếp tục chu kỳ quay mặt trời.
   * Đổi hạt giống ngẫu nhiên (Random Seed).
   * Đặt lại góc nhìn mặc định (Reset view).
4. **Chế độ hiển thị (View):**
   * Bảng màu sinh thái (Biomes), bản đồ nhiệt (Heatmap), thang xám (Grayscale).
   * Bật / tắt chế độ Khung dây (Wireframe Mesh).
   * Bật / tắt Sương mù (Fog) và Mực nước biển.
5. **Cấu hình thông số nhiễu (Noise Settings):**
   * Kéo slider tùy biến: Tần số ô lưới, Số tầng Octaves, Hệ số Lacunarity, Hệ số Persistence, Độ cao núi, Độ dốc lũy thừa.

---

## 2. GIẢI PHÁP CHỐNG LỖI PHÔNG CHỮ TIẾNG VIỆT

Để đảm bảo các chữ Tiếng Việt có dấu (`Đồi núi`, `Quần đảo`, `Chiều cao`...) hiển thị sắc nét 100% không bị lỗi ô vuông hay dấu `?`:
* Dự án tích hợp trực tiếp tệp phông chữ Unicode TrueType: `assets/fonts/OpenSans-Regular.ttf`.
* Nạp dải ký tự `io.Fonts->GetGlyphRangesVietnamese()` của Dear ImGui.
* Có cơ chế tự động nạp font dự phòng (`Segoe UI` / `Arial` trên Windows; `Liberation Sans` trên Linux).
* Cấu hình cờ `/utf-8` trong CMake để trình biên dịch MSVC trên Windows Visual Studio luôn dịch đúng mã UTF-8.

---

## 3. HƯỚNG DẪN THAO TÁC

* **Chạy chương trình:** Nhấn **`F5`** trong VS Code hoặc gõ `make run`.
* **Thao tác chuột:**
  * **Giữ Chuột Phải:** Khóa chuột để quay góc nhìn 360 độ và bay lượn bằng `W, A, S, D, Space, Shift`.
  * **Thả Chuột Phải:** Hiện con trỏ chuột để thao tác nút bấm và kéo slider trên Bảng điều khiển Tiếng Việt.
* **Phím tắt nhanh:**
  * `TAB`: Bật/tắt nhanh khung dây.
  * `ESC`: Thoát ứng dụng.
