#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <cmath>
#include "Core/System.hpp"
#include "Core/Context.hpp"
#include "Core/Config.h"

namespace Bocchi {

class ClassicBackgroundRenderSystem : public System {
public:
    ClassicBackgroundRenderSystem(float worldW = 8000.f, float worldH = 8000.f)
        : m_worldSize(worldW, worldH)
    {
        auto& config = Config::getInstance();
        m_worldSize = {config.mapWidth, config.mapHeight};
        m_tileSize = 2048.f;
        m_gridSpacing = 100.f;

        m_bgGradient.setPrimitiveType(sf::Quads);
        m_bgGradient.resize(4);

        initStarLayer(m_dustStars, 4000, {200, 200, 255, 70});
        initStarLayer(m_midStars, 1500, {255, 255, 255, 160}, true);
        initBigStars(500);

        m_border.setSize(m_worldSize);
        m_border.setPosition(0.f, 0.f);
        m_border.setFillColor(sf::Color::Transparent);
        m_border.setOutlineThickness(4.f);

        m_gridLines.setPrimitiveType(sf::Lines);
    }

    void update(entt::registry& reg) override {
        auto& ctx = reg.ctx().get<GameContext>();
        auto* window = ctx.window.window;
        if (!window) return;

        m_timer += ctx.time.dt;
        sf::Vector2f winSize = static_cast<sf::Vector2f>(window->getSize());
        sf::Vector2f camPos = ctx.window.cameraPos;

        sf::View originalView = window->getView();

        if (ctx.window.uiView) window->setView(*ctx.window.uiView);
        else window->setView(window->getDefaultView());

        updateBackgroundGradient(winSize);
        window->draw(m_bgGradient, sf::RenderStates::Default);

        if (ctx.window.worldView) window->setView(*ctx.window.worldView);

        drawInfiniteStarLayer(*window, m_dustStars, camPos, 0.02f);
        drawInfiniteStarLayer(*window, m_midStars,  camPos, 0.10f);
        drawInfiniteBigStars(*window, camPos, 0.25f);

        drawInfiniteGrid(*window, camPos, winSize);

        updateBorderEffect(ctx.state.isPaused);
        window->draw(m_border, sf::RenderStates::Default);

        window->setView(originalView);
    }

private:
    struct Star {
        sf::Vector2f pos;
        sf::Color color;
        float size = 1.f;
        float phase = 0.f;
    };

    sf::VertexArray m_bgGradient, m_gridLines;
    std::vector<Star> m_dustStars, m_midStars, m_bigStars;
    sf::RectangleShape m_border;
    sf::Vector2f m_worldSize;
    float m_tileSize, m_gridSpacing, m_timer = 0.f;
    const float M_PI_F = 3.14159265f;

    void initStarLayer(std::vector<Star>& layer, int count, sf::Color base, bool colorful = false) {
        std::mt19937 rng(12345);
        std::uniform_real_distribution<float> posDist(0.f, m_tileSize);
        std::uniform_real_distribution<float> phaseDist(0.f, 10.f);
        std::uniform_real_distribution<float> alphaMult(0.7f, 1.0f);

        std::vector<sf::Color> palette = {
            sf::Color(200, 220, 255), sf::Color(255, 255, 255),
            sf::Color(255, 240, 200), sf::Color(220, 200, 255)
        };

        layer.clear();
        layer.reserve(count);
        for (int i = 0; i < count; ++i) {
            Star s;
            s.pos = {posDist(rng), posDist(rng)};
            s.phase = phaseDist(rng);
            s.color = colorful ? palette[i % palette.size()] : base;
            s.color.a = static_cast<sf::Uint8>(s.color.a * alphaMult(rng));
            layer.push_back(s);
        }
    }

    void initBigStars(int count) {
        std::mt19937 rng(12345);
        std::uniform_real_distribution<float> sizeDist(2.0f, 5.0f);
        initStarLayer(m_bigStars, count, {255, 255, 255, 255}, true);
        for (auto& s : m_bigStars) s.size = sizeDist(rng);
    }

    void updateBackgroundGradient(sf::Vector2f size) {
        sf::Color deepSpace(5, 5, 15);
        sf::Color nebulaColor(20, 15, 30);
        m_bgGradient[0] = { {0.f, 0.f}, deepSpace };
        m_bgGradient[1] = { {size.x, 0.f}, deepSpace };
        m_bgGradient[2] = { {size.x, size.y}, nebulaColor };
        m_bgGradient[3] = { {0.f, size.y}, nebulaColor };
    }

