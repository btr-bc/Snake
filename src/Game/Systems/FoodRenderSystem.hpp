#pragma once 
#include "Core/Component.hpp"
#include "Core/System.hpp"

namespace Bocchi{
    class FoodRenderSystem : public System{
    public:
        virtual void update(entt::registry& reg) override {
            auto& ctx = reg.ctx().get<GameContext>();
            ctx.food.foodSystem->render(ctx);
        }
    };
}