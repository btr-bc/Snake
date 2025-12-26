#pragma once
#include <SFML/Graphics.hpp>
#include "Core/System.hpp"
#include "Core/Context.hpp"

namespace Bocchi{
    class StaticBackgroundSystem : public System {
    public:
        StaticBackgroundSystem(sf::Texture& tex) {
            m_sprite.setTexture(tex);
        }

        inline void update(entt::registry& reg) override {
            auto& ctx = reg.ctx<GameContext>();
            if (ctx.window.window) {
                ctx.window.window->draw(m_sprite);
            }
        }

    private:
        sf::Sprite m_sprite;
    };
}
