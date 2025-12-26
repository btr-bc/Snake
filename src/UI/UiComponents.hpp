#pragma once
#include <SFML/Graphics.hpp>
#include <functional>

namespace Bocchi {

    enum class Anchor {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Center
    };

    struct RectTransform {
        sf::Vector2f position{};     // local position
        sf::Vector2f size{};         // width/height
        Anchor anchor = Anchor::TopLeft;
        sf::Vector2f pivot{0.5f, 0.5f}; // 0..1
        sf::FloatRect worldRect{};   // cached world rect (computed by system)
    };

    struct UIElement {
        int zIndex = 0;
        bool isVisible = true;
    };

    enum class InteractionState {
        Idle,
        Hover,
        Pressed
    };

    struct Interaction {
        InteractionState state = InteractionState::Idle;
        std::function<void()> onClick;
        bool wasPressed = false;
    };

    struct Appearance {
        sf::Color color = sf::Color::White;
        sf::IntRect textureRect{};
        sf::Sprite sprite;
    };

    struct UIText {
        sf::Text text;
    };

} // namespace Bocchi