    void drawInfiniteStarLayer(sf::RenderWindow& window, const std::vector<Star>& layer,
                                    sf::Vector2f camPos, float parallax) {
            sf::VertexArray va(sf::Points);
            sf::View currentView = window.getView();
            sf::Vector2f viewSize = currentView.getSize();
            sf::Vector2f viewCenter = currentView.getCenter();
            
            sf::Vector2f parallaxShift = camPos * (1.0f - parallax); 
            sf::Vector2f effectiveCenter = viewCenter - parallaxShift;

            int minTileX = static_cast<int>(std::floor((effectiveCenter.x - viewSize.x / 2.f) / m_tileSize));
            int maxTileX = static_cast<int>(std::ceil((effectiveCenter.x + viewSize.x / 2.f) / m_tileSize));
            int minTileY = static_cast<int>(std::floor((effectiveCenter.y - viewSize.y / 2.f) / m_tileSize));
            int maxTileY = static_cast<int>(std::ceil((effectiveCenter.y + viewSize.y / 2.f) / m_tileSize));

            for (int tx = minTileX; tx <= maxTileX; ++tx) {
                for (int ty = minTileY; ty <= maxTileY; ++ty) {
                    sf::Vector2f tileBase(tx * m_tileSize, ty * m_tileSize);
                    for (const auto& s : layer) {
                        float flash = std::sin(m_timer * 1.5f + s.phase) * 0.3f + 0.7f;
                        sf::Color c = s.color;
                        c.a = static_cast<sf::Uint8>(c.a * flash);
                        va.append(sf::Vertex(s.pos + tileBase + parallaxShift, c));
                    }
                }
            }
            window.draw(va, sf::RenderStates::Default);
    }

    void drawInfiniteBigStars(sf::RenderWindow& window, sf::Vector2f camPos, float parallax) {
        const int segments = 16;
        sf::View currentView = window.getView();
        sf::Vector2f viewSize = currentView.getSize();
        sf::Vector2f viewCenter = currentView.getCenter();

        sf::Vector2f parallaxShift = camPos * (1.0f - parallax);
        sf::Vector2f effectiveCenter = viewCenter - parallaxShift;

        int minTileX = static_cast<int>(std::floor((effectiveCenter.x - viewSize.x / 2.f) / m_tileSize));
        int maxTileX = static_cast<int>(std::ceil((effectiveCenter.x + viewSize.x / 2.f) / m_tileSize));
        int minTileY = static_cast<int>(std::floor((effectiveCenter.y - viewSize.y / 2.f) / m_tileSize));
        int maxTileY = static_cast<int>(std::ceil((effectiveCenter.y + viewSize.y / 2.f) / m_tileSize));

        for (int tx = minTileX; tx <= maxTileX; ++tx) {
            for (int ty = minTileY; ty <= maxTileY; ++ty) {
                sf::Vector2f tileBase(tx * m_tileSize, ty * m_tileSize);
                for (const auto& s : m_bigStars) {
                    sf::Vector2f pos = s.pos + tileBase + parallaxShift;
                    float radius = s.size;
                    float flash = std::sin(m_timer * 2.0f + s.phase) * 0.4f + 0.6f;
                    
                    sf::Color coreColor = s.color;
                    coreColor.a = static_cast<sf::Uint8>(255 * flash);
                    sf::Color edgeColor = s.color;
                    edgeColor.a = 0;

                    sf::VertexArray va(sf::TriangleFan);
                    va.append(sf::Vertex(pos, coreColor));
                    for (int i = 0; i <= segments; ++i) {
                        float angle = i * 2.f * M_PI_F / segments;
                        va.append(sf::Vertex({pos.x + std::cos(angle) * radius, pos.y + std::sin(angle) * radius}, edgeColor));
                    }
                    window.draw(va, sf::RenderStates::Default);
                }
            }
        }
    }

        void drawInfiniteGrid(sf::RenderWindow& window, sf::Vector2f camPos, sf::Vector2f winSize) {
        m_gridLines.clear();
        sf::Color c(50, 50, 60, 100);

        sf::View view = window.getView();
        sf::Vector2f vSize = view.getSize();
        sf::Vector2f vCenter = view.getCenter();

        float L = vCenter.x - vSize.x / 2.f - m_gridSpacing;
        float R = vCenter.x + vSize.x / 2.f + m_gridSpacing;
        float T = vCenter.y - vSize.y / 2.f - m_gridSpacing;
        float B = vCenter.y + vSize.y / 2.f + m_gridSpacing;

        float startX = std::floor(L / m_gridSpacing) * m_gridSpacing;
        float startY = std::floor(T / m_gridSpacing) * m_gridSpacing;

        for (float x = startX; x <= R; x += m_gridSpacing) {
            m_gridLines.append({ {x, T}, c });
            m_gridLines.append({ {x, B}, c });
        }
        for (float y = startY; y <= B; y += m_gridSpacing) {
            m_gridLines.append({ {L, y}, c });
            m_gridLines.append({ {R, y}, c });
        }
        window.draw(m_gridLines, sf::RenderStates::Default);
    }

    void updateBorderEffect(bool isPaused) {
        float glow = std::sin(m_timer * 3.0f) * 0.5f + 0.5f;
        sf::Uint8 alpha = static_cast<sf::Uint8>(100 + 155 * glow);
        if (isPaused) alpha = 255;
        m_border.setOutlineColor(sf::Color(255, 0, 0, alpha));
    }
};

}
