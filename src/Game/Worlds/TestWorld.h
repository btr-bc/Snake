#pragma once
#include "Core/World.hpp"
#include "Game/Systems/ClassicBackgroundRenderSystem.hpp"
#include "Game/Systems/GameInputSystem.hpp"
#include "Game/Systems/SnakeHeadMoveSystem.hpp"
#include "Game/Systems/SnakeBodyMoveSystem.hpp"
#include "Game/Systems/InputSystem.hpp"
#include "Game/Systems/SnakeRenderSystem.hpp"
#include "Game/Systems/CameraSystem.hpp"
#include "Game/Systems/PauseRenderSystem.hpp"
#include "Game/Systems/SnakeGrowthSystem.hpp"
#include "Game/Systems/FoodSpawnSystem.hpp"
#include "Game/Systems/FoodRenderSystem.hpp"
#include "Game/Systems/CollisionSystem.hpp"
#include "Game/Systems/DeathSystem.hpp"
#include "Game/Systems/AiControlSystem.hpp"
#include "Game/Systems/AiSpawnSystem.hpp"

#include "Game/Builders/EntityBuilder.hpp"

namespace Bocchi{
    class TestWorld : public World {
    public:
        virtual void init(const GameContext& ctx) override;
        virtual void quit() override;
    private:
        class FoodSpawnSystem* m_foodSystem = nullptr;
    };
}
