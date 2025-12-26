#pragma once
#include <SFML/Graphics.hpp>

namespace Bocchi {
    class App;
    class EntityBuilder;
    class ResourceManager;
    class FoodSpawnSystem;

    struct TimeContext {
        float dt = 0.f;
        uint32_t frameCount = 0;
    };

    struct WindowContext {
        sf::RenderWindow* window = nullptr;
        sf::View* worldView = nullptr;
        sf::View* uiView = nullptr;
        sf::Vector2f cameraPos{};
        sf::Vector2f mouseWorldPos{};
        sf::Vector2f windowSize{};
        sf::Vector2f mapSize{5000.f, 5000.f};
    };

    struct ServiceContext {
        App* app = nullptr;
        ResourceManager* res = nullptr;
        EntityBuilder* builder = nullptr;
    };

    struct InputContext {

        bool inputConsumedByUI = false;
        bool isLeftMouseDown = false;
    };

    struct GameStateContext {
        bool isPaused = false;
        bool isPlayerDead = false;
        bool isGameOver = false;
    };

    struct FoodServices {
        FoodSpawnSystem* foodSystem = nullptr;
    };

    struct GameContext {
        TimeContext time;
        WindowContext window;
        ServiceContext services;
        InputContext input;
        GameStateContext state;
        FoodServices food;
    };
} // namespace Bocchi
