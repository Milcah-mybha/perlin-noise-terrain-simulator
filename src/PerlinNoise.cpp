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
    return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
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

    return lerp(lerpX1, lerpX2, v) / 1.5f;
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

float PerlinNoise::island2D(float x, float y, float normalizedDist, int octaves) const {
    float baseH = fbm2D(x, y, octaves, 0.5f, 2.0f);
    float islandShape = std::max(0.0f, 1.0f - normalizedDist * 1.35f);
    islandShape = islandShape * islandShape;
    float finalH = islandShape * 0.75f + baseH * 0.35f;
    return std::clamp(finalH, 0.0f, 1.0f);
}
