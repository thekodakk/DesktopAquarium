#include "Fish.h"
#include "AssetManager.h"
#include <cstdlib>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const FishTypeInfo G_FISH_INFO[FISH_COUNT] = {
    { FISH_GOLDFISH,   L"Japon Balığı",     L"Parlak turuncu, tül kuyruklu klasik japon balığı.", 42.0f, 26.0f, 65.0f },
    { FISH_NEON_TETRA, L"Neon Balığı",      L"Elektrik mavisi ve parlak kırmızı çizgili neoni.",   30.0f, 16.0f, 90.0f },
    { FISH_CLOWNFISH,  L"Palyaço Balığı",   L"Turuncu-beyaz çizgli palyaço (Nemo).",              38.0f, 22.0f, 75.0f },
    { FISH_GUPPY,      L"Lepidestes",       L"Gökkuşağı desenli büyük yelpaze kuyruklu lepistes.",34.0f, 20.0f, 80.0f },
    { FISH_ANGELFISH,  L"Melek Balığı",     L"Üçgen gümüş gövdeli uzun yüzgeçli melek balığı.",   36.0f, 44.0f, 60.0f },
    { FISH_BETTA,      L"Dövüşçü Balık",    L"Koyu mor-kırmızı gösterişli geniş yüzgeçli Betta.",  40.0f, 30.0f, 55.0f },
    { FISH_BLUE_TANG,  L"Mavi Cerrah",      L"Koyu kobalt mavisi ve sarı kuyruklu Cerrah balığı.",38.0f, 24.0f, 85.0f },
    { FISH_YELLOW_LAB, L"Sarı Prenses",     L"Canlı sarı renkli prenses ciklet balığı.",           40.0f, 22.0f, 70.0f },
    { FISH_DISCUS,     L"Diskus Balığı",    L"Turkuaz desenli yuvarlak diskus balığı.",           44.0f, 42.0f, 50.0f },
    { FISH_CORYDORAS,  L"Çöpçü Balığı",     L"Tabanda bıyıklarıyla gezen sevimli çöpçü balığı.",  36.0f, 20.0f, 60.0f },
    { FISH_AXOLOTL,    L"Pembe Aksolotl",   L"Tabanda süzülen dış solungaçlı sevimli pembe Aksolotl.", 46.0f, 26.0f, 45.0f }
};

const FishTypeInfo& GetFishInfo(FishType type) {
    if (type >= 0 && type < FISH_COUNT) {
        return G_FISH_INFO[type];
    }
    return G_FISH_INFO[0];
}

Fish::Fish(FishType type, float startX, float startY)
    : m_type(type), m_active(true), m_x(startX), m_y(startY),
      m_vx(0.0f), m_vy(0.0f), m_facingLeft(false), m_tailPhase(0.0f),
      m_targetTimer(0.0f) 
{
    const auto& info = GetFishInfo(type);
    m_width = info.defaultWidth;
    m_height = info.defaultHeight;
    m_maxSpeed = info.maxSpeed;

    // Randomize initial tail phase and speed
    m_tailSpeed = 6.0f + (rand() % 100) / 20.0f;
    m_targetX = startX + (rand() % 100 - 50);
    m_targetY = startY + (rand() % 60 - 30);
}

void Fish::ReactToSpill() {
    // Quick burst reaction downwards when water spills
    m_vy += 40.0f + (rand() % 40);
    m_targetTimer = 0.0f;
}

void Fish::EatFood() {
    // Excited wiggle when eating food
    m_tailSpeed = 15.0f;
}

