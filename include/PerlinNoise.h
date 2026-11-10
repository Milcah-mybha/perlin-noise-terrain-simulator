#ifndef PERLIN_NOISE_H
#define PERLIN_NOISE_H

#include <vector>
#include <cstdint>

enum PresetType {
    PRESET_ROLLING_HILLS = 0,    // Đồi núi nhấp nhô
    PRESET_ALPINE_RIDGES = 1,    // Dãy núi nhọn (Alpine)
    PRESET_ISLANDS = 2,          // Quần đảo đại dương
    PRESET_SINGLE_OCTAVE = 3,    // Đơn tầng (không chi tiết gồ ghề)
    PRESET_ROUGH_CRAGS = 4,      // Vách đá gồ ghề (Rough Crags)
    PRESET_TERRACED = 5          // Hẻm núi bậc thang
};

/**
 * @brief Lớp tính toán nhiễu Perlin 2D và các thuật toán mở rộng
 */
class PerlinNoise {
public:
    PerlinNoise(uint32_t seed = 2026);

    void setSeed(uint32_t seed);
    uint32_t getSeed() const { return currentSeed; }

    // Giá trị Perlin Noise 2D tại (x, y) trong [-1.0, 1.0]
    float noise2D(float x, float y) const;

    // 1. Fractal Brownian Motion (fBm)
    float fbm2D(float x, float y, int octaves, float persistence = 0.5f, float lacunarity = 2.0f) const;

    // 2. Ridged Multifractal (sống núi sắc nhọn)
    float ridged2D(float x, float y, int octaves, float persistence = 0.5f, float lacunarity = 2.0f) const;

    // 3. Terraced Noise (bậc thang trầm tích)
    float terraced2D(float x, float y, int octaves, float steps = 8.0f) const;

    // 4. Quần đảo (Islands)
    float island2D(float x, float y, float normalizedDist, int octaves) const;

private:
    uint32_t currentSeed;
    std::vector<int> p;

    static inline float fade(float t) {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    static inline float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    static float grad(int hash, float x, float y);
};

#endif // PERLIN_NOISE_H
