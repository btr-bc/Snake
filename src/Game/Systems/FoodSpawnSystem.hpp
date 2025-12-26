#pragma once
#include <vector>
#include <list>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include "Core/System.hpp"
#include "Core/Component.hpp"
#include "Core/Context.hpp"
#include "Core/ResourceManager.h"
#include "Core/Config.h"

namespace Bocchi {

    enum class FoodType { 
        Normal,
        MassDrop,
    };

    struct FoodItem {
        sf::Vector2f pos;
        FoodType type;
        float energyValue;
        sf::Color color;
        ResID resID;
        bool active = true;
        float radius;
    };

    class FoodSpawnSystem : public System {
        float m_mapWidth, m_mapHeight, m_cellSize;
        int m_cols, m_rows;
        std::vector<FoodItem> m_foods;
        std::vector<std::list<size_t>> m_foodGrid;
        std::vector<std::unordered_set<entt::entity>> m_bodyGrid;
        std::vector<size_t> m_freeIndices;

        int MAX_TOTAL_FOOD;
        int MIN_PER_CELL;
        float SPAWN_CHANCE;

        inline static std::vector<sf::Color> m_colorPalette;

    public:
        FoodSpawnSystem(float mapWidth = 0.f, float mapHeight = 0.f, float cellSize = 0.f) {
            MAX_TOTAL_FOOD = Config::getInstance().maxTotalFood;
            MIN_PER_CELL = Config::getInstance().minFoodPerCell;
            SPAWN_CHANCE = Config::getInstance().spawnChance;
            m_mapWidth  = (mapWidth == 0.f)  ? Config::getInstance().mapWidth  : mapWidth;
            m_mapHeight = (mapHeight == 0.f) ? Config::getInstance().mapHeight : mapHeight;
            m_cellSize  = (cellSize == 0.f)  ? Config::getInstance().cellSize  : cellSize;
            
            m_cols = static_cast<int>(m_mapWidth / m_cellSize) + 1;
            m_rows = static_cast<int>(m_mapHeight / m_cellSize) + 1;
            
            m_foodGrid.resize(m_cols * m_rows);
            m_bodyGrid.resize(m_cols * m_rows);

            m_colorPalette = {
                sf::Color(255, 100, 100), sf::Color(100, 255, 100),
                sf::Color(100, 100, 255), sf::Color(255, 255, 100),
                sf::Color(255, 100, 255), sf::Color(100, 255, 255)
            };
        }

        float getCellSize() const { return m_cellSize; }
        int getCols() const { return m_cols; }
        int getRows() const { return m_rows; }

        const std::unordered_set<entt::entity>& getBodiesInCell(int gx, int gy) const {
            static const std::unordered_set<entt::entity> empty;
            if (gx < 0 || gx >= m_cols || gy < 0 || gy >= m_rows) return empty;
            return m_bodyGrid[gy * m_cols + gx];
        }
        void updateBodyInGrid(entt::entity ent, sf::Vector2f oldPos, sf::Vector2f newPos) {
            int ox = static_cast<int>(oldPos.x / m_cellSize);
            int oy = static_cast<int>(oldPos.y / m_cellSize);
            int nx = static_cast<int>(newPos.x / m_cellSize);
            int ny = static_cast<int>(newPos.y / m_cellSize);

            if (ox != nx || oy != ny) {
                removeBodyFromGrid(ent, oldPos);
                addBodyToGrid(ent, newPos);
            }
        }

        void addBodyToGrid(entt::entity ent, sf::Vector2f pos) {
            int gx = static_cast<int>(pos.x / m_cellSize);
            int gy = static_cast<int>(pos.y / m_cellSize);
            if (gx >= 0 && gx < m_cols && gy >= 0 && gy < m_rows) {
                m_bodyGrid[gy * m_cols + gx].insert(ent);
            }
        }

        void removeBodyFromGrid(entt::entity ent, sf::Vector2f pos) {
            int gx = static_cast<int>(pos.x / m_cellSize);
            int gy = static_cast<int>(pos.y / m_cellSize);
            if (gx >= 0 && gx < m_cols && gy >= 0 && gy < m_rows) {
                m_bodyGrid[gy * m_cols + gx].erase(ent);
            }
        }

