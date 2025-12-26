#pragma once
#include <cmath>
#include <entt/entt.hpp>
#include "Core/System.hpp"
#include "Core/Component.hpp"
#include "Core/Context.hpp"
#include <SFML/Graphics.hpp>

namespace Bocchi {

    class SnakeRenderSystem : public System {

        sf::CircleShape m_shield;

    public:
        void update(entt::registry& reg) override {
            auto& ctx = reg.ctx().get<GameContext>();
            auto* window = ctx.window.window;
            if (!window) return;

            if (ctx.window.worldView) window->setView(*ctx.window.worldView);

            sf::FloatRect viewBounds;
            const bool hasView = ctx.window.worldView != nullptr;
            if (hasView) {
                sf::Vector2f center = ctx.window.worldView->getCenter();
                sf::Vector2f size = ctx.window.worldView->getSize();
                viewBounds.left = center.x - size.x / 2.f - 50.f;
                viewBounds.top = center.y - size.y / 2.f - 50.f;
                viewBounds.width = size.x + 100.f;
                viewBounds.height = size.y + 100.f;
            }

            auto bodyView = reg.view<SnakeBody, Position>();
            for (auto entity : bodyView) {
                const auto& pos = bodyView.get<Position>(entity);
                if (hasView && !viewBounds.contains(pos.val)) continue;

                const auto& body = bodyView.get<SnakeBody>(entity);
                if (reg.valid(body.headOwner)) {
                    const auto& headData = reg.get<SnakeHead>(body.headOwner);
                    drawInternal(*window, ctx, headData.bodyID, headData.color, pos.val, 0.0f, headData.currentRadius, false);
                }
            }

            auto headView = reg.view<SnakeHead, Position, Rotation>();
            for (auto entity : headView) {
                const auto& pos = headView.get<Position>(entity);
                if (hasView && !viewBounds.contains(pos.val)) continue;

                const auto& head = headView.get<SnakeHead>(entity);
                const auto& rot = headView.get<Rotation>(entity);

                if (head.headID != ResID::head_shantianliang) drawInternal(*window, ctx, head.headID, head.color, pos.val, rot.angle + 90.f, head.currentRadius, true);
                else drawInternal(*window, ctx, head.headID, head.color, pos.val, rot.angle, head.currentRadius, true);

                if (head.spawnProtectionTime > 0) {
                    float shieldRadius = head.currentRadius * head.currentRadius / 2.f;
                    m_shield.setRadius(shieldRadius);
                    m_shield.setOrigin(shieldRadius, shieldRadius);
                    m_shield.setPosition(pos.val);

                    float pulse = (std::sin(ctx.time.frameCount * 6.0f) + 1.0f) * 0.5f;
                    
                    sf::Uint8 alphaEdge = static_cast<sf::Uint8>(150 + 105 * pulse);
                    sf::Uint8 alphaFill = static_cast<sf::Uint8>(30 + 30 * pulse);
                    
                    sf::Color blueColor(100, 149, 237);
                    
                    m_shield.setOutlineColor(sf::Color(blueColor.r, blueColor.g, blueColor.b, alphaEdge));
                    m_shield.setOutlineThickness(3.0f);
                    
                    m_shield.setFillColor(sf::Color(blueColor.r, blueColor.g, blueColor.b, alphaFill));

                    window->draw(m_shield);
                }
            }

            if (ctx.window.uiView) window->setView(*ctx.window.uiView);
        }

    private:

        void drawInternal(sf::RenderWindow& window, GameContext& ctx, ResID id, sf::Color color,
                          sf::Vector2f pos, float rotation, float radius, bool isHead) 
        {
            if (id != ResID::NONE && ctx.services.res) {
                auto& tex = ctx.services.res->get<sf::Texture>(id);
                static sf::Sprite brush;
                brush.setTexture(tex, true);
                sf::Vector2u size = tex.getSize();
                brush.setOrigin(size.x / 2.f, size.y / 2.f);
                float scale = (radius * 2.0f) / static_cast<float>(size.x);
                brush.setScale(scale, scale);
                brush.setPosition(pos);
                brush.setRotation(rotation);
                window.draw(brush);
            } 
            else {
                static sf::CircleShape circle;
                circle.setRadius(radius);
                circle.setOrigin(radius, radius);
                circle.setFillColor(color);
                circle.setOutlineThickness(-(radius * 0.4f));
                circle.setOutlineColor(sf::Color(0, 0, 0, 50)); 
                circle.setPosition(pos);
                window.draw(circle);
            }

            if (isHead && id == ResID::NONE) {
                drawEyes(window, pos, rotation, radius);
            }
        }

        void drawEyes(sf::RenderWindow& window, sf::Vector2f pos, float rotationDeg, float radius) {
            static sf::CircleShape eye;
            static sf::CircleShape pupil;
            
            float eyeRadius = radius * 0.25f;
            eye.setRadius(eyeRadius);
            eye.setOrigin(eyeRadius, eyeRadius);
            eye.setFillColor(sf::Color::White);

            pupil.setRadius(eyeRadius * 0.5f);
            pupil.setOrigin(eyeRadius * 0.5f, eyeRadius * 0.5f);
            pupil.setFillColor(sf::Color::Black);

            float rad = (rotationDeg - 90.f) * 3.14159f / 180.f;

            sf::Vector2f eyeOffsets[2] = {
                { std::cos(rad - 0.5f) * radius * 0.6f, std::sin(rad - 0.5f) * radius * 0.6f },
                { std::cos(rad + 0.5f) * radius * 0.6f, std::sin(rad + 0.5f) * radius * 0.6f }
            };

            for (int i = 0; i < 2; ++i) {
                eye.setPosition(pos + eyeOffsets[i]);
                pupil.setPosition(pos + eyeOffsets[i] + sf::Vector2f(std::cos(rad)*2.f, std::sin(rad)*2.f));
                window.draw(eye);
                window.draw(pupil);
            }
        }
    };
}
