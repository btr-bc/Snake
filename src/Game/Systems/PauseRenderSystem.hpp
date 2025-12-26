#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include "Core/System.hpp"
#include "Core/Component.hpp"
#include "Core/Context.hpp"

namespace Bocchi {
    class PauseRenderSystem : public System {
    public:
        void update(entt::registry& reg) override {
            auto& ctx = reg.ctx().get<GameContext>();
            auto* window = ctx.window.window;
            if (!window || !ctx.window.uiView) return;

            updateAnimation(ctx.state.isPaused, ctx.time.dt);
            if (m_animFactor <= 0.0f) return;

            window->setView(*ctx.window.uiView);
            sf::Vector2f sz = ctx.window.windowSize;

            sf::RectangleShape dim(sz);
            dim.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(70 * m_animFactor)));
            window->draw(dim);

            float thickness = std::min(sz.x, sz.y) / 20.0f;
            float pulse = (ctx.state.isPaused && m_animFactor > 0.9f) ? (std::sin(m_timer * 2.5f) * 0.5f + 0.5f) : 0.0f;
            sf::Uint8 alpha = static_cast<sf::Uint8>((100 + 40 * pulse) * m_animFactor);
            
            sf::Color edgeColor(180, 0, 0, alpha); 
            sf::Color innerColor(180, 0, 0, 0);

            sf::VertexArray va(sf::Quads, 16);
            drawEdge(va, 0, {0,0}, {thickness, thickness}, {0,sz.y}, {thickness, sz.y-thickness}, edgeColor, innerColor);
            drawEdge(va, 4, {sz.x,0}, {sz.x-thickness, thickness}, {sz.x,sz.y}, {sz.x-thickness, sz.y-thickness}, edgeColor, innerColor);
            drawEdge(va, 8, {0,0}, {thickness, thickness}, {sz.x,0}, {sz.x-thickness, thickness}, edgeColor, innerColor);
            drawEdge(va, 12, {0,sz.y}, {thickness, sz.y-thickness}, {sz.x,sz.y}, {sz.x-thickness, sz.y-thickness}, edgeColor, innerColor);

            window->draw(va);
        }

    private:
        float m_animFactor = 0.0f;
        float m_timer = 0.0f;

        void updateAnimation(bool isPaused, float dt) {
            float fadeSpeed = 5.0f;
            m_timer += dt;

            if (isPaused) {
                m_animFactor = std::min(1.0f, m_animFactor + fadeSpeed * dt);
            } else {
                m_animFactor = std::max(0.0f, m_animFactor - fadeSpeed * dt);
            }
        }

        void drawEdge(sf::VertexArray& va, int i, sf::Vector2f o1, sf::Vector2f i1, sf::Vector2f o2, sf::Vector2f i2, sf::Color oc, sf::Color ic) {
            va[i+0] = sf::Vertex(o1, oc); va[i+1] = sf::Vertex(i1, ic);
            va[i+2] = sf::Vertex(i2, ic); va[i+3] = sf::Vertex(o2, oc);
        }
    };
}
