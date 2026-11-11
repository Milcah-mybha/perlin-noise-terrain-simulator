#include "PerlinNoise.h"
#include <numeric>
#include <random>
#include <algorithm>
#include <cmath>

PerlinNoise::PerlinNoise(uint32_t seed) {
    setSeed(seed);
}

void PerlinNoise::setSeed(uint32_t seed) {
    currentSeed = seed;
    p.resize(256);
    std::iota(p.begin(), p.end(), 0);

    std::mt19937 generator(seed);
    std::shuffle(p.begin(), p.end(), generator);
    p.insert(p.end(), p.begin(), p.end());
}

float PerlinNoise::grad(int hash, float x, float y) {
    int h = hash & 7;
    float u = h < 4 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float PerlinNoise::noise2D(float x, float y) const {
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;

    float xf = x - std::floor(x);
    float yf = y - std::floor(y);

    float u = fade(xf);
    float v = fade(yf);

    int A  = p[X] + Y;
    int AA = p[A];
    int AB = p[A + 1];
    int B  = p[X + 1] + Y;
    int BA = p[B];
    int BB = p[B + 1];

    float g1 = grad(p[AA], xf, yf);
    float g2 = grad(p[BA], xf - 1.0f, yf);
    float g3 = grad(p[AB], xf, yf - 1.0f);
    float g4 = grad(p[BB], xf - 1.0f, yf - 1.0f);

    float lerpX1 = lerp(g1, g2, u);
    float lerpX2 = lerp(g3, g4, u);

    return lerp(lerpX1, lerpX2, v);
}

float PerlinNoise::fbm2D(float x, float y, int octaves, float persistence, float lacunarity) const {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        total += noise2D(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= lacunarity;
    }

    if (maxValue <= 0.0f) return 0.5f;
    float normalized = (total / maxValue + 1.0f) * 0.5f;
    return std::clamp(normalized, 0.0f, 1.0f);
}

float PerlinNoise::ridged2D(float x, float y, int octaves, float persistence, float lacunarity) const {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float weight = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        float val = noise2D(x * frequency, y * frequency);
        val = 1.0f - std::abs(val);
        val = val * val;

        total += val * amplitude * weight;
        maxValue += amplitude;

        weight = std::clamp(val * 2.0f, 0.0f, 1.0f);
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    if (maxValue <= 0.0f) return 0.5f;
    return std::clamp(total / maxValue, 0.0f, 1.0f);
}

float PerlinNoise::terraced2D(float x, float y, int octaves, float steps) const {
    float h = fbm2D(x, y, octaves, 0.5f, 2.0f);
    float frac = (h * steps) - std::floor(h * steps);
    float smoothFrac = frac * frac * (3.0f - 2.0f * frac);
    return (std::floor(h * steps) + smoothFrac * 0.35f) / steps;
}

float PerlinNoise::island2D(float x, float y, float normalizedDist, int octaves, float persistence, float lacunarity) const {
    float qx = noise2D(x * 0.85f, y * 0.85f);
    float qy = noise2D(x * 0.85f + 5.2f, y * 0.85f + 1.3f);

    float rx = noise2D(x + 1.2f * qx, y + 1.2f * qy);
    float ry = noise2D(x + 1.2f * qx + 3.4f, y + 1.2f * qy + 7.1f);

    float wx = x + 0.45f * rx;
    float wy = y + 0.45f * ry;

    float baseFbm = fbm2D(wx, wy, octaves, persistence, lacunarity);
    float ridges  = ridged2D(wx * 1.3f, wy * 1.3f, std::max(1, octaves - 1), 0.5f, 2.0f);
    float terrain = baseFbm * 0.70f + ridges * 0.30f;

    float warpDist = noise2D(x * 0.75f + 1.7f, y * 0.75f + 2.8f) * 0.28f;
    float dist = normalizedDist + warpDist;

    float falloff = std::clamp((dist - 0.25f) / 0.65f, 0.0f, 1.0f);
    falloff = falloff * falloff;

    float finalH = terrain - falloff * 0.75f;
    return std::clamp(finalH, 0.0f, 1.0f);
}
