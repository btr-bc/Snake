#pragma once
#include <string>

namespace Bocchi {

struct Config {
    float mapWidth = 8000.0f;
    float mapHeight = 8000.0f;
    float cellSize = 250.0f;
    
    float defaultSnakeRadius = 20.0f;
    float defaultSnakeSpeed = 180.0f;
    int defaultSnakeLength = 5;

    float baseRadius = 20.0f;
    float growthScale = 1.2f;
    float maxRadius = 100.0f;
    
    int maxAICount = 15;

    int maxTotalFood = 1500;
    int minFoodPerCell = 1;
    float spawnChance = 0.01f;
    
    int windowWidth = 800;
    int windowHeight = 600;
    std::string windowTitle = "Snake";
    
    static Config& getInstance() {
        static Config instance;
        return instance;
    }
    
private:
    Config() = default;
};

} // namespace Bocchi