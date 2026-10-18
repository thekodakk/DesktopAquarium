#ifndef AQUARIUM_H
#define AQUARIUM_H

#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <memory>
#include "Fish.h"

struct FoodPellet {
    float x;
    float y;
    float vy;
    float size;
    bool active;
};

struct SplashParticle {
    float x;
    float y;
    float vx;
    float vy;
    float alpha;
    float size;
    float lifetime;
    bool active;
};

struct AirBubble {
    float x;
    float y;
    float vy;
    float radius;
    float wobblePhase;
};

class Aquarium {
public:
    Aquarium(int width, int height);

    void Update(float dt, int windowX, int windowY, bool isDragging);
    void Draw(Gdiplus::Graphics& g) const;

    // Actions
    void FeedFish();
    void RefillWater();
    void SetFishTypeActive(FishType type, bool active);
    bool IsFishTypeActive(FishType type) const;
    void ToggleAllFish(bool active);

    // Getters
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    float GetWaterLevelRatio() const { return m_currentWaterHeight / m_maxWaterHeight; }
    float GetHungerProgress() const { return m_hungerProgress; }
    bool IsHungry() const { return m_hungerProgress >= 0.8f; }
    bool CanFeed() const { return m_hungerProgress >= 0.15f && m_feedCooldown <= 0.0f; }
    float GetFeedCooldown() const { return m_feedCooldown; }
    float GetAnimTime() const { return m_animTime; }

    const std::vector<Fish>& GetFishes() const { return m_fishes; }
    const std::vector<FoodPellet>& GetFoodPellets() const { return m_foodPellets; }
    const std::vector<SplashParticle>& GetSplashParticles() const { return m_splashParticles; }
    const std::vector<AirBubble>& GetAirBubbles() const { return m_airBubbles; }

    // Hit Testing
    bool IsFeedButtonClicked(int clickX, int clickY) const;

private:
    int m_width;
    int m_height;
    float m_animTime;
    float m_feedCooldown;

    // Window position tracking for drag velocity calculation
    int m_lastWindowX;
    int m_lastWindowY;
    float m_dragSpeed;
    bool m_wasDragging;

    // Water Level & Spill Physics
    float m_maxWaterHeight;
    float m_currentWaterHeight;
    float m_sloshAngle;       // Tilt angle of water surface when dragging
    float m_sloshVelocity;
    float m_timeSinceLastSpill;
    bool m_isRefilling;

    // Hunger System
    float m_hungerProgress;   // 0.0 (Satisfied) to 1.0 (Starving)
    float m_hungerRate;       // Fills meter over ~35 seconds

    // Collections
    std::vector<Fish> m_fishes;
    std::vector<FoodPellet> m_foodPellets;
    std::vector<SplashParticle> m_splashParticles;
    std::vector<AirBubble> m_airBubbles;

    // Helpers
    void UpdateWaterPhysics(float dt, float currentVelocityX, float currentVelocityY);
    void EmitSplash(float amount);
    void UpdateFoodAndFeeding(float dt);
    void UpdateBubbles(float dt);
};

#endif // AQUARIUM_H
