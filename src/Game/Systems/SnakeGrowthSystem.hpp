#pragma once
#include <cmath>
#include <entt/entt.hpp>
#include "Core/System.hpp"
#include "Core/Component.hpp"
#include "Core/Context.hpp"
#include "Core/Config.h"
#include <algorithm>

namespace Bocchi {

    class SnakeGrowthSystem : public System {
    public:
        void update(entt::registry& registry) override {
            auto& ctx = registry.ctx().get<GameContext>();
            if (ctx.state.isPaused) return;
            auto& config = Config::getInstance();

            auto view = registry.view<SnakeHead, Position>();
            view.each([&](auto entity, auto& head, auto& pos) {
                float targetRadius = std::min(
                    static_cast<float>(config.defaultSnakeRadius + (config.defaultSnakeRadius * std::pow(head.currentLength-5, 0.5f) * 0.3f)),
                    config.maxRadius
                );        
                if (std::abs(head.currentRadius - targetRadius) > 0.1f) {
                    head.currentRadius += (targetRadius - head.currentRadius) * 2.0f * ctx.time.dt;
                }

            float deltaR = std::max(0.f, head.currentRadius - config.defaultSnakeRadius);
            head.turnSpeed = 8.f * (0.4f + 0.6f* std::exp(-0.02f * deltaR));

                while (head.pendingGrowth > 0) {
                    spawnBodySegment(registry, entity, head);
                    head.pendingGrowth--;
                }

                if (registry.all_of<CircleCollider>(entity)) {
                    registry.get<CircleCollider>(entity).radius = head.currentRadius;
                }

            });
        }

    private:
        void spawnBodySegment(entt::registry& reg, entt::entity headOwner, SnakeHead& head) {
            auto bodyEnt = reg.create();
            
            int newIdx = ++head.currentLength; 
            reg.emplace<SnakeBody>(bodyEnt, headOwner, newIdx);        
            reg.emplace<Position>(bodyEnt, sf::Vector2f(-10000.f, -10000.f));
            reg.emplace<CircleCollider>(bodyEnt, head.currentRadius * 0.9f);

            head.bodyEntities.push_back(bodyEnt);
        }
    };

} // namespace Bocchi
