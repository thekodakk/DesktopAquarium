#include "Renderer.h"
#include "AssetManager.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper to construct smooth rounded rectangle paths
static void AddRoundRect(Gdiplus::GraphicsPath& path, float x, float y, float w, float h, float radius) {
    float diameter = radius * 2.0f;
    if (diameter > w) diameter = w;
    if (diameter > h) diameter = h;

    path.AddArc(x, y, diameter, diameter, 180, 90);
    path.AddArc(x + w - diameter, y, diameter, diameter, 270, 90);
    path.AddArc(x + w - diameter, y + h - diameter, diameter, diameter, 0, 90);
    path.AddArc(x, y + h - diameter, diameter, diameter, 90, 90);
    path.CloseFigure();
}

void Renderer::DrawAquarium(Gdiplus::Graphics& g, const Aquarium& aquarium) {
    // Ultra High Quality Anti-Aliasing & Rendering Settings
    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

    int width = aquarium.GetWidth();
    int height = aquarium.GetHeight();

    // 1. Render Minecraft Shader Style Water Body (sloshing surface, caustics & light rays)
    DrawWaterBody(g, aquarium);

    // 2. Render Bottom Gravel & Animated Swaying Seaweed
    DrawGravelAndPlants(g, width, height, aquarium.GetAnimTime());

    // 3. Render Floating Bubbles
    DrawBubbles(g, aquarium);

    // 4. Render Active Food Pellets
    DrawFoodPellets(g, aquarium);

    // 5. Render Swimming Fish & Axolotl
    aquarium.Draw(g);

    // 6. Render Splash Droplets (if spilling)
    DrawSplashParticles(g, aquarium);

    // 7. Render Tank Frame & Glass Reflection Glare
    DrawTankFrame(g, width, height);

    // 8. Render Modern Glassmorphism HUD Bar & Feed Button
    DrawHungerHeader(g, aquarium);
}

