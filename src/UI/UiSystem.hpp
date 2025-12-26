#pragma once
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include <algorithm>
#include "UiComponents.hpp"

namespace Bocchi {

    class UiSystem {
    public:
        void update(entt::registry& reg, const sf::Vector2f& mousePos, bool clickDown) {
            auto view = reg.view<RectTransform, Interaction, UIElement>();
            for (auto entity : view) {
                auto& rect = view.get<RectTransform>(entity);
                auto& inter = view.get<Interaction>(entity);
                const auto& ui = view.get<UIElement>(entity);
                if (!ui.isVisible) continue;

                sf::FloatRect aabb = computeWorldRect(rect);
                rect.worldRect = aabb;
                bool inside = aabb.contains(mousePos);

                if (inside && clickDown && !inter.wasPressed) {
                    inter.state = InteractionState::Pressed;
                    inter.wasPressed = true;
                } else if (inside && !clickDown && inter.wasPressed) {
                    inter.wasPressed = false;
                    inter.state = InteractionState::Hover;
                    if (inter.onClick) inter.onClick();
                } else if (inside) {
                    inter.state = InteractionState::Hover;
                } else {
                    inter.state = InteractionState::Idle;
                    inter.wasPressed = clickDown && inter.wasPressed;
                }
            }
        }

        void render(entt::registry& reg, sf::RenderWindow& window, sf::View& uiView) {
            window.setView(uiView);
            auto view = reg.view<UIElement, RectTransform>();
            std::vector<entt::entity> sorted(view.begin(), view.end());
            std::sort(sorted.begin(), sorted.end(), [&](entt::entity a, entt::entity b){
                return view.get<UIElement>(a).zIndex < view.get<UIElement>(b).zIndex;
            });

            for (auto entity : sorted) {
                const auto& ui = view.get<UIElement>(entity);
                if (!ui.isVisible) continue;
                const auto& rect = view.get<RectTransform>(entity);
                if (auto* app = reg.try_get<Appearance>(entity)) {
                    drawAppearance(window, *app, rect);
                }
                if (auto* txt = reg.try_get<UIText>(entity)) {
                    drawText(window, *txt, rect);
                }
            }
        }

    private:
        sf::FloatRect computeWorldRect(const RectTransform& rect) const {
            sf::Vector2f pos = rect.position;
            sf::Vector2f size = rect.size;
            sf::Vector2f origin = { size.x * rect.pivot.x, size.y * rect.pivot.y };
            return { pos - origin, size };
        }

        void drawAppearance(sf::RenderWindow& window, Appearance& app, const RectTransform& rect) {
            if (app.sprite.getTexture()) {
                sf::Sprite spr = app.sprite;
                auto bounds = spr.getGlobalBounds();
                float sx = rect.size.x / bounds.width;
                float sy = rect.size.y / bounds.height;
                spr.setScale(sx, sy);
                spr.setPosition(rect.worldRect.left, rect.worldRect.top);
                window.draw(spr);
            } else {
                sf::RectangleShape shape(rect.size);
                shape.setPosition(rect.worldRect.left, rect.worldRect.top);
                shape.setFillColor(app.color);
                window.draw(shape);
            }
        }

        void drawText(sf::RenderWindow& window, UIText& txt, const RectTransform& rect) {
            auto bounds = txt.text.getLocalBounds();
            sf::Vector2f pos(rect.worldRect.left + rect.size.x * 0.5f - bounds.width * 0.5f, rect.worldRect.top + rect.size.y * 0.5f - bounds.height);
            txt.text.setPosition(pos);
            window.draw(txt.text);
        }
    };

} // namespace Bocchi
