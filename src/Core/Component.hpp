#pragma once
#include "ResourceManager.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <deque>
#include <entt/entt.hpp>

namespace Bocchi {

    struct Position {
        sf::Vector2f val;
        Position() = default;
        Position(sf::Vector2f p) : val(p) {}
        Position(float x, float y) : val(x, y) {}
    };

    struct Rotation {
        float angle = 0.0f;
    };

    struct Speed {
        float value = 180.0f;
    };

    struct Scale {
        float val = 1.0f;
    };

    struct CircleCollider {
        float radius = 20.0f;
    };

    struct MagnetRange {
        float range = 150.0f;
    };

    struct SnakeHead {
        SnakeHead() { pathHistory.reserve(2000); }

        float targetAngle = 0.0f;
        float turnSpeed = 8.0f; 
        
        float currentRadius = 20.0f; 
        float spacingFactor = 0.8f;
        float distAccumulator = 0.0f;

        std::vector<sf::Vector2f> pathHistory;
        std::vector<entt::entity> bodyEntities;

        int pendingGrowth = 0;
        int currentLength = 1;
        float energyAccumulator = 0.0f;
        float totalEnergy = 0.0f;

        ResID headID;
        ResID bodyID;
        ResID foodID;

        sf::Color color;

        bool isDead = false;

        float spawnProtectionTime;
    };

    struct SnakeBody {
        entt::entity headOwner;
        int segmentIndex;
        
        SnakeBody(entt::entity head, int index) 
            : headOwner(head), segmentIndex(index) {}
    };

    // struct FoodData {
    //     ResID resID;
    //     float energy;  
    //     bool isPersistent;
    //     sf::Color color;
    //     float radius;
    // };


    struct PlayerTag {};
    struct AiTag{
        int level = 1;
        float stateTimer = 0.f;
        int prevSlot = -1;
        float momentumBias = 0.f;
        float cachedFrontDanger = 0.f;
        float cachedFrontLeftDanger = 0.f;
        float cachedFrontRightDanger = 0.f;
    };
    // struct FoodTag   {}; 

    // struct Collectible {
    //     float energyValue = 1.0f;
    // };

    struct ColorComponent {
        sf::Color color = sf::Color::White;
    };

    struct SoundComponent {
        ResID soundID = ResID::NONE;
        sf::Sound sound; 
    };


    struct Wallet {
        int coins = 0;
    };

    struct Inventory {
        std::vector<int> itemIds;
    };

} // namespace Bocchi
