#pragma once
#include <algorithm>
#include <cmath>
#include "Core/System.hpp"
#include "Core/Component.hpp"
#include "Core/Context.hpp"

namespace Bocchi {

    class CameraSystem : public System {
    public:
        void update(entt::registry& reg) override {
            auto& ctx = reg.ctx().get<GameContext>();
            if (ctx.state.isPaused || !ctx.window.worldView || !ctx.window.uiView) return;

            auto view = reg.view<Position, SnakeHead, PlayerTag>();
            auto it = view.begin();
            if (it == view.end()) return;

            auto playerEntity = *it;
            const auto& posComponent = view.get<Position>(playerEntity);
            const auto& head = view.get<SnakeHead>(playerEntity);
            
            float radiusBonus = std::max(0.0f, head.currentRadius - 20.0f);
            float targetZoom = 1.0f + std::sqrt(radiusBonus) * 0.12f;
            targetZoom = std::min(targetZoom, 4.0f);

            sf::Vector2f baseSize = ctx.window.uiView->getSize();
            sf::Vector2f targetSize = baseSize * targetZoom;

            if (head.spawnProtectionTime > 0.0f) {
                ctx.window.worldView->setSize(targetSize);
                ctx.window.cameraPos = posComponent.val;
            } 
            else {
                sf::Vector2f currentSize = ctx.window.worldView->getSize();
                float zoomSmoothness = 1.5f;
                ctx.window.worldView->setSize(
                    currentSize.x + (targetSize.x - currentSize.x) * zoomSmoothness * ctx.time.dt,
                    currentSize.y + (targetSize.y - currentSize.y) * zoomSmoothness * ctx.time.dt
                );

                float moveSmoothness = 5.0f;
                ctx.window.cameraPos.x += (posComponent.val.x - ctx.window.cameraPos.x) * moveSmoothness * ctx.time.dt;
                ctx.window.cameraPos.y += (posComponent.val.y - ctx.window.cameraPos.y) * moveSmoothness * ctx.time.dt;
            }
            
            ctx.window.worldView->setCenter(ctx.window.cameraPos);
        }

        void handleResize(GameContext& ctx, float width, float height) {
            if (ctx.window.worldView && ctx.window.uiView) {
                ctx.window.uiView->setSize(width, height);
                ctx.window.uiView->setCenter(width / 2.0f, height / 2.0f);
            }
        }
    };
}
