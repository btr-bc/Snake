#pragma once
#include "Core/System.hpp"
#include "Core/Component.hpp"
#include "Game/Builders/EntityBuilder.hpp"
#include "Core/Config.h"
#include "Core/Context.hpp"

namespace Bocchi {
    class AiSpawnSystem : public System {
    public:
        void update(entt::registry& reg) override {
            auto& ctx = reg.ctx().get<GameContext>();
            const int MAX_AI = Config::getInstance().maxAICount;

            auto aiView = reg.view<AiTag>();
            if (aiView.size() < MAX_AI && ctx.services.builder) {
                ctx.services.builder->createRandomAi(
                    reg,
                    ctx.window.mapSize.x,
                    ctx.window.mapSize.y,
                    ctx.food.foodSystem
                ); 
            }
        }
    };
}