void Renderer::DrawWaterBody(Gdiplus::Graphics& g, const Aquarium& aquarium) {
    int width = aquarium.GetWidth();
    int height = aquarium.GetHeight();
    float waterRatio = aquarium.GetWaterLevelRatio();
    float animTime = aquarium.GetAnimTime();

    float gravelTopY = height - 35.0f;
    float waterHeight = (height - 85.0f) * waterRatio;
    float waterSurfaceY = gravelTopY - waterHeight;

    // Build Water Polygon with multi-frequency Minecraft shader style wave top surface
    Gdiplus::GraphicsPath waterPath;

    float leftY = waterSurfaceY + std::tan(aquarium.GetWaterLevelRatio()) * 15.0f;
    float rightY = waterSurfaceY - std::tan(aquarium.GetWaterLevelRatio()) * 15.0f;

    waterPath.AddLine(10.0f, gravelTopY, 10.0f, leftY);

    // Add organic fluid surface wave points
    int steps = 25;
    for (int i = 0; i <= steps; ++i) {
        float t = (float)i / steps;
        float x = 10.0f + t * (width - 20.0f);
        float waveOffset = std::sin(t * M_PI * 4.0f + animTime * 2.8f) * 2.5f +
                           std::sin(t * M_PI * 8.0f - animTime * 1.5f) * 1.2f;
        float y = leftY + t * (rightY - leftY) + waveOffset;
        waterPath.AddLine(x, y, x, y);
    }

    waterPath.AddLine((float)width - 10.0f, gravelTopY, (float)width - 10.0f, gravelTopY);
    waterPath.CloseFigure();

    // High Resolution Underwater Background Image Fill
    Gdiplus::Bitmap* bgBmp = AssetManager::GetBackground();
    if (bgBmp) {
        Gdiplus::GraphicsState bgState = g.Save();
        g.SetClip(&waterPath);
        g.DrawImage(bgBmp, 10.0f, 10.0f, (float)width - 20.0f, (float)height - 20.0f);
        g.Restore(bgState);
    } else {
        Gdiplus::LinearGradientBrush waterBrush(
            Gdiplus::PointF(0.0f, waterSurfaceY),
            Gdiplus::PointF(0.0f, gravelTopY),
            Gdiplus::Color(170, 15, 185, 245),
            Gdiplus::Color(220, 5, 35, 95)
        );
        g.FillPath(&waterBrush, &waterPath);
    }

    // Minecraft Shader Style Animated Caustic Light Ripples & Underwater God Rays
    Gdiplus::GraphicsState state = g.Save();
    g.SetClip(&waterPath);

    // Ambient God Rays
    float rayAlpha1 = 35.0f + 15.0f * std::sin(animTime * 1.5f);
    float rayAlpha2 = 30.0f + 12.0f * std::cos(animTime * 1.8f + 1.0f);
    float rayAlpha3 = 40.0f + 10.0f * std::sin(animTime * 1.2f + 2.0f);

    Gdiplus::SolidBrush rayBrush1(Gdiplus::Color((BYTE)rayAlpha1, 200, 245, 255));
    Gdiplus::SolidBrush rayBrush2(Gdiplus::Color((BYTE)rayAlpha2, 180, 240, 255));
    Gdiplus::SolidBrush rayBrush3(Gdiplus::Color((BYTE)rayAlpha3, 220, 250, 255));

    Gdiplus::PointF ray1[4] = {
        Gdiplus::PointF(width * 0.12f, waterSurfaceY),
        Gdiplus::PointF(width * 0.32f, waterSurfaceY),
        Gdiplus::PointF(width * 0.52f, gravelTopY),
        Gdiplus::PointF(width * 0.25f, gravelTopY)
    };
    g.FillPolygon(&rayBrush1, ray1, 4);

    Gdiplus::PointF ray2[4] = {
        Gdiplus::PointF(width * 0.42f, waterSurfaceY),
        Gdiplus::PointF(width * 0.62f, waterSurfaceY),
        Gdiplus::PointF(width * 0.82f, gravelTopY),
        Gdiplus::PointF(width * 0.58f, gravelTopY)
    };
    g.FillPolygon(&rayBrush2, ray2, 4);

    Gdiplus::PointF ray3[4] = {
        Gdiplus::PointF(width * 0.72f, waterSurfaceY),
        Gdiplus::PointF(width * 0.88f, waterSurfaceY),
        Gdiplus::PointF(width * 0.98f, gravelTopY),
        Gdiplus::PointF(width * 0.80f, gravelTopY)
    };
    g.FillPolygon(&rayBrush3, ray3, 4);

    // Animated Caustic Light Spots Floating in Water
    Gdiplus::SolidBrush causticBrush(Gdiplus::Color(25, 220, 250, 255));
    for (int i = 0; i < 6; ++i) {
        float cx = 40.0f + fmod((float)(i * 85 + animTime * 15.0f), (float)width - 80.0f);
        float cy = waterSurfaceY + 25.0f + fmod((float)(i * 45 + animTime * 20.0f), waterHeight - 40.0f);
        float cw = 35.0f + 10.0f * std::sin(animTime * 2.0f + i);
        float ch = 18.0f + 5.0f * std::cos(animTime * 2.0f + i);
        g.FillEllipse(&causticBrush, cx, cy, cw, ch);
    }

    g.Restore(state);

    // Water Surface Shimmer & Bloom Line
    Gdiplus::Pen surfacePen(Gdiplus::Color(230, 235, 255, 255), 2.8f);
    g.DrawLine(&surfacePen, 10.0f, leftY, (float)width - 10.0f, rightY);
}