void Fish::Update(float dt, float tankWidth, float waterTopY, float tankBottomY,
                  float foodX, float foodY, bool foodAvailable) 
{
    if (!m_active) return;

    m_tailPhase += m_tailSpeed * dt;
    if (m_tailPhase > 2.0f * M_PI) {
        m_tailPhase -= 2.0f * M_PI;
    }

    // Keep tail speed returning to normal float
    m_tailSpeed += (7.0f - m_tailSpeed) * dt;

    m_targetTimer -= dt;

    // Corydoras & Axolotl stay close to gravel at bottom
    bool isBottomDweller = (m_type == FISH_CORYDORAS || m_type == FISH_AXOLOTL);
    float preferredBottom = isBottomDweller ? (tankBottomY - m_height - 4.0f) : (tankBottomY - m_height - 15.0f);
    float preferredTop = isBottomDweller ? (tankBottomY - m_height - 30.0f) : (waterTopY + 15.0f);

    if (preferredTop > preferredBottom) {
        preferredTop = preferredBottom - 10.0f;
    }

    if (foodAvailable) {
        // Swim towards target food pellet
        m_targetX = foodX;
        m_targetY = std::max(preferredTop, std::min(foodY, preferredBottom));
    } else if (m_targetTimer <= 0.0f) {
        // Pick new random target inside valid water bounds
        float margin = 30.0f;
        m_targetX = margin + (rand() % (int)std::max(10.0f, tankWidth - margin * 2));
        m_targetY = preferredTop + (rand() % (int)std::max(10.0f, preferredBottom - preferredTop));

        m_targetTimer = 2.0f + (rand() % 300) / 100.0f; // 2 to 5 seconds
    }

    // Smooth steering physics towards target position
    float dx = m_targetX - m_x;
    float dy = m_targetY - m_y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist > 5.0f) {
        float speed = m_maxSpeed;
        if (foodAvailable) speed *= 1.4f; // Swim faster towards food!

        float targetVx = (dx / dist) * speed;
        float targetVy = (dy / dist) * speed;

        m_vx += (targetVx - m_vx) * 3.0f * dt;
        m_vy += (targetVy - m_vy) * 3.0f * dt;
    } else {
        m_vx *= 0.95f;
        m_vy *= 0.95f;
    }

    // Update position
    m_x += m_vx * dt;
    m_y += m_vy * dt;

    // Orientation
    if (m_vx < -2.0f) m_facingLeft = true;
    else if (m_vx > 2.0f) m_facingLeft = false;

    // Hard boundary constraints (stay inside water volume)
    float minX = 15.0f;
    float maxX = tankWidth - m_width - 15.0f;
    if (m_x < minX) { m_x = minX; m_vx = std::abs(m_vx); m_targetTimer = 0.0f; }
    if (m_x > maxX) { m_x = maxX; m_vx = -std::abs(m_vx); m_targetTimer = 0.0f; }

    float minY = preferredTop;
    float maxY = preferredBottom;
    if (m_y < minY) { m_y = minY; m_vy = std::abs(m_vy); m_targetTimer = 0.0f; }
    if (m_y > maxY) { m_y = maxY; m_vy = -std::abs(m_vy); m_targetTimer = 0.0f; }
}

void Fish::Draw(Gdiplus::Graphics& g) const {
    if (!m_active) return;

    Gdiplus::Bitmap* sprite = nullptr;
    switch (m_type) {
        case FISH_GOLDFISH:   sprite = AssetManager::GetGoldfish(); break;
        case FISH_CLOWNFISH:  sprite = AssetManager::GetClownfish(); break;
        case FISH_BLUE_TANG:  sprite = AssetManager::GetBlueTang(); break;
        case FISH_AXOLOTL:    sprite = AssetManager::GetAxolotl(); break;
        default:
            if (m_type == FISH_YELLOW_LAB || m_type == FISH_NEON_TETRA) {
                sprite = AssetManager::GetGoldfish();
            } else if (m_type == FISH_BETTA || m_type == FISH_DISCUS) {
                sprite = AssetManager::GetBlueTang();
            } else {
                sprite = AssetManager::GetClownfish();
            }
            break;
    }

    if (!sprite) return;

    Gdiplus::GraphicsState state = g.Save();
    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    float centerX = m_x + m_width * 0.5f;
    float centerY = m_y + m_height * 0.5f;

    g.TranslateTransform(centerX, centerY);

    // Natural swimming float sway (pitch oscillation)
    float pitchAngle = std::sin(m_tailPhase) * 4.0f;
    g.RotateTransform(pitchAngle);

    // Flip horizontally if facing left
    if (m_facingLeft) {
        g.ScaleTransform(-1.0f, 1.0f);
    }

    g.DrawImage(sprite, -m_width * 0.5f, -m_height * 0.5f, m_width, m_height);

    g.Restore(state);
}

// -------------------------------------------------------------
// Procedural GDI+ Renderers for 10 Fish Species
// -------------------------------------------------------------

