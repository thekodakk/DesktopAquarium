#include "Aquarium.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Aquarium::Aquarium(int width, int height)
    : m_width(width), m_height(height), m_animTime(0.0f), m_feedCooldown(0.0f),
      m_lastWindowX(-1), m_lastWindowY(-1), m_dragSpeed(0.0f), m_wasDragging(false),
      m_maxWaterHeight(height - 80.0f), m_currentWaterHeight(height - 80.0f),
      m_sloshAngle(0.0f), m_sloshVelocity(0.0f), m_timeSinceLastSpill(0.0f), m_isRefilling(false),
      m_hungerProgress(0.0f), m_hungerRate(1.0f / 35.0f) // 35 seconds to reach full hunger
{
    // Initialize 10 distinct fish species in the aquarium
    for (int i = 0; i < FISH_COUNT; ++i) {
        FishType type = static_cast<FishType>(i);
        float startX = 60.0f + (rand() % (width - 120));
        float startY = 100.0f + (rand() % (height - 180));
        m_fishes.emplace_back(type, startX, startY);
    }

    // Initialize decorative ambient air bubbles
    for (int i = 0; i < 15; ++i) {
        AirBubble b;
        b.x = (width * 0.5f) + (rand() % 80 - 40);
        b.y = height - 50.0f - (rand() % (height - 100));
        b.vy = 25.0f + (rand() % 30);
        b.radius = 2.0f + (rand() % 40) / 10.0f;
        b.wobblePhase = (rand() % 100) / 10.0f;
        m_airBubbles.push_back(b);
    }
}

void Aquarium::Update(float dt, int windowX, int windowY, bool isDragging) {
    if (dt <= 0.0f) return;
    m_animTime += dt;
    if (m_feedCooldown > 0.0f) m_feedCooldown -= dt;

    // Calculate window velocity vector when dragged
    float velocityX = 0.0f;
    float velocityY = 0.0f;

    if (m_lastWindowX != -1 && m_lastWindowY != -1 && dt > 0.001f) {
        velocityX = (windowX - m_lastWindowX) / dt;
        velocityY = (windowY - m_lastWindowY) / dt;
    }
    m_lastWindowX = windowX;
    m_lastWindowY = windowY;

    m_dragSpeed = std::sqrt(velocityX * velocityX + velocityY * velocityY);

    // Update Water Sloshing & Spill Physics
    UpdateWaterPhysics(dt, velocityX, velocityY);

    // Update Hunger Meter
    m_hungerProgress += m_hungerRate * dt;
    if (m_hungerProgress > 1.0f) m_hungerProgress = 1.0f;

    // Update Food Pellets & Fish Eating Mechanics
    UpdateFoodAndFeeding(dt);

    // Update Air Bubbles
    UpdateBubbles(dt);

    // Tank bottom gravel level and water top surface level
    float tankBottomY = m_height - 35.0f;
    float waterTopY = tankBottomY - m_currentWaterHeight;

    // Nearest food target for fish steering
    float activeFoodX = 0.0f;
    float activeFoodY = 0.0f;
    bool hasActiveFood = false;

    for (const auto& pellet : m_foodPellets) {
        if (pellet.active) {
            activeFoodX = pellet.x;
            activeFoodY = pellet.y;
            hasActiveFood = true;
            break;
        }
    }

    // Update all fish
    for (auto& fish : m_fishes) {
        fish.Update(dt, (float)m_width, waterTopY, tankBottomY, activeFoodX, activeFoodY, hasActiveFood);
    }

    // Update Water Splash Particles
    for (auto& p : m_splashParticles) {
        if (!p.active) continue;
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.vy += 450.0f * dt; // Gravity
        p.alpha -= 1.8f * dt;
        p.lifetime -= dt;
        if (p.alpha <= 0.0f || p.lifetime <= 0.0f) {
            p.active = false;
        }
    }

    m_wasDragging = isDragging;
}

void Aquarium::UpdateWaterPhysics(float dt, float velocityX, float velocityY) {
    // Water Surface Wave Slosh Spring Simulation
    float targetAngle = -velocityX * 0.0008f; // Tilt opposite to drag direction
    targetAngle = std::max(-0.35f, std::min(0.35f, targetAngle));

    float springK = 25.0f;
    float damping = 4.0f;

    float angleAccel = (targetAngle - m_sloshAngle) * springK - m_sloshVelocity * damping;
    m_sloshVelocity += angleAccel * dt;
    m_sloshAngle += m_sloshVelocity * dt;

    // Check Spill Threshold (Hızlı hareket edince sular dökülsün!)
    float spillSpeedLimit = 750.0f; // Pixels per second drag speed threshold
    if (m_dragSpeed > spillSpeedLimit) {
        float speedExcess = m_dragSpeed - spillSpeedLimit;
        float spillAmount = speedExcess * 0.08f * dt;

        m_currentWaterHeight -= spillAmount;
        if (m_currentWaterHeight < m_maxWaterHeight * 0.25f) {
            m_currentWaterHeight = m_maxWaterHeight * 0.25f; // Keep at least 25% water
        }

        // Trigger splash particles & fish reaction
        EmitSplash(speedExcess * 0.02f);
        m_timeSinceLastSpill = 0.0f;
        m_isRefilling = false;

        for (auto& fish : m_fishes) {
            fish.ReactToSpill();
        }
    } else {
        m_timeSinceLastSpill += dt;
    }

    // Auto-Refill Physics (Belli bir süre sonra geri gelsin)
    // Refill begins 2.8 seconds after fast motion stops
    if (m_timeSinceLastSpill > 2.8f && m_currentWaterHeight < m_maxWaterHeight) {
        m_isRefilling = true;
        float refillSpeed = (m_maxWaterHeight - m_currentWaterHeight) * 0.8f * dt + 15.0f * dt;
        m_currentWaterHeight += refillSpeed;
        if (m_currentWaterHeight >= m_maxWaterHeight) {
            m_currentWaterHeight = m_maxWaterHeight;
            m_isRefilling = false;
        }
    }
}

