// Định nghĩa lớp thuật toán Perlin Noise và các hàm toán học sinh địa hình
#ifndef PERLIN_NOISE_H
#define PERLIN_NOISE_H

#include <vector>
#include <cstdint>

enum PresetType {
    PRESET_ISLANDS = 0,
    PRESET_ROLLING_HILLS = 1,
    PRESET_ALPINE_RIDGES = 2,
    PRESET_TERRACED = 3,
    PRESET_VOLCANO = 4,
    PRESET_SINGLE_OCTAVE = 5
};

class PerlinNoise {
public:
    PerlinNoise(uint32_t seed = 2026);

    void setSeed(uint32_t seed);
    uint32_t getSeed() const { return currentSeed; }

    // Nhiễu Perlin 2D cơ sở [-1.0, 1.0]
    float noise2D(float x, float y) const;

    // Fractal Brownian Motion (cộng dồn đa tầng)
    float fbm2D(float x, float y, int octaves, float persistence = 0.5f, float lacunarity = 2.0f) const;

    // Ridged Multifractal (sống núi sắc nhọn)
    float ridged2D(float x, float y, int octaves, float persistence = 0.5f, float lacunarity = 2.0f) const;

    // Terraced Noise (bậc thang trầm tích)
    float terraced2D(float x, float y, int octaves, float steps = 8.0f) const;

    // Thuật toán mặt nạ quần đảo tự nhiên
    float island2D(float x, float y, float normalizedDist, int octaves, float persistence = 0.5f, float lacunarity = 2.0f) const;

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