void Fish::DrawGoldfish(Gdiplus::Graphics& g) const {
    float tailWiggle = std::sin(m_tailPhase) * 6.0f;

    // Tail fin (Flowing fan tail)
    Gdiplus::SolidBrush tailBrush(Gdiplus::Color(230, 255, 120, 20));
    Gdiplus::PointF tailPts[3] = {
        Gdiplus::PointF(-m_width * 0.3f, 0.0f),
        Gdiplus::PointF(-m_width * 0.55f, -m_height * 0.45f + tailWiggle),
        Gdiplus::PointF(-m_width * 0.55f, m_height * 0.45f + tailWiggle)
    };
    g.FillPolygon(&tailBrush, tailPts, 3);

    // Body (Round orange)
    Gdiplus::SolidBrush bodyBrush(Gdiplus::Color(255, 255, 140, 0));
    g.FillEllipse(&bodyBrush, -m_width * 0.35f, -m_height * 0.45f, m_width * 0.7f, m_height * 0.9f);

    // Eye
    Gdiplus::SolidBrush eyeWhite(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::SolidBrush eyeBlack(Gdiplus::Color(255, 20, 20, 20));
    g.FillEllipse(&eyeWhite, m_width * 0.12f, -m_height * 0.25f, 7.0f, 7.0f);
    g.FillEllipse(&eyeBlack, m_width * 0.16f, -m_height * 0.20f, 3.5f, 3.5f);
}

void Fish::DrawNeonTetra(Gdiplus::Graphics& g) const {
    float tailWiggle = std::sin(m_tailPhase) * 4.0f;

    // Tail
    Gdiplus::SolidBrush redTail(Gdiplus::Color(240, 240, 30, 30));
    Gdiplus::PointF tailPts[3] = {
        Gdiplus::PointF(-m_width * 0.25f, 0.0f),
        Gdiplus::PointF(-m_width * 0.5f, -m_height * 0.35f + tailWiggle),
        Gdiplus::PointF(-m_width * 0.5f, m_height * 0.35f + tailWiggle)
    };
    g.FillPolygon(&redTail, tailPts, 3);

    // Body base
    Gdiplus::SolidBrush silverBody(Gdiplus::Color(240, 210, 220, 230));
    g.FillEllipse(&silverBody, -m_width * 0.4f, -m_height * 0.4f, m_width * 0.75f, m_height * 0.8f);

    // Neon Blue Stripe
    Gdiplus::Pen neonBluePen(Gdiplus::Color(255, 0, 210, 255), 3.5f);
    g.DrawLine(&neonBluePen, -m_width * 0.3f, -m_height * 0.08f, m_width * 0.2f, -m_height * 0.08f);

    // Red belly
    Gdiplus::SolidBrush redBelly(Gdiplus::Color(240, 230, 40, 40));
    g.FillEllipse(&redBelly, -m_width * 0.2f, 0.0f, m_width * 0.4f, m_height * 0.35f);

    // Eye
    Gdiplus::SolidBrush eyeBlack(Gdiplus::Color(255, 10, 10, 30));
    g.FillEllipse(&eyeBlack, m_width * 0.15f, -m_height * 0.2f, 4.0f, 4.0f);
}

void Fish::DrawClownfish(Gdiplus::Graphics& g) const {
    float tailWiggle = std::sin(m_tailPhase) * 5.0f;

    // Tail
    Gdiplus::SolidBrush orangeTail(Gdiplus::Color(255, 255, 120, 0));
    Gdiplus::PointF tailPts[3] = {
        Gdiplus::PointF(-m_width * 0.25f, 0.0f),
        Gdiplus::PointF(-m_width * 0.5f, -m_height * 0.35f + tailWiggle),
        Gdiplus::PointF(-m_width * 0.5f, m_height * 0.35f + tailWiggle)
    };
    g.FillPolygon(&orangeTail, tailPts, 3);

    // Main Body
    Gdiplus::SolidBrush orangeBody(Gdiplus::Color(255, 255, 125, 10));
    g.FillEllipse(&orangeBody, -m_width * 0.4f, -m_height * 0.45f, m_width * 0.75f, m_height * 0.9f);

    // White Vertical Stripes
    Gdiplus::SolidBrush whiteStripe(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::Pen blackPen(Gdiplus::Color(255, 20, 20, 20), 1.5f);

    // Stripe 1
    g.FillRectangle(&whiteStripe, -m_width * 0.15f, -m_height * 0.42f, 5.0f, m_height * 0.84f);
    g.DrawRectangle(&blackPen, -m_width * 0.15f, -m_height * 0.42f, 5.0f, m_height * 0.84f);

    // Stripe 2
    g.FillRectangle(&whiteStripe, m_width * 0.05f, -m_height * 0.38f, 5.0f, m_height * 0.76f);
    g.DrawRectangle(&blackPen, m_width * 0.05f, -m_height * 0.38f, 5.0f, m_height * 0.76f);

    // Eye
    Gdiplus::SolidBrush eyeWhite(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::SolidBrush eyeBlack(Gdiplus::Color(255, 0, 0, 0));
    g.FillEllipse(&eyeWhite, m_width * 0.18f, -m_height * 0.22f, 6.0f, 6.0f);
    g.FillEllipse(&eyeBlack, m_width * 0.21f, -m_height * 0.18f, 3.0f, 3.0f);
}

void Fish::DrawGuppy(Gdiplus::Graphics& g) const {
    float tailWiggle = std::sin(m_tailPhase) * 8.0f;

    // Big Colorful Veil Tail
    Gdiplus::SolidBrush tailBrush(Gdiplus::Color(230, 230, 50, 200)); // Vivid magenta
    Gdiplus::PointF tailPts[4] = {
        Gdiplus::PointF(-m_width * 0.2f, 0.0f),
        Gdiplus::PointF(-m_width * 0.58f, -m_height * 0.55f + tailWiggle),
        Gdiplus::PointF(-m_width * 0.52f, tailWiggle * 0.5f),
        Gdiplus::PointF(-m_width * 0.58f, m_height * 0.55f + tailWiggle)
    };
    g.FillPolygon(&tailBrush, tailPts, 4);

    // Body
    Gdiplus::SolidBrush bodyBrush(Gdiplus::Color(255, 0, 220, 180)); // Cyan iridescent
    g.FillEllipse(&bodyBrush, -m_width * 0.35f, -m_height * 0.35f, m_width * 0.65f, m_height * 0.7f);

    // Eye
    Gdiplus::SolidBrush eyeBlack(Gdiplus::Color(255, 20, 20, 20));
    g.FillEllipse(&eyeBlack, m_width * 0.12f, -m_height * 0.18f, 5.0f, 5.0f);
}

void Fish::DrawAngelfish(Gdiplus::Graphics& g) const {
    float tailWiggle = std::sin(m_tailPhase) * 4.0f;

    // Top & Bottom long triangular fins
    Gdiplus::SolidBrush finBrush(Gdiplus::Color(240, 200, 200, 210));
    Gdiplus::PointF topFin[3] = {
        Gdiplus::PointF(-m_width * 0.1f, -m_height * 0.2f),
        Gdiplus::PointF(m_width * 0.05f, -m_height * 0.5f),
        Gdiplus::PointF(-m_width * 0.3f, -m_height * 0.15f)
    };
    g.FillPolygon(&finBrush, topFin, 3);

    Gdiplus::PointF bottomFin[3] = {
        Gdiplus::PointF(-m_width * 0.1f, m_height * 0.2f),
        Gdiplus::PointF(m_width * 0.05f, m_height * 0.5f),
        Gdiplus::PointF(-m_width * 0.3f, m_height * 0.15f)
    };
    g.FillPolygon(&finBrush, bottomFin, 3);

    // Tail
    Gdiplus::PointF tailPts[3] = {
        Gdiplus::PointF(-m_width * 0.25f, 0.0f),
        Gdiplus::PointF(-m_width * 0.5f, -m_height * 0.25f + tailWiggle),
        Gdiplus::PointF(-m_width * 0.5f, m_height * 0.25f + tailWiggle)
    };
    g.FillPolygon(&finBrush, tailPts, 3);

    // Silver Body
    Gdiplus::SolidBrush bodyBrush(Gdiplus::Color(255, 220, 225, 235));
    Gdiplus::PointF bodyPts[4] = {
        Gdiplus::PointF(m_width * 0.35f, 0.0f),
        Gdiplus::PointF(-m_width * 0.1f, -m_height * 0.3f),
        Gdiplus::PointF(-m_width * 0.3f, 0.0f),
        Gdiplus::PointF(-m_width * 0.1f, m_height * 0.3f)
    };
    g.FillPolygon(&bodyBrush, bodyPts, 4);

    // Black stripes
    Gdiplus::Pen stripePen(Gdiplus::Color(255, 40, 40, 40), 3.0f);
    g.DrawLine(&stripePen, 0.0f, -m_height * 0.25f, 0.0f, m_height * 0.25f);
    g.DrawLine(&stripePen, -m_width * 0.15f, -m_height * 0.18f, -m_width * 0.15f, m_height * 0.18f);

    // Eye
    Gdiplus::SolidBrush eyeBlack(Gdiplus::Color(255, 0, 0, 0));
    g.FillEllipse(&eyeBlack, m_width * 0.15f, -m_height * 0.1f, 4.5f, 4.5f);
}

void Fish::DrawBetta(Gdiplus::Graphics& g) const {
    float tailWiggle = std::sin(m_tailPhase) * 9.0f;

    // Huge veil tail (Magenta / Deep Purple)
    Gdiplus::SolidBrush tailBrush(Gdiplus::Color(230, 180, 20, 80));
    Gdiplus::PointF tailPts[5] = {
        Gdiplus::PointF(-m_width * 0.25f, 0.0f),
        Gdiplus::PointF(-m_width * 0.6f, -m_height * 0.6f + tailWiggle),
        Gdiplus::PointF(-m_width * 0.5f, tailWiggle * 0.3f),
        Gdiplus::PointF(-m_width * 0.6f, m_height * 0.6f + tailWiggle),
        Gdiplus::PointF(-m_width * 0.35f, 0.0f)
    };
    g.FillPolygon(&tailBrush, tailPts, 5);

    // Body
    Gdiplus::SolidBrush bodyBrush(Gdiplus::Color(255, 120, 0, 60));
    g.FillEllipse(&bodyBrush, -m_width * 0.38f, -m_height * 0.38f, m_width * 0.72f, m_height * 0.76f);

    // Eye
    Gdiplus::SolidBrush eyeWhite(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::SolidBrush eyeBlack(Gdiplus::Color(255, 0, 0, 0));
    g.FillEllipse(&eyeWhite, m_width * 0.15f, -m_height * 0.18f, 5.0f, 5.0f);
    g.FillEllipse(&eyeBlack, m_width * 0.18f, -m_height * 0.14f, 2.5f, 2.5f);
}

void Fish::DrawBlueTang(Gdiplus::Graphics& g) const {
    float tailWiggle = std::sin(m_tailPhase) * 5.0f;

    // Yellow Tail
    Gdiplus::SolidBrush yellowTail(Gdiplus::Color(255, 255, 220, 0));
    Gdiplus::PointF tailPts[3] = {
        Gdiplus::PointF(-m_width * 0.25f, 0.0f),
        Gdiplus::PointF(-m_width * 0.5f, -m_height * 0.4f + tailWiggle),
        Gdiplus::PointF(-m_width * 0.5f, m_height * 0.4f + tailWiggle)
    };
    g.FillPolygon(&yellowTail, tailPts, 3);

    // Royal Blue Body
    Gdiplus::SolidBrush blueBody(Gdiplus::Color(255, 20, 60, 200));
    g.FillEllipse(&blueBody, -m_width * 0.4f, -m_height * 0.45f, m_width * 0.75f, m_height * 0.9f);

    // Black marking
    Gdiplus::SolidBrush blackMark(Gdiplus::Color(255, 10, 10, 30));
    g.FillEllipse(&blackMark, -m_width * 0.2f, -m_height * 0.3f, m_width * 0.4f, m_height * 0.5f);

    // Eye
    Gdiplus::SolidBrush eyeBlack(Gdiplus::Color(255, 0, 0, 0));
    g.FillEllipse(&eyeBlack, m_width * 0.18f, -m_height * 0.2f, 5.0f, 5.0f);
}

void Fish::DrawYellowLab(Gdiplus::Graphics& g) const {
    float tailWiggle = std::sin(m_tailPhase) * 5.0f;

    // Yellow Tail
    Gdiplus::SolidBrush yellowTail(Gdiplus::Color(255, 250, 210, 20));
    Gdiplus::PointF tailPts[3] = {
        Gdiplus::PointF(-m_width * 0.25f, 0.0f),
        Gdiplus::PointF(-m_width * 0.5f, -m_height * 0.35f + tailWiggle),
        Gdiplus::PointF(-m_width * 0.5f, m_height * 0.35f + tailWiggle)
    };
    g.FillPolygon(&yellowTail, tailPts, 3);

    // Yellow Body
    Gdiplus::SolidBrush yellowBody(Gdiplus::Color(255, 255, 225, 0));
    g.FillEllipse(&yellowBody, -m_width * 0.4f, -m_height * 0.42f, m_width * 0.75f, m_height * 0.84f);

    // Dark dorsal edge
    Gdiplus::Pen darkPen(Gdiplus::Color(255, 30, 30, 30), 2.5f);
    g.DrawArc(&darkPen, -m_width * 0.3f, -m_height * 0.42f, m_width * 0.6f, m_height * 0.5f, 190, 140);

    // Eye
    Gdiplus::SolidBrush eyeBlack(Gdiplus::Color(255, 0, 0, 0));
    g.FillEllipse(&eyeBlack, m_width * 0.18f, -m_height * 0.18f, 5.0f, 5.0f);
}

void Fish::DrawDiscus(Gdiplus::Graphics& g) const {
    float tailWiggle = std::sin(m_tailPhase) * 4.0f;

    // Tail
    Gdiplus::SolidBrush turquoiseTail(Gdiplus::Color(230, 0, 180, 200));
    Gdiplus::PointF tailPts[3] = {
        Gdiplus::PointF(-m_width * 0.25f, 0.0f),
        Gdiplus::PointF(-m_width * 0.48f, -m_height * 0.3f + tailWiggle),
        Gdiplus::PointF(-m_width * 0.48f, m_height * 0.3f + tailWiggle)
    };
    g.FillPolygon(&turquoiseTail, tailPts, 3);

    // Disc Body (Round Turquoise)
    Gdiplus::SolidBrush discBody(Gdiplus::Color(255, 40, 190, 210));
    g.FillEllipse(&discBody, -m_width * 0.4f, -m_height * 0.48f, m_width * 0.78f, m_height * 0.96f);

    // Decorative Wavy Lines
    Gdiplus::Pen wavePen(Gdiplus::Color(255, 230, 100, 140), 1.8f);
    g.DrawArc(&wavePen, -m_width * 0.25f, -m_height * 0.35f, m_width * 0.5f, m_height * 0.7f, 30, 120);
    g.DrawArc(&wavePen, -m_width * 0.15f, -m_height * 0.25f, m_width * 0.4f, m_height * 0.5f, 30, 120);

    // Eye (Red ring, black center)
    Gdiplus::SolidBrush eyeRed(Gdiplus::Color(255, 220, 40, 40));
    Gdiplus::SolidBrush eyeBlack(Gdiplus::Color(255, 0, 0, 0));
    g.FillEllipse(&eyeRed, m_width * 0.16f, -m_height * 0.18f, 6.0f, 6.0f);
    g.FillEllipse(&eyeBlack, m_width * 0.19f, -m_height * 0.14f, 3.0f, 3.0f);
}

void Fish::DrawCorydoras(Gdiplus::Graphics& g) const {
    float tailWiggle = std::sin(m_tailPhase) * 6.0f;

    // Tail
    Gdiplus::SolidBrush brownTail(Gdiplus::Color(240, 140, 100, 70));
    Gdiplus::PointF tailPts[3] = {
        Gdiplus::PointF(-m_width * 0.25f, 0.0f),
        Gdiplus::PointF(-m_width * 0.5f, -m_height * 0.35f + tailWiggle),
        Gdiplus::PointF(-m_width * 0.5f, m_height * 0.35f + tailWiggle)
    };
    g.FillPolygon(&brownTail, tailPts, 3);

    // Speckled Brown Body
    Gdiplus::SolidBrush bodyBrush(Gdiplus::Color(255, 170, 130, 90));
    g.FillEllipse(&bodyBrush, -m_width * 0.4f, -m_height * 0.4f, m_width * 0.75f, m_height * 0.8f);

    // Whiskers (Barbels) near mouth
    Gdiplus::Pen whiskerPen(Gdiplus::Color(255, 120, 80, 50), 1.5f);
    g.DrawLine(&whiskerPen, m_width * 0.3f, m_height * 0.1f, m_width * 0.42f, m_height * 0.22f);
    g.DrawLine(&whiskerPen, m_width * 0.3f, m_height * 0.15f, m_width * 0.45f, m_height * 0.1f);

    // Eye
    Gdiplus::SolidBrush eyeBlack(Gdiplus::Color(255, 20, 20, 20));
    g.FillEllipse(&eyeBlack, m_width * 0.15f, -m_height * 0.15f, 4.5f, 4.5f);
}

void Fish::DrawAxolotl(Gdiplus::Graphics& g) const {
    float tailWiggle = std::sin(m_tailPhase) * 5.0f;
    float gillSway = std::sin(m_tailPhase * 0.8f) * 2.5f;

    // 1. Translucent Soft Pink Ribbon Tail Fin
    Gdiplus::SolidBrush tailFinBrush(Gdiplus::Color(180, 255, 170, 200));
    Gdiplus::PointF tailPts[4] = {
        Gdiplus::PointF(-m_width * 0.15f, 0.0f),
        Gdiplus::PointF(-m_width * 0.52f, -m_height * 0.40f + tailWiggle),
        Gdiplus::PointF(-m_width * 0.58f, tailWiggle * 1.2f),
        Gdiplus::PointF(-m_width * 0.52f, m_height * 0.40f + tailWiggle)
    };
    g.FillPolygon(&tailFinBrush, tailPts, 4);

    // 2. Iconic External Gills (6 feathery frills sticking out from back of head)
    Gdiplus::Pen gillStemPen(Gdiplus::Color(255, 240, 70, 130), 2.8f);
    Gdiplus::Pen gillFeatherPen(Gdiplus::Color(220, 255, 100, 150), 1.6f);

    // Top Gills (Left/Right)
    g.DrawLine(&gillStemPen, -m_width * 0.05f, -m_height * 0.25f, -m_width * 0.22f, -m_height * 0.55f + gillSway);
    g.DrawLine(&gillFeatherPen, -m_width * 0.12f, -m_height * 0.35f, -m_width * 0.20f, -m_height * 0.42f + gillSway);
    g.DrawLine(&gillFeatherPen, -m_width * 0.17f, -m_height * 0.45f, -m_width * 0.26f, -m_height * 0.50f + gillSway);

    // Mid Gills
    g.DrawLine(&gillStemPen, -m_width * 0.08f, -m_height * 0.05f, -m_width * 0.28f, -m_height * 0.25f + gillSway);
    g.DrawLine(&gillFeatherPen, -m_width * 0.16f, -m_height * 0.12f, -m_width * 0.24f, -m_height * 0.18f + gillSway);

    // Lower Gills
    g.DrawLine(&gillStemPen, -m_width * 0.05f, m_height * 0.20f, -m_width * 0.22f, m_height * 0.48f - gillSway);

    // 3. Cute Tiny Feet / Paws under body
    Gdiplus::SolidBrush footBrush(Gdiplus::Color(255, 255, 180, 198));
    g.FillEllipse(&footBrush, -m_width * 0.15f, m_height * 0.30f, 6.0f, 7.0f);
    g.FillEllipse(&footBrush, m_width * 0.10f, m_height * 0.30f, 6.0f, 7.0f);

    // 4. Soft Rosy Pink Main Body
    Gdiplus::SolidBrush bodyBrush(Gdiplus::Color(255, 255, 192, 210));
    g.FillEllipse(&bodyBrush, -m_width * 0.38f, -m_height * 0.42f, m_width * 0.72f, m_height * 0.84f);

    // Belly Highlight
    Gdiplus::SolidBrush bellyBrush(Gdiplus::Color(140, 255, 225, 235));
    g.FillEllipse(&bellyBrush, -m_width * 0.30f, -m_height * 0.15f, m_width * 0.55f, m_height * 0.50f);

    // 5. Cute Face: Rosy Cheeks & Big Shiny Eyes
    Gdiplus::SolidBrush cheekBrush(Gdiplus::Color(160, 255, 140, 170));
    g.FillEllipse(&cheekBrush, m_width * 0.08f, -m_height * 0.08f, 6.0f, 6.0f);
    g.FillEllipse(&cheekBrush, m_width * 0.08f, m_height * 0.12f, 6.0f, 6.0f);

    // Eyes
    Gdiplus::SolidBrush eyeBlack(Gdiplus::Color(255, 30, 20, 35));
    Gdiplus::SolidBrush eyeShine(Gdiplus::Color(255, 255, 255, 255));
    g.FillEllipse(&eyeBlack, m_width * 0.18f, -m_height * 0.22f, 4.5f, 4.5f);
    g.FillEllipse(&eyeShine, m_width * 0.20f, -m_height * 0.20f, 1.8f, 1.8f);

    // Cute Smile Arc
    Gdiplus::Pen smilePen(Gdiplus::Color(200, 200, 100, 130), 1.4f);
    g.DrawArc(&smilePen, m_width * 0.20f, -m_height * 0.08f, 6.0f, 6.0f, 20, 140);
}
