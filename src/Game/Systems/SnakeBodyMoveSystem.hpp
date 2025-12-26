#pragma once
#include <cmath>
#include <entt/entt.hpp>
#include <vector>
#include "FoodSpawnSystem.hpp"
#include "Core/System.hpp"
#include "Core/Component.hpp"
#include "Core/Context.hpp"

namespace Bocchi {

    class SnakeBodyMoveSystem : public System {
    public:
        void update(entt::registry& reg) override {
            auto& ctx = reg.ctx().get<GameContext>();
            if (ctx.state.isPaused) return;

            auto headView = reg.view<SnakeHead, Position>();
            
            headView.each([&](entt::entity headEnt, auto& head, auto& hPos) {
                if (head.bodyEntities.empty()) return;

                float spacing = head.currentRadius * 2.0f * head.spacingFactor;
                

                float accumulatedPathDist = 0.0f;
                float targetDistForNextBody = spacing;
                size_t bodyIdx = 0;
                
                sf::Vector2f currentPoint = hPos.val;
                
                for (auto it = head.pathHistory.rbegin(); it != head.pathHistory.rend(); ++it) {
                    if (bodyIdx >= head.bodyEntities.size()) break;

                    sf::Vector2f nextPoint = *it;
                    float dx = currentPoint.x - nextPoint.x;
                    float dy = currentPoint.y - nextPoint.y;
                    float segmentLen = std::sqrt(dx*dx + dy*dy);

                    while (bodyIdx < head.bodyEntities.size() && 
                           accumulatedPathDist + segmentLen >= targetDistForNextBody) {
                        
                        float localDist = targetDistForNextBody - accumulatedPathDist;
                        float t = (segmentLen > 0.001f) ? (localDist / segmentLen) : 0.0f;
                        
                        sf::Vector2f newBodyPos = currentPoint + t * (nextPoint - currentPoint);
                        
                        entt::entity bEnt = head.bodyEntities[bodyIdx];
                        if (reg.valid(bEnt)) {
                            auto& posComp = reg.get<Position>(bEnt);   
                            if (ctx.food.foodSystem) {
                                ctx.food.foodSystem->updateBodyInGrid(bEnt, posComp.val, newBodyPos);
                            }
                            posComp.val = newBodyPos;
                        }

                        bodyIdx++;
                        targetDistForNextBody += spacing; 
                    }

                    accumulatedPathDist += segmentLen;
                    currentPoint = nextPoint;
                }

                for (; bodyIdx < head.bodyEntities.size(); ++bodyIdx) {
                    entt::entity bEnt = head.bodyEntities[bodyIdx];
                    if (reg.valid(bEnt)) {
                        reg.get<Position>(bEnt).val = currentPoint;
                    }
                }
            });
        }
    };

} // namespace Bocchi