        void spawnFood(sf::Vector2f pos, FoodType type, float energy, ResID resID, sf::Color color, float radius = 6.f) {
            size_t index;

            sf::Vector2f finalPos = pos;
            if (type == FoodType::MassDrop) {
                float angle = (rand() % 360) * 3.14159f / 180.f;
                float dist = static_cast<float>(rand() % 25); 
                finalPos.x += std::cos(angle) * dist;
                finalPos.y += std::sin(angle) * dist;
                finalPos.x = std::clamp(finalPos.x, 0.f, m_mapWidth);
                finalPos.y = std::clamp(finalPos.y, 0.f, m_mapHeight);
            }

            if (!m_freeIndices.empty()) {
                index = m_freeIndices.back();
                m_freeIndices.pop_back();
                removeFoodFromGrid(index);
                m_foods[index] = {finalPos, type, energy, color, resID, true, radius};
            } else {
                index = m_foods.size();
                m_foods.push_back({finalPos, type, energy, color, resID, true, radius});
            }
            addFoodToGrid(index);
        }

        void update(entt::registry& reg) override {
            auto& ctx = reg.ctx().get<GameContext>();
            if (ctx.state.isPaused) return;
            if (ctx.food.foodSystem != this) ctx.food.foodSystem = this;

            auto snakeView = reg.view<Position, SnakeHead, CircleCollider, MagnetRange>();

            for (auto snake : snakeView) {
                auto& sPos = snakeView.get<Position>(snake).val;
                auto& sHead = snakeView.get<SnakeHead>(snake);
                float sRadius = snakeView.get<CircleCollider>(snake).radius;
                float sMagnet = snakeView.get<MagnetRange>(snake).range;

                int gx = static_cast<int>(sPos.x / m_cellSize);
                int gy = static_cast<int>(sPos.y / m_cellSize);

                for (int x = gx - 1; x <= gx + 1; ++x) {
                    for (int y = gy - 1; y <= gy + 1; ++y) {
                        if (x < 0 || x >= m_cols || y < 0 || y >= m_rows) continue;

                        auto& cell = m_foodGrid[y * m_cols + x];
                        auto it = cell.begin();
                        while (it != cell.end()) {
                            FoodItem& food = m_foods[*it];
                            if (!food.active) {
                                it = cell.erase(it);
                                continue;
                            }

                            float dx = sPos.x - food.pos.x;
                            float dy = sPos.y - food.pos.y;
                            float distSq = dx * dx + dy * dy;
                            if (distSq < std::pow(sRadius + food.radius, 2)) {
                                applyCollectionEffect(sHead, food);
                                if (reg.all_of<PlayerTag, SoundComponent>(snake)) {
                                    auto& sc = reg.get<SoundComponent>(snake);
                                    ResID finalID = (sc.soundID != ResID::NONE) ? sc.soundID : ResID::eat_sound_maodie;
                                    const auto& buffer = ctx.services.res->get<sf::SoundBuffer>(finalID);
                                    sc.sound.setBuffer(buffer);
                                    if (food.energyValue > 1) {
                                        sc.sound.setVolume(50);
                                        sc.sound.play();
                                    }                               
                                }
                                food.active = false;
                                m_freeIndices.push_back(*it);
                                it = cell.erase(it);
                                continue;
                            }

                            if (distSq < sMagnet * sMagnet) {
                                float dist = std::sqrt(distSq);
                                sf::Vector2f dir = (sPos - food.pos) / dist;
                                food.pos += dir * 650.f * ctx.time.dt; 
                            }
                            ++it;
                        }
                    }
                }
            }

            static float genTimer = 0;
            genTimer += ctx.time.dt;
            if (genTimer > 0.5f) {
                handleMapGeneration();
                genTimer = 0;
            }
        }

        void render(GameContext& ctx) {
            auto* window = ctx.window.window;
            auto* res = ctx.services.res;
            if (!window || !res) return;
            if (ctx.window.worldView) window->setView(*ctx.window.worldView);

            static sf::CircleShape dot;
            static sf::Sprite sprite;
            for (const auto& food : m_foods) {
                if (!food.active) continue;
                if (food.resID != ResID::NONE) {
                    auto& tex = res->get<sf::Texture>(food.resID);
                    sprite.setTexture(tex, true);
                    sprite.setOrigin(tex.getSize().x / 2.f, tex.getSize().y / 2.f);
                    sprite.setPosition(food.pos);
                    float scale = food.radius / tex.getSize().x * 3.5f;
                    sprite.setScale(scale, scale);
                    window->draw(sprite);
                } else {
                    dot.setFillColor(food.color);
                    float r = (food.type == FoodType::MassDrop ? food.radius : 6.f);
                    dot.setRadius(r);
                    dot.setOrigin(r, r);
                    dot.setPosition(food.pos);
                    window->draw(dot);
                }
            }
        }

