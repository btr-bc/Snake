#pragma once
#include <cmath>
#include <entt/entt.hpp>
#include "Core/System.hpp"
#include "Core/Component.hpp"
#include "Core/Context.hpp"

namespace Bocchi {

    class SnakeHeadMoveSystem : public System {
    public:
        void update(entt::registry& reg) override {
            auto& ctx = reg.ctx().get<GameContext>();
            if (ctx.state.isPaused) return;

            const float dt = ctx.time.dt;
            const float PI = 3.14159265f;
            const float DEG_TO_RAD = PI / 180.f;
            const float RAD_TO_DEG = 180.f / PI;

            auto view = reg.view<Position, Rotation, SnakeHead, Speed>();

            view.each([&](auto entity, auto& pos, auto& rot, auto& head, auto& speed) {

                float currentRad = rot.angle * DEG_TO_RAD;
                float targetRad = head.targetAngle * DEG_TO_RAD;
                float angleDiff = targetRad - currentRad;

                while (angleDiff < -PI) angleDiff += 2 * PI;
                while (angleDiff > PI)  angleDiff -= 2 * PI;

                float maxTurn = head.turnSpeed * dt;
                if (std::abs(angleDiff) < maxTurn) {
                    currentRad = targetRad;
                } else {
                    currentRad += std::copysign(maxTurn, angleDiff);
                }
                
                rot.angle = currentRad * RAD_TO_DEG;

                sf::Vector2f oldPos = pos.val;
                pos.val.x += std::cos(currentRad) * speed.value * dt;
                pos.val.y += std::sin(currentRad) * speed.value * dt;

                float dx = pos.val.x - oldPos.x;
                float dy = pos.val.y - oldPos.y;
                float distMoved = std::sqrt(dx * dx + dy * dy);
                
                head.distAccumulator += distMoved;
                const float samplingDist = 5.0f;

                if (head.distAccumulator >= samplingDist) {

                    head.pathHistory.push_back(pos.val);
                    head.distAccumulator = 0.0f;

                    if (head.pathHistory.size() > 2000) {
                        head.pathHistory.erase(head.pathHistory.begin());
                    }
                }

                
                if (head.spawnProtectionTime > 0) {
                    head.spawnProtectionTime -= ctx.time.dt;
                }
            });
        }
    };

} // namespace Bocchi
