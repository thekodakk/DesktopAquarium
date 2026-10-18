#ifndef FISH_H
#define FISH_H

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <cmath>

enum FishType {
    FISH_GOLDFISH = 0,    // Japon Balığı (Bright Orange, flowing tail)
    FISH_NEON_TETRA,      // Neon Balığı (Electric blue stripe, red tail)
    FISH_CLOWNFISH,       // Palyaço Balığı (Orange with white stripes)
    FISH_GUPPY,           // Lepidestes (Colorful fan tail)
    FISH_ANGELFISH,       // Melek Balığı (Tall silver triangular body)
    FISH_BETTA,           // Dövüşçü Balık (Deep purple/red expansive fins)
    FISH_BLUE_TANG,       // Mavi Cerrah (Cobalt blue with yellow tail)
    FISH_YELLOW_LAB,      // Sarı Prenses (Canary yellow with dark fin edge)
    FISH_DISCUS,          // Diskus (Turquoise disc body)
    FISH_CORYDORAS,       // Çöpçü Balığı (Bottom dweller with whiskers)
    FISH_AXOLOTL,         // Aksolotl (Pembe, dış solungaçlı sevimli aksolotl)
    FISH_COUNT
};

struct FishTypeInfo {
    FishType type;
    std::wstring name;
    std::wstring description;
    float defaultWidth;
    float defaultHeight;
    float maxSpeed;
};

// Global species info getter
const FishTypeInfo& GetFishInfo(FishType type);

class Fish {
public:
    Fish(FishType type, float startX, float startY);

    void Update(float dt, float tankWidth, float waterTopY, float tankBottomY, 
                float foodX, float foodY, bool foodAvailable);
    void Draw(Gdiplus::Graphics& g) const;

    // Getters and Setters
    FishType GetType() const { return m_type; }
    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }
    
    float GetX() const { return m_x; }
    float GetY() const { return m_y; }
    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }

    void ReactToSpill();
    void EatFood();

private:
    FishType m_type;
    bool m_active;

    float m_x;
    float m_y;
    float m_vx;
    float m_vy;
    float m_targetX;
    float m_targetY;

    float m_width;
    float m_height;
    float m_maxSpeed;

    bool m_facingLeft;
    float m_tailPhase;
    float m_tailSpeed;
    float m_targetTimer;

    // Custom helper drawing functions for each of the 10 species
    void DrawGoldfish(Gdiplus::Graphics& g) const;
    void DrawNeonTetra(Gdiplus::Graphics& g) const;
    void DrawClownfish(Gdiplus::Graphics& g) const;
    void DrawGuppy(Gdiplus::Graphics& g) const;
    void DrawAngelfish(Gdiplus::Graphics& g) const;
    void DrawBetta(Gdiplus::Graphics& g) const;
    void DrawBlueTang(Gdiplus::Graphics& g) const;
    void DrawYellowLab(Gdiplus::Graphics& g) const;
    void DrawDiscus(Gdiplus::Graphics& g) const;
    void DrawCorydoras(Gdiplus::Graphics& g) const;
    void DrawAxolotl(Gdiplus::Graphics& g) const;
};

#endif // FISH_H