void Renderer::DrawGravelAndPlants(Gdiplus::Graphics& g, int width, int height, float animTime) {
    float gravelTopY = height - 40.0f;

    // Gravel bed base
    Gdiplus::SolidBrush gravelBase(Gdiplus::Color(230, 95, 75, 60)); // Natural warm pebble brown
    g.FillRectangle(&gravelBase, 10.0f, gravelTopY, (float)width - 20.0f, 30.0f);

    // Individual Pebble Details
    Gdiplus::SolidBrush pebble1(Gdiplus::Color(240, 150, 120, 85));
    Gdiplus::SolidBrush pebble2(Gdiplus::Color(240, 190, 160, 120));
    Gdiplus::SolidBrush pebble3(Gdiplus::Color(240, 95, 130, 150));

    for (int x = 15; x < width - 20; x += 12) {
        g.FillEllipse(&pebble1, (float)x, gravelTopY + 2.0f, 10.0f, 7.0f);
        g.FillEllipse(&pebble2, (float)x + 5.0f, gravelTopY + 8.0f, 11.0f, 8.0f);
        g.FillEllipse(&pebble3, (float)x - 3.0f, gravelTopY + 16.0f, 9.0f, 6.0f);
    }

    // PNG Plant Sprite Rendering with Animated Sway
    Gdiplus::Bitmap* plantBmp = AssetManager::GetPlant();
    if (plantBmp) {
        // Left Plant Cluster
        Gdiplus::GraphicsState pState1 = g.Save();
        g.TranslateTransform(50.0f, gravelTopY);
        g.RotateTransform(std::sin(animTime * 2.0f) * 6.0f);
        g.DrawImage(plantBmp, -25.0f, -110.0f, 50.0f, 110.0f);
        g.Restore(pState1);

        // Right Plant Cluster
        Gdiplus::GraphicsState pState2 = g.Save();
        g.TranslateTransform((float)width - 55.0f, gravelTopY);
        g.RotateTransform(std::sin(animTime * 2.0f + 1.2f) * 7.0f);
        g.DrawImage(plantBmp, -25.0f, -115.0f, 50.0f, 115.0f);
        g.Restore(pState2);
    } else {
        Gdiplus::Pen plantPen1(Gdiplus::Color(240, 25, 185, 75), 4.5f);
        float sway1 = std::sin(animTime * 2.2f) * 14.0f;
        g.DrawBezier(&plantPen1, 40.0f, gravelTopY, 35.0f + sway1 * 0.3f, gravelTopY - 40.0f, 55.0f + sway1 * 0.7f, gravelTopY - 80.0f, 45.0f + sway1, gravelTopY - 110.0f);
    }
}

void Renderer::DrawBubbles(Gdiplus::Graphics& g, const Aquarium& aquarium) {
    const auto& bubbles = aquarium.GetAirBubbles();
    Gdiplus::Bitmap* bubbleBmp = AssetManager::GetBubble();

    for (const auto& b : bubbles) {
        float r = b.radius;
        if (bubbleBmp) {
            g.DrawImage(bubbleBmp, b.x - r, b.y - r, r * 2.2f, r * 2.2f);
        } else {
            Gdiplus::SolidBrush bubbleFill(Gdiplus::Color(100, 180, 235, 255));
            Gdiplus::Pen bubbleBorder(Gdiplus::Color(180, 220, 255, 255), 1.2f);
            g.FillEllipse(&bubbleFill, b.x - r, b.y - r, r * 2.0f, r * 2.0f);
            g.DrawEllipse(&bubbleBorder, b.x - r, b.y - r, r * 2.0f, r * 2.0f);
        }
    }
}

void Renderer::DrawFoodPellets(Gdiplus::Graphics& g, const Aquarium& aquarium) {
    const auto& pellets = aquarium.GetFoodPellets();

    Gdiplus::SolidBrush auraBrush(Gdiplus::Color(120, 255, 210, 60));
    Gdiplus::SolidBrush coreBrush(Gdiplus::Color(255, 235, 140, 30));
    Gdiplus::SolidBrush spotBrush(Gdiplus::Color(255, 255, 240, 180));

    for (const auto& p : pellets) {
        if (!p.active) continue;

        // Outer glow aura
        g.FillEllipse(&auraBrush, p.x - p.size * 0.9f, p.y - p.size * 0.9f, p.size * 2.6f, p.size * 2.6f);

        // Core pellet
        g.FillEllipse(&coreBrush, p.x - p.size * 0.5f, p.y - p.size * 0.5f, p.size, p.size);

        // Highlight spot
        float spotR = std::max(1.0f, p.size * 0.35f);
        g.FillEllipse(&spotBrush, p.x - p.size * 0.25f, p.y - p.size * 0.25f, spotR, spotR);
    }
}

