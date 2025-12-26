#pragma once
#include <entt/entt.hpp>
#include <cmath>
#include <algorithm>
#include <array>
#include "Core/System.hpp"
#include "Core/Component.hpp"
#include "Core/Context.hpp"
#include "FoodSpawnSystem.hpp"

namespace Bocchi {

    class AiControlSystem : public System {
    public:
        void update(entt::registry& reg) override {
            auto& ctx = reg.ctx().get<GameContext>();
            

            if (ctx.state.isPaused || !ctx.food.foodSystem) return;

            auto view = reg.view<SnakeHead, Speed, Position, Rotation, AiTag>();
            
            view.each([&](auto entity, auto& head, auto& speed, auto& pos, auto& rot, auto& ai) {
                bool isHeavyTick = (ctx.time.frameCount + static_cast<uint32_t>(entity)) % 10 == 0;
                float desiredSpeed = 220.f;
                bool behaviorTaken = false;

                if (isHeavyTick) {
                    evaluateContext(entity, head, speed, pos, rot, ai, reg, ctx, desiredSpeed);
                    behaviorTaken = true;
                }

                if (!behaviorTaken && !head.isDead) {
                    float wander = std::sin((ctx.time.frameCount + static_cast<uint32_t>(entity)) * 0.17f) * 5.f;
                    applySteer(head, head.targetAngle + wander, ctx.time.dt);
                }

                float accel = 6.0f;
                speed.value += (desiredSpeed - speed.value) * accel * ctx.time.dt;
            });
        }

    private:
        float normalizeDeg(float deg) {
            while (deg > 180.f) deg -= 360.f;
            while (deg < -180.f) deg += 360.f;
            return deg;
        }

        void applySteer(SnakeHead& head, float desiredAngleDeg, float dt) {
            float diff = normalizeDeg(desiredAngleDeg - head.targetAngle);
            float maxStep = std::max(30.f, 180.f * dt);
            diff = std::clamp(diff, -maxStep, maxStep);
            head.targetAngle = normalizeDeg(head.targetAngle + diff);
        }

        void evaluateContext(entt::entity entity, SnakeHead& head, Speed& speed, Position& pos, Rotation& rot,
                             AiTag& ai, entt::registry& reg, GameContext& ctx, float& desiredSpeed) {
            static constexpr int SLOT_COUNT = 24;

            static const std::array<sf::Vector2f, SLOT_COUNT> slotDirs = []() {
                std::array<sf::Vector2f, SLOT_COUNT> dirs{};
                for (int i = 0; i < SLOT_COUNT; ++i) {
                    float rad = i * 15.f * 3.1415926535f / 180.f;
                    dirs[i] = sf::Vector2f(std::cos(rad), std::sin(rad));
                }
                return dirs;
            }();


            std::array<float, SLOT_COUNT> danger{};
            std::array<float, SLOT_COUNT> interest{};

            float interestFood = 0.f;
            computeDanger(entity, head, pos.val, rot.angle, reg, ctx, slotDirs, danger);
            computeInterest(entity, head, pos.val, reg, ctx, slotDirs, interest, interestFood);
            applyMomentum(rot.angle, ai, danger, interest);

            float bestScore = -1e9f;
            int bestSlot = 0;
            for (int i = 0; i < SLOT_COUNT; ++i) {
                float score = interest[i] + danger[i];
                if (score > bestScore) {
                    bestScore = score;
                    bestSlot = i;
                }
            }

            ai.prevSlot = bestSlot;
            float bestAngle = bestSlot * 15.f;
            applySteer(head, bestAngle, ctx.time.dt);

            float angleDiff = std::abs(normalizeDeg(bestAngle - rot.angle));
            float baseSpeed = 220.f;
            desiredSpeed = baseSpeed * (1.f - angleDiff / 180.f);

            float frontDanger = (danger[0] + danger[23] + danger[1]) / 3.f;
            if (frontDanger < -50.f) {
                desiredSpeed *= 0.7f;
            }

            if (ai.level >= 2 && interestFood > 0.f) {
                desiredSpeed = 350.f;
            }

            ai.cachedFrontDanger = danger[0];
            ai.cachedFrontLeftDanger = danger[23];
            ai.cachedFrontRightDanger = danger[1];
        }

