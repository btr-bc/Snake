#pragma once
#include <cmath>
#include "Core/Component.hpp"
#include "Core/System.hpp"
#include "FoodSpawnSystem.hpp"

namespace Bocchi{
    class CollisionSystem : public System {
    public:
        void update(entt::registry& reg) override {
            auto& ctx = reg.ctx().get<GameContext>();
            auto headView = reg.view<SnakeHead, Position, CircleCollider>();
            auto* foodSys = ctx.food.foodSystem;
            if (!foodSys) return;

            if (ctx.state.isPaused) return;

            headView.each([&](auto entity, auto& head, auto& pos, auto& col) {
                if (head.isDead) return;
                if (head.spawnProtectionTime > 0) return;

                if (pos.val.x < 0 || pos.val.x > ctx.window.mapSize.x || pos.val.y < 0 || pos.val.y > ctx.window.mapSize.y) {
                    head.isDead = true;
                    return;
                }

                int gx = static_cast<int>(pos.val.x / foodSys->getCellSize());
                int gy = static_cast<int>(pos.val.y / foodSys->getCellSize());

                for (int x = gx - 1; x <= gx + 1; ++x) {
                    for (int y = gy - 1; y <= gy + 1; ++y) {
                        const auto& bodies = foodSys->getBodiesInCell(x, y);
                        for (auto bEnt : bodies) {
                            if (!reg.valid(bEnt)) continue;
                            auto& bData = reg.get<SnakeBody>(bEnt);

                            auto* ownerHead = reg.try_get<SnakeHead>(bData.headOwner);
                            if (!ownerHead || ownerHead->spawnProtectionTime > 0) continue;     

                            if (bData.headOwner == entity) continue;

                            auto& bPos = reg.get<Position>(bEnt);
                            auto& bCol = reg.get<CircleCollider>(bEnt);
                            
                            float dx = pos.val.x - bPos.val.x;
                            float dy = pos.val.y - bPos.val.y;
                            float distSq = dx * dx + dy * dy;

                            if (distSq < std::pow(col.radius + bCol.radius, 2)) {
                                head.isDead = true;
                                return;
                            }
                        }
                    }
                }
            });
        }
    };
}