void Renderer::DrawSplashParticles(Gdiplus::Graphics& g, const Aquarium& aquarium) {
    const auto& splashes = aquarium.GetSplashParticles();

    for (const auto& p : splashes) {
        if (!p.active) continue;

        int alphaInt = std::max(0, std::min(255, (int)(p.alpha * 255.0f)));
        Gdiplus::SolidBrush splashBrush(Gdiplus::Color(alphaInt, 200, 235, 255));
        g.FillEllipse(&splashBrush, p.x - p.size * 0.5f, p.y - p.size * 0.5f, p.size, p.size);
    }
}

void Renderer::DrawTankFrame(Gdiplus::Graphics& g, int width, int height) {
    // Glass Container Frame
    Gdiplus::RectF tankRect(4.0f, 4.0f, (float)width - 8.0f, (float)height - 8.0f);
    float cornerRadius = 16.0f;

    Gdiplus::GraphicsPath path;
    path.AddArc(tankRect.X, tankRect.Y, cornerRadius * 2, cornerRadius * 2, 180, 90);
    path.AddArc(tankRect.GetRight() - cornerRadius * 2, tankRect.Y, cornerRadius * 2, cornerRadius * 2, 270, 90);
    path.AddArc(tankRect.GetRight() - cornerRadius * 2, tankRect.GetBottom() - cornerRadius * 2, cornerRadius * 2, cornerRadius * 2, 0, 90);
    path.AddArc(tankRect.X, tankRect.GetBottom() - cornerRadius * 2, cornerRadius * 2, cornerRadius * 2, 90, 90);
    path.CloseFigure();

    // Subtle Glass Frame Outline
    Gdiplus::Pen glassBorderPen(Gdiplus::Color(180, 200, 230, 255), 4.0f);
    g.DrawPath(&glassBorderPen, &path);

    // Glass Reflection Glare (Diagonal Light Stripe)
    Gdiplus::GraphicsState state = g.Save();
    g.SetClip(&path);

    Gdiplus::SolidBrush glareBrush(Gdiplus::Color(35, 255, 255, 255));
    Gdiplus::PointF glarePts[4] = {
        Gdiplus::PointF(20.0f, 0.0f),
        Gdiplus::PointF(140.0f, 0.0f),
        Gdiplus::PointF(40.0f, (float)height),
        Gdiplus::PointF(-80.0f, (float)height)
    };
    g.FillPolygon(&glareBrush, glarePts, 4);

    g.Restore(state);
}

