#pragma once
#include "Core/System.hpp"
#include "Core/Component.hpp"

namespace Bocchi{
    class DeathSystem : public System{
    public:
        void update(entt::registry& reg) override {
            auto& ctx = reg.ctx().get<GameContext>();
            auto view = reg.view<SnakeHead, Position>();

            view.each([&](auto entity, auto& head, auto& pos) {
                if (!head.isDead) return;

                for (auto bodyEnt : head.bodyEntities) {
                    if (reg.valid(bodyEnt)) {
                        auto& bPos = reg.get<Position>(bodyEnt).val;

                        if (ctx.food.foodSystem) {
                            ctx.food.foodSystem->removeBodyFromGrid(bodyEnt, bPos);
                        }

                        if (ctx.food.foodSystem) {
                            ctx.food.foodSystem->spawnFood(
                            bPos, 
                            FoodType::MassDrop, 
                            head.currentRadius * 0.1f,
                            head.foodID,
                            head.color,
                            head.currentRadius * 0.8f
                            );
                        }

                        reg.destroy(bodyEnt);
                    }
                }
                head.bodyEntities.clear();

                if (reg.all_of<PlayerTag>(entity)) {
                    handlePlayerDeath(reg, entity, head, ctx);
                } else {
                    reg.destroy(entity);
                }
            });
        }


        void handlePlayerDeath(entt::registry& reg, entt::entity entity, SnakeHead& head, GameContext& ctx) {
            ctx.state.isPaused = true;
            
        }


    };
}
