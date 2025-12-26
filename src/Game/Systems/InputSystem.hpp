#pragma once
#include "Core/System.hpp"
#include "Core/Component.hpp"

namespace Bocchi {
    class InputSystem : public System {
    public:
        void update(entt::registry& reg) override {
            auto& ctx = reg.ctx().get<GameContext>();

            if (ctx.window.window && ctx.window.worldView) {
                sf::Vector2i pixelPos = sf::Mouse::getPosition(*ctx.window.window);
                ctx.window.mouseWorldPos = ctx.window.window->mapPixelToCoords(pixelPos, *ctx.window.worldView);
            }
            ctx.input.isLeftMouseDown = sf::Mouse::isButtonPressed(sf::Mouse::Left);

            ctx.input.inputConsumedByUI = false;
        }
    };
}