    private:
        void handleMapGeneration() {
            int activeCount = 0;
            for(const auto& f : m_foods) if(f.active) activeCount++;
            if (activeCount >= MAX_TOTAL_FOOD) return;

            float globalFactor = 1.0f - (static_cast<float>(activeCount) / MAX_TOTAL_FOOD);
            for (int y = 0; y < m_rows; ++y) {
                for (int x = 0; x < m_cols; ++x) {
                    auto& cell = m_foodGrid[y * m_cols + x];
                    if (cell.size() < (size_t)MIN_PER_CELL || (static_cast<float>(rand())/RAND_MAX) < (SPAWN_CHANCE * globalFactor)) {
                        trySpawnInCell(x, y);
                    }
                }
            }
        }

        void trySpawnInCell(int gx, int gy) {
            float x = std::clamp((gx + static_cast<float>(rand())/RAND_MAX) * m_cellSize, 0.f, m_mapWidth);
            float y = std::clamp((gy + static_cast<float>(rand())/RAND_MAX) * m_cellSize, 0.f, m_mapHeight);
            sf::Color col = m_colorPalette[rand() % m_colorPalette.size()];
            float energy = 1.0f;
            spawnFood({x, y}, FoodType::Normal, energy, ResID::NONE, col);
        }

        void addFoodToGrid(size_t index) {
            auto& f = m_foods[index];
            int gx = static_cast<int>(f.pos.x / m_cellSize);
            int gy = static_cast<int>(f.pos.y / m_cellSize);
            if (gx >= 0 && gx < m_cols && gy >= 0 && gy < m_rows) {
                m_foodGrid[gy * m_cols + gx].push_back(index);
            }
        }

        void removeFoodFromGrid(size_t index) {
            auto& f = m_foods[index];
            int gx = static_cast<int>(f.pos.x / m_cellSize);
            int gy = static_cast<int>(f.pos.y / m_cellSize);
            if (gx >= 0 && gx < m_cols && gy >= 0 && gy < m_rows) {
                m_foodGrid[gy * m_cols + gx].remove(index);
            }
        }

        void applyCollectionEffect(SnakeHead& head, const FoodItem& food) {
            head.energyAccumulator += food.energyValue;
            head.totalEnergy += food.energyValue;

            float growthThreshold = 10.f + (head.currentLength * 0.5f);

            if (head.energyAccumulator >= growthThreshold) { 
                head.pendingGrowth += 1;
                head.energyAccumulator -= growthThreshold;
            }

        }   
            

    public:
        bool isAreaSafe(sf::Vector2f pos, float radius) {
            int gx = static_cast<int>(pos.x / m_cellSize);
            int gy = static_cast<int>(pos.y / m_cellSize);
            for (int x = gx - 1; x <= gx + 1; ++x) {
                for (int y = gy - 1; y <= gy + 1; ++y) {
                    if (x < 0 || x >= m_cols || y < 0 || y >= m_rows) continue;
                    if (!m_bodyGrid[y * m_cols + x].empty()) return false; 
                }
            }
            return true;
        }

        bool findNearestLoot(sf::Vector2f pos, float range, sf::Vector2f& outPos) {
            int r = static_cast<int>(range / m_cellSize) + 1;
            int gx = static_cast<int>(pos.x / m_cellSize);
            int gy = static_cast<int>(pos.y / m_cellSize);
            float minDistSq = range * range;
            bool found = false;

            for (int x = gx - r; x <= gx + r; ++x) {
                for (int y = gy - r; y <= gy + r; ++y) {
                    if (x < 0 || x >= m_cols || y < 0 || y >= m_rows) continue;
                    for (size_t idx : m_foodGrid[y * m_cols + x]) {
                        const auto& food = m_foods[idx];
                        if (food.active && food.type == FoodType::MassDrop) {
                            float dSq = std::pow(food.pos.x - pos.x, 2) + std::pow(food.pos.y - pos.y, 2);
                            if (dSq < minDistSq) {
                                minDistSq = dSq;
                                outPos = food.pos;
                                found = true;
                            }
                        }
                    }
                }
            }
            return found;
        }

        sf::Vector2f getAverageBodyOffset(sf::Vector2f pos, float range) {
            int r = static_cast<int>(range / m_cellSize) + 1;
            int gx = static_cast<int>(pos.x / m_cellSize);
            int gy = static_cast<int>(pos.y / m_cellSize);
            sf::Vector2f sum{0.f, 0.f};
            int count = 0;

            for (int x = gx - r; x <= gx + r; ++x) {
                for (int y = gy - r; y <= gy + r; ++y) {
                    if (x < 0 || x >= m_cols || y < 0 || y >= m_rows) continue;
                    for (auto ent : m_bodyGrid[y * m_cols + x]) {
                        sum += sf::Vector2f(static_cast<float>(x) * m_cellSize, static_cast<float>(y) * m_cellSize);
                        count++;
                    }
                }
            }
            if (count == 0) return {0.f, 0.f};
            return (sum / static_cast<float>(count)) - pos;
        }
    };
}