void Aquarium::EmitSplash(float intensity) {
    int count = (int)std::min(15.0f, 3.0f + intensity);
    float tankBottomY = m_height - 35.0f;
    float surfaceY = tankBottomY - m_currentWaterHeight;

    for (int i = 0; i < count; ++i) {
        SplashParticle p;
        p.x = 20.0f + (rand() % (m_width - 40));
        p.y = surfaceY;
        p.vx = (rand() % 300 - 150) * 1.2f;
        p.vy = -(120.0f + (rand() % 200));
        p.alpha = 1.0f;
        p.size = 3.0f + (rand() % 50) / 10.0f;
        p.lifetime = 0.6f + (rand() % 40) / 100.0f;
        p.active = true;
        m_splashParticles.push_back(p);
    }
}

void Aquarium::Draw(Gdiplus::Graphics& g) const {
    for (const auto& fish : m_fishes) {
        if (fish.IsActive()) {
            fish.Draw(g);
        }
    }
}

void Aquarium::FeedFish() {
    if (!CanFeed()) return;

    m_feedCooldown = 1.2f; // 1.2 seconds cooldown between feed clicks

    float tankBottomY = m_height - 35.0f;
    float surfaceY = tankBottomY - m_currentWaterHeight;

    // Limit active food pellets to prevent lag
    if (m_foodPellets.size() >= 16) return;

    int spawnCount = std::min(6, 16 - (int)m_foodPellets.size());

    for (int i = 0; i < spawnCount; ++i) {
        FoodPellet p;
        p.x = (m_width * 0.2f) + (rand() % (int)(m_width * 0.6f));
        p.y = surfaceY + (rand() % 15);
        p.vy = 22.0f + (rand() % 25);
        p.size = 4.0f + (rand() % 20) / 10.0f;
        p.active = true;
        m_foodPellets.push_back(p);
    }

    m_hungerProgress = std::max(0.0f, m_hungerProgress - 0.25f);
}

void Aquarium::RefillWater() {
    m_currentWaterHeight = m_maxWaterHeight;
    m_isRefilling = false;
    m_timeSinceLastSpill = 5.0f;
}

void Aquarium::UpdateFoodAndFeeding(float dt) {
    float tankBottomY = m_height - 35.0f;

    for (auto& pellet : m_foodPellets) {
        if (!pellet.active) continue;

        pellet.y += pellet.vy * dt;

        // Stop at gravel bed
        if (pellet.y >= tankBottomY - 10.0f) {
            pellet.y = tankBottomY - 10.0f;
            pellet.vy = 0.0f;
        }

        // Check collision / eating by fish
        for (auto& fish : m_fishes) {
            if (!fish.IsActive()) continue;

            float fx = fish.GetX() + fish.GetWidth() * 0.5f;
            float fy = fish.GetY() + fish.GetHeight() * 0.5f;
            float dx = fx - pellet.x;
            float dy = fy - pellet.y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist < (fish.GetWidth() * 0.4f + pellet.size)) {
                pellet.active = false; // Fish eats pellet!
                fish.EatFood();
                m_hungerProgress = std::max(0.0f, m_hungerProgress - 0.15f);
                break;
            }
        }
    }

    // Clean up inactive food pellets
    m_foodPellets.erase(
        std::remove_if(m_foodPellets.begin(), m_foodPellets.end(),
                       [](const FoodPellet& p) { return !p.active; }),
        m_foodPellets.end()
    );
}

void Aquarium::UpdateBubbles(float dt) {
    float tankBottomY = m_height - 35.0f;
    float waterTopY = tankBottomY - m_currentWaterHeight;

    for (auto& b : m_airBubbles) {
        b.y -= b.vy * dt;
        b.wobblePhase += 4.0f * dt;
        b.x += std::sin(b.wobblePhase) * 12.0f * dt;

        if (b.y <= waterTopY) {
            // Reset bubble back to bottom bubbler stone
            b.y = tankBottomY - 12.0f;
            b.x = (m_width * 0.5f) + (rand() % 90 - 45);
        }
    }
}

void Aquarium::SetFishTypeActive(FishType type, bool active) {
    for (auto& fish : m_fishes) {
        if (fish.GetType() == type) {
            fish.SetActive(active);
        }
    }
}

bool Aquarium::IsFishTypeActive(FishType type) const {
    for (const auto& fish : m_fishes) {
        if (fish.GetType() == type) {
            return fish.IsActive();
        }
    }
    return false;
}

void Aquarium::ToggleAllFish(bool active) {
    for (auto& fish : m_fishes) {
        fish.SetActive(active);
    }
}

bool Aquarium::IsFeedButtonClicked(int clickX, int clickY) const {
    // Top header feed button hit test box
    int btnX = m_width - 110;
    int btnY = 12;
    int btnW = 90;
    int btnH = 26;

    return (clickX >= btnX && clickX <= btnX + btnW &&
            clickY >= btnY && clickY <= btnY + btnH);
}
