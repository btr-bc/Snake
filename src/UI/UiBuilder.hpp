#pragma once
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include "UiComponents.hpp"

namespace Bocchi {

    class UiBuilder {
    public:
        explicit UiBuilder(entt::registry& registry)
            : m_reg(registry) {}

        UiBuilder& beginCanvas(const sf::FloatRect& region) {
            m_canvas = region;
            return *this;
        }

        UiBuilder& button(const std::string& name, sf::Vector2f size, sf::Vector2f pos, std::function<void()> onClick) {
            m_current = createEntity();
            auto& rect = m_reg.emplace<RectTransform>(m_current);
            rect.size = size;
            rect.position = pos;
            m_reg.emplace<UIElement>(m_current);
            auto& inter = m_reg.emplace<Interaction>(m_current);
            inter.onClick = std::move(onClick);
            m_reg.emplace<Appearance>(m_current);
            if (!name.empty()) {
                auto& txt = m_reg.emplace<UIText>(m_current);
                txt.text.setString(name);
            }
            return *this;
        }

        UiBuilder& text(const std::string& content, unsigned fontSize, sf::Color color = sf::Color::White) {
            ensureCurrent();
            auto& txt = m_reg.get_or_emplace<UIText>(m_current);
            txt.text.setString(content);
            txt.text.setCharacterSize(fontSize);
            txt.text.setFillColor(color);
            return *this;
        }

        UiBuilder& image(const sf::Texture& tex, const sf::IntRect& rect = sf::IntRect()) {
            ensureCurrent();
            auto& app = m_reg.get_or_emplace<Appearance>(m_current);
            app.sprite.setTexture(tex, true);
            if (rect != sf::IntRect()) app.sprite.setTextureRect(rect);
            app.sprite.setColor(app.color);
            return *this;
        }

    private:
        entt::entity createEntity() {
            return m_reg.create();
        }

        void ensureCurrent() {
            if (m_current == entt::null) {
                m_current = createEntity();
                m_reg.emplace<RectTransform>(m_current);
                m_reg.emplace<UIElement>(m_current);
            }
        }

        entt::registry& m_reg;
        entt::entity m_current{entt::null};
        sf::FloatRect m_canvas{};
    };

} // namespace Bocchi