void Renderer::DrawHungerHeader(Gdiplus::Graphics& g, const Aquarium& aquarium) {
    int width = aquarium.GetWidth();
    int height = aquarium.GetHeight();

    // Modern Glassmorphism Card Container
    float cardX = 14.0f;
    float cardY = 10.0f;
    float cardW = (float)width - 28.0f;
    float cardH = 38.0f;
    float cardRadius = 12.0f;

    Gdiplus::GraphicsPath cardPath;
    AddRoundRect(cardPath, cardX, cardY, cardW, cardH, cardRadius);

    // Glass Fill & Soft Glow Outline
    Gdiplus::SolidBrush cardBg(Gdiplus::Color(190, 16, 24, 40));
    g.FillPath(&cardBg, &cardPath);

    Gdiplus::Pen cardBorder(Gdiplus::Color(160, 110, 150, 210), 1.5f);
    g.DrawPath(&cardBorder, &cardPath);

    // Label: "Acıkma Barı:"
    Gdiplus::Font titleFont(L"Segoe UI", 9.0f, Gdiplus::FontStyleBold);
    Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 240, 245, 255));
    g.DrawString(L"Acıkma Barı:", -1, &titleFont, Gdiplus::PointF(22.0f, 20.0f), &textBrush);

    // Modern Rounded Pill Hunger Bar
    float barX = 115.0f;
    float barY = 22.0f;
    float barW = (float)width - 275.0f;
    float barH = 14.0f;
    float pillRadius = 7.0f;

    Gdiplus::GraphicsPath barFramePath;
    AddRoundRect(barFramePath, barX, barY, barW, barH, pillRadius);

    Gdiplus::SolidBrush barBg(Gdiplus::Color(240, 25, 30, 45));
    g.FillPath(&barBg, &barFramePath);

    float hungerRatio = aquarium.GetHungerProgress();
    float progressW = std::max(pillRadius * 2.0f, barW * hungerRatio);

    if (hungerRatio > 0.02f) {
        Gdiplus::GraphicsPath progressPath;
        AddRoundRect(progressPath, barX, barY, progressW, barH, pillRadius);

        Gdiplus::Color progressColor;
        if (hungerRatio < 0.40f) {
            progressColor = Gdiplus::Color(255, 40, 215, 90);  // Glowing Emerald Green (Full)
        } else if (hungerRatio < 0.75f) {
            progressColor = Gdiplus::Color(255, 245, 175, 20); // Golden Amber (Hungry)
        } else {
            progressColor = Gdiplus::Color(255, 245, 55, 55);   // Coral Crimson Red (Starving)
        }

        Gdiplus::SolidBrush progressBrush(progressColor);
        g.FillPath(&progressBrush, &progressPath);
    }

    Gdiplus::Pen barOutline(Gdiplus::Color(180, 100, 130, 160), 1.0f);
    g.DrawPath(&barOutline, &barFramePath);

    // Modern Pill "BALIKLARI BESLE" Feed Button
    float btnX = (float)width - 145.0f;
    float btnY = 15.0f;
    float btnW = 120.0f;
    float btnH = 28.0f;
    float btnRadius = 14.0f;

    Gdiplus::GraphicsPath btnPath;
    AddRoundRect(btnPath, btnX, btnY, btnW, btnH, btnRadius);

    bool canFeed = aquarium.CanFeed();

    if (canFeed) {
        // Active Enabled Button: Vibrant Cyan-Blue Linear Gradient Fill
        Gdiplus::LinearGradientBrush btnBrush(
            Gdiplus::PointF(btnX, btnY),
            Gdiplus::PointF(btnX, btnY + btnH),
            Gdiplus::Color(255, 20, 165, 235), // Glowing top cyan
            Gdiplus::Color(255, 10, 105, 195)  // Deep azure bottom
        );
        g.FillPath(&btnBrush, &btnPath);

        Gdiplus::Pen btnPen(Gdiplus::Color(255, 255, 255, 255), 1.5f);
        g.DrawPath(&btnPen, &btnPath);

        Gdiplus::Font btnFont(L"Segoe UI", 8.8f, Gdiplus::FontStyleBold);
        Gdiplus::SolidBrush btnTextBrush(Gdiplus::Color(255, 255, 255, 255));
        g.DrawString(L"BALIKLARI BESLE", -1, &btnFont, Gdiplus::PointF(btnX + 6.0f, btnY + 6.0f), &btnTextBrush);
    } else {
        // Disabled / Full / Cooldown Button: Muted Semi-Transparent Glass
        Gdiplus::SolidBrush btnDisabledBrush(Gdiplus::Color(160, 45, 55, 70));
        g.FillPath(&btnDisabledBrush, &btnPath);

        Gdiplus::Pen btnDisabledPen(Gdiplus::Color(110, 120, 135, 150), 1.2f);
        g.DrawPath(&btnDisabledPen, &btnPath);

        Gdiplus::Font btnFont(L"Segoe UI", 8.8f, Gdiplus::FontStyleBold);
        Gdiplus::SolidBrush btnDisabledText(Gdiplus::Color(180, 180, 195, 210));
        std::wstring statusText = (aquarium.GetHungerProgress() < 0.15f) ? L"BALIKLAR TOK" : L"BEKLEYİN...";
        g.DrawString(statusText.c_str(), -1, &btnFont, Gdiplus::PointF(btnX + (aquarium.GetHungerProgress() < 0.15f ? 16.0f : 24.0f), btnY + 6.0f), &btnDisabledText);
    }

    // Water level spill indicator message
    if (aquarium.GetWaterLevelRatio() < 0.98f) {
        Gdiplus::Font warningFont(L"Segoe UI", 8.5f, Gdiplus::FontStyleItalic);
        Gdiplus::SolidBrush warningBrush(Gdiplus::Color(255, 255, 210, 60));
        g.DrawString(L"Su Döküldü! Geri Doluyor...", -1, &warningFont, Gdiplus::PointF(20.0f, (float)width > 400 ? (float)height - 25.0f : (float)height - 35.0f), &warningBrush);
    }
}
