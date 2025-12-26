#include "TestWorld.h"
#include "Game/Systems/FoodSpawnSystem.hpp"
#include <random>

namespace Bocchi{
    void TestWorld::init(const GameContext& ctx){
        if (auto* existing = m_registry.ctx().find<GameContext>()) {
            *existing = ctx;
        } else {
            m_registry.ctx().emplace<GameContext>(ctx);
        }

        auto& gctx = m_registry.ctx().get<GameContext>();

        

        auto foodSystem = std::make_unique<FoodSpawnSystem>(gctx.window.mapSize.x, gctx.window.mapSize.y);
        m_foodSystem = foodSystem.get();
        gctx.food.foodSystem = m_foodSystem;
        m_systems.push_back(std::move(foodSystem));

        addSystem<ClassicBackgroundRenderSystem>();
        addSystem<AiSpawnSystem>();
        addSystem<InputSystem>();
        addSystem<GameInputSystem>();
        addSystem<AiControlSystem>();
        addSystem<SnakeHeadMoveSystem>();
        addSystem<SnakeGrowthSystem>();
        addSystem<SnakeBodyMoveSystem>();
        addSystem<CollisionSystem>();
        addSystem<DeathSystem>();
        addSystem<CameraSystem>();
        
        addSystem<FoodRenderSystem>();
        addSystem<SnakeRenderSystem>();
        addSystem<PauseRenderSystem>();

        auto& config = Config::getInstance();

        std::mt19937 m_rng{ std::random_device{}() };

        std::uniform_real_distribution<float> distX(200.f, config.mapWidth - 200.f);
        std::uniform_real_distribution<float> distY(200.f, config.mapHeight - 200.f);
        sf::Vector2f randPos(distX(m_rng), distY(m_rng));

        gctx.services.builder->createSnake(
            m_registry, 
            randPos,
            std::uniform_real_distribution<float>(0, 360)(m_rng),
            true, 
            ResID::head_shantianliang, 
            ResID::body_shantianliang,
            ResID::food_pingguohe,
            sf::Color::Red,
            5,
            m_foodSystem
        );
    }

    void TestWorld::quit() {
        m_foodSystem = nullptr;
    }
}