        void computeDanger(entt::entity self, SnakeHead& head, const sf::Vector2f& pos, float curAngle,
                           entt::registry& reg, GameContext& ctx,
                           const std::array<sf::Vector2f,24>& slotDirs,
                           std::array<float,24>& outDanger) {
            const float probeLen = 300 * 0.8f;
            auto headView = reg.view<Position, SnakeHead, Rotation>();

            for (int i = 0; i < 24; ++i) {
                float d = 0.f;
                sf::Vector2f dir = slotDirs[i];

                for (float factor : {0.5f, 1.0f}) {
                    sf::Vector2f sample = pos + dir * (probeLen * factor);

                    if (sample.x < 0.f || sample.y < 0.f ||
                        sample.x > ctx.window.mapSize.x || sample.y > ctx.window.mapSize.y) {
                        d -= 500.f;
                        continue;
                    }

                    if (!ctx.food.foodSystem->isAreaSafe(sample, head.currentRadius * 2.f)) {
                        float penalty = -200.f * std::exp(-factor);
                        d += penalty;
                    }
                }

                for (auto other : headView) {
                    if (other == self) continue;
                    const auto& oPos = headView.get<Position>(other).val;
                    const auto& oRot = headView.get<Rotation>(other).angle;
                    float oRad = oRot * 0.017453f;
                    sf::Vector2f oDir(std::cos(oRad), std::sin(oRad));
                    float predictT = 0.4f;
                    sf::Vector2f predictPos = oPos + oDir * 300.f * predictT;
                    float proj = (predictPos.x - pos.x) * dir.x + (predictPos.y - pos.y) * dir.y;
                    if (proj > 0.f && proj < probeLen) {
                        float lateral = std::abs((predictPos.x - pos.x) * dir.y - (predictPos.y - pos.y) * dir.x);
                        if (lateral < head.currentRadius * 2.f) {
                            d -= 150.f * std::exp(-proj / probeLen);
                        }
                    }
                }

                outDanger[i] = d;
            }

            std::array<float,24> smoothed = outDanger;
            for (int i = 0; i < 24; ++i) {
                int l = (i + 23) % 24;
                int r = (i + 1) % 24;
                smoothed[i] = (outDanger[l] + outDanger[i] * 2.f + outDanger[r]) / 4.f;
            }
            outDanger = smoothed;
        }

        void computeInterest(entt::entity entity, SnakeHead& head, const sf::Vector2f& pos,
                             entt::registry& reg, GameContext& ctx,
                             const std::array<sf::Vector2f,24>& slotDirs,
                             std::array<float,24>& outInterest,
                             float& outFoodScore) {
            outFoodScore = 0.f;
            for (float& v : outInterest) {
                v = ((rand() % 1000) / 1000.f - 0.5f) * 0.1f;
            }

            sf::Vector2f lootPos;
            if (ctx.food.foodSystem->findNearestLoot(pos, 500.f, lootPos)) {
                sf::Vector2f dir = lootPos - pos;
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len > 0.001f) dir /= len;
                for (int i = 0; i < 24; ++i) {
                    float dot = dir.x * slotDirs[i].x + dir.y * slotDirs[i].y;
                    if (dot > 0.f) {
                        outInterest[i] += dot * 1.5f;
                        outFoodScore = std::max(outFoodScore, dot);
                    }
                }
            }

            if (head.currentLength > 0) {
                auto playerView = reg.view<PlayerTag, Position, Rotation, SnakeHead>();
                if (playerView.begin() != playerView.end()) {
                    auto playerEntity = playerView.front();
                    const auto& pHead = reg.get<SnakeHead>(playerEntity);
                    const auto& pPos = reg.get<Position>(playerEntity).val;
                    float pRad = reg.get<Rotation>(playerEntity).angle * 0.017453f;
                    sf::Vector2f pDir(std::cos(pRad), std::sin(pRad));
                    sf::Vector2f side(-pDir.y, pDir.x);
                    float offset = (static_cast<int>(entity) % 2 == 0 ? 1.f : -1.f) * (pHead.currentRadius * 1.5f);
                    sf::Vector2f target = pPos + pDir * 100.f + side * offset;
                    sf::Vector2f dir = target - pos;
                    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                    if (len > 0.001f) dir /= len;
                    for (int i = 0; i < 24; ++i) {
                        float dot = dir.x * slotDirs[i].x + dir.y * slotDirs[i].y;
                        if (dot > 0.f) {
                            outInterest[i] += dot * 1.2f;
                        }
                    }
                }
            }
        }

        void applyMomentum(float currentAngle, AiTag& ai,
                           const std::array<float,24>& danger,
                           std::array<float,24>& interest) {
            int forwardSlot = static_cast<int>(std::round(normalizeDeg(currentAngle) / 15.f)) % 24;
            if (forwardSlot < 0) forwardSlot += 24;

            auto boost = [&](int slot, float weight){
                int s = (slot + 24) % 24;
                float decay = std::exp(danger[s] * 0.01f);
                interest[s] += weight * decay;
            };

            boost(forwardSlot, 0.3f);
            if (ai.prevSlot >= 0) {
                boost(ai.prevSlot, 0.4f);
            }
        }
    };
}
