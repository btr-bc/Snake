#pragma once
#include <cmath>
#include <random>
#include "Core/Component.hpp"
#include "Core/ResourceManager.h"
#include "Core/Config.h"
#include "Game/Systems/FoodSpawnSystem.hpp"

namespace Bocchi {


    struct SkinSet {
        ResID head;
        ResID body;
        ResID food;
    };

    
    class EntityBuilder {
    private:


        std::vector<SkinSet> m_skinPool = {
            {ResID::head_maodie, ResID::body_maodie, ResID::food_huotuichang },
            {ResID::head_maodie_o, ResID::body_maodie, ResID::food_huotuichang },
            {ResID::head_shantianliang, ResID::body_shantianliang, ResID::food_pingguohe},
            {ResID::head_xiduoyudai, ResID::NONE, ResID::food_bocchi}
        };

        std::mt19937 m_rng{ std::random_device{}() };
    
    public:
        EntityBuilder() = default;

        entt::entity createSnake(
            entt::registry& registry, 
            sf::Vector2f pos,
            float rotation, 
            bool isPlayer,
            ResID headID = ResID::NONE, 
            ResID bodyID = ResID::NONE,
            ResID foodID = ResID::NONE,
            sf::Color color = sf::Color::Red,
            int length = 0,
            FoodSpawnSystem* foodSystem = nullptr
        ) {
            const auto& config = Config::getInstance();
            if (length == 0) {
                length = config.defaultSnakeLength;
            }
            
            const float DEG_TO_RAD = 3.14159265f / 180.f;

            entt::entity snakeHead = registry.create();
            auto& headData = registry.emplace<SnakeHead>(snakeHead);
            
            headData.headID = headID;
            headData.bodyID = bodyID;
            headData.foodID = foodID;
            headData.targetAngle = rotation;
            headData.currentLength = length; 
            headData.currentRadius = config.defaultSnakeRadius; 
            headData.color = color;
            headData.spawnProtectionTime = 5.f;

            registry.emplace<Position>(snakeHead, pos);
            registry.emplace<Rotation>(snakeHead, rotation);
            registry.emplace<Speed>(snakeHead, config.defaultSnakeSpeed);
            registry.emplace<CircleCollider>(snakeHead, headData.currentRadius);
            registry.emplace<MagnetRange>(snakeHead, headData.currentRadius * 2.5f);

            if (isPlayer) registry.emplace<PlayerTag>(snakeHead);

            float spacing = headData.currentRadius * 2.0f * headData.spacingFactor;
            float rad = rotation * DEG_TO_RAD;
            sf::Vector2f dir(-std::cos(rad), -std::sin(rad));

            int pointsToPreGen = length + 5; 
            for (int i = 1; i <= pointsToPreGen; ++i) {
                sf::Vector2f historyPos = pos + dir * (spacing * (pointsToPreGen - i + 1));
                headData.pathHistory.push_back(historyPos);
            }


            for (int i = 0; i < length; ++i) {
                entt::entity snakeBody = registry.create();
                
                registry.emplace<SnakeBody>(snakeBody, snakeHead, i);
                
                float offsetDist = spacing * (i + 1);
                sf::Vector2f bodyPos = pos + dir * offsetDist;
                registry.emplace<Position>(snakeBody, bodyPos);
                
                registry.emplace<CircleCollider>(snakeBody, headData.currentRadius * 0.85f);
                if (headID == ResID::NONE) {
                    registry.emplace<ColorComponent>(snakeBody, color);
                }

                headData.bodyEntities.push_back(snakeBody);
            }

            if (foodSystem) {
                for (auto bodyEnt : headData.bodyEntities) {
                    auto& bPos = registry.get<Position>(bodyEnt).val;
                    foodSystem->addBodyToGrid(bodyEnt, bPos);
                }
            }

            switch (headData.headID) {
                case ResID::head_maodie:
                case ResID::head_maodie_o:
                case ResID::head_shantianliang:
                    registry.emplace<SoundComponent>(snakeHead, ResID::eat_sound_maodie);
                    break;
                case ResID::head_xiduoyudai:
                    break;
                default:
                    registry.emplace<SoundComponent>(snakeHead, ResID::eat_sound_maodie);
            }

            return snakeHead;
        }


        entt::entity createRandomAi(entt::registry& registry, float mapWidth, float mapHeight, FoodSpawnSystem* foodSystem = nullptr) {
            const auto& config = Config::getInstance();
            
            std::uniform_real_distribution<float> distX(200.f, mapWidth - 200.f);
            std::uniform_real_distribution<float> distY(200.f, mapHeight - 200.f);
            sf::Vector2f randPos(distX(m_rng), distY(m_rng));

            std::uniform_int_distribution<int> rollDist(0, 99);
            int roll = rollDist(m_rng);
            
            int level = 1;
            int length = 5;
            float baseSpeed = config.defaultSnakeSpeed;


            if (roll < 10) {
                level = 3;
                std::uniform_int_distribution<int> lenDist(20, 30);
                length = lenDist(m_rng);
            } 
            else if (roll < 40) {
                level = 2;
                std::uniform_int_distribution<int> lenDist(10, 20);
                length = lenDist(m_rng);
            } 
            else {
                level = 1;
                std::uniform_int_distribution<int> lenDist(5, 10);
                length = lenDist(m_rng);
                baseSpeed = config.defaultSnakeSpeed;
            }


            ResID headID = ResID::NONE, bodyID = ResID::NONE, foodID = ResID::NONE;
            sf::Color color = sf::Color::Red;

            
            if (level >= 2 && !m_skinPool.empty()) {
                std::uniform_int_distribution<size_t> skinDist(0, m_skinPool.size() - 1);
                const auto& skin = m_skinPool[skinDist(m_rng)];
                headID = skin.head;
                bodyID = skin.body;
                foodID = skin.food;
            } else {
                static const std::vector<sf::Color> aiColors = {
                    sf::Color::Yellow, sf::Color::Cyan, sf::Color::Magenta, 
                    sf::Color(255, 128, 0), sf::Color(0, 255, 128)
                };
                std::uniform_int_distribution<size_t> colorDist(0, aiColors.size() - 1);
                color = aiColors[colorDist(m_rng)];
            }

            entt::entity aiHead = createSnake(registry, randPos, 
                                            std::uniform_real_distribution<float>(0, 360)(m_rng), 
                                            false, headID, bodyID, foodID, 
                                            color, length, foodSystem);

            registry.emplace<AiTag>(aiHead, level);
            registry.replace<Speed>(aiHead, baseSpeed);

            return aiHead;
        }

    };

}
