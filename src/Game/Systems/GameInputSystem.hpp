#pragma once
#include <cmath>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <entt/entt.hpp>
#include "Core/System.hpp"
#include "Core/Component.hpp"
#include "Core/Context.hpp"

namespace Bocchi {

    class GameInputSystem : public System {
    public:
        // 控制模式：Direct (持续追踪), HoldToTurn (按住转向)
        enum class ControlMode { Direct, HoldToTurn };

        GameInputSystem(ControlMode mode = ControlMode::HoldToTurn) 
            : m_mode(mode), m_tildeWasPressed(false), m_escWasPressed(false) {}

        void update(entt::registry& reg) override {
            auto& ctx = reg.ctx().get<GameContext>();

            if (ctx.input.inputConsumedByUI) return;

            bool escPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);
            if (m_escWasPressed && !escPressed) {
                ctx.state.isPaused = !ctx.state.isPaused;
            }
            m_escWasPressed = escPressed;
            if (ctx.state.isPaused) return;

            bool tildeIsPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Tilde);
            if (m_tildeWasPressed && !tildeIsPressed) {
                m_mode = (m_mode == ControlMode::Direct) ? ControlMode::HoldToTurn : ControlMode::Direct;
            }
            m_tildeWasPressed = tildeIsPressed;

            bool isMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
            
            auto view = reg.view<Position, SnakeHead, Speed, PlayerTag>();
            view.each([&](auto entity, auto& pos, auto& head, auto& speed) {

                float targetSpeed = (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && head.energyAccumulator > 0)
                    ? 320.f
                    : 180.f;
                speed.value = speed.value + (targetSpeed - speed.value) * 10.f * ctx.time.dt;
               
                bool shouldUpdate = (m_mode == ControlMode::Direct) ? true : isMousePressed;

                if (shouldUpdate) {
                    float dx = ctx.window.mouseWorldPos.x - pos.val.x;
                    float dy = ctx.window.mouseWorldPos.y - pos.val.y;

                    float distSq = dx * dx + dy * dy;
                    if (distSq > 100.f) { 
                        float targetRad = std::atan2(dy, dx);
                        head.targetAngle = targetRad * (180.f / 3.14159265f);
                    }
                }
            });
        }

    private:
        ControlMode m_mode;
        bool m_tildeWasPressed;
        bool m_escWasPressed;
        bool m_spaceWasPressed;
    };

} // namespace Bocchi
