#ifndef RENDERER_H
#define RENDERER_H

#include <windows.h>
#include <gdiplus.h>
#include "Aquarium.h"

class Renderer {
public:
    static void DrawAquarium(Gdiplus::Graphics& g, const Aquarium& aquarium);

private:
    static void DrawTankFrame(Gdiplus::Graphics& g, int width, int height);
    static void DrawGravelAndPlants(Gdiplus::Graphics& g, int width, int height, float animTime);
    static void DrawWaterBody(Gdiplus::Graphics& g, const Aquarium& aquarium);
    static void DrawBubbles(Gdiplus::Graphics& g, const Aquarium& aquarium);
    static void DrawFoodPellets(Gdiplus::Graphics& g, const Aquarium& aquarium);
    static void DrawSplashParticles(Gdiplus::Graphics& g, const Aquarium& aquarium);
    static void DrawHungerHeader(Gdiplus::Graphics& g, const Aquarium& aquarium);
};

#endif // RENDERER_H
