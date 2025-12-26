#pragma once
#include "Context.hpp"
#include "System.hpp"
#include <entt/entt.hpp>
#include <memory>
#include <type_traits>
#include <vector>

namespace Bocchi {

    enum class WorldType {
        Empty,
        MainMenu,
        ClassicMode
    };

    class World {
    public:
        virtual ~World() = default;

        virtual void init(const GameContext& ctx) = 0;
        virtual void quit() = 0;

        void update() {
            // 按注册顺序执行
            for (auto& system : m_systems) {
                system->update(m_registry);
            }
        }

        // 添加注册系统
        template<typename T, typename... Args>
        void addSystem(Args&&... args) {
            static_assert(std::is_base_of_v<System, T>, "T must derive from System");
            m_systems.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        }

        entt::registry& registry() {return m_registry; }
        GameContext& context() {return m_registry.ctx().get<GameContext>();}
        const GameContext& context() const{ return m_registry.ctx().get<GameContext>(); }

    protected:
        entt::registry m_registry;
        std::vector<std::unique_ptr<System>> m_systems;
    };
}
