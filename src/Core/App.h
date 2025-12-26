#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "World.hpp"
#include "Context.hpp"
#include "Game/Builders/EntityBuilder.hpp"

namespace Bocchi {

    class App;
    class ResourceManager;

    class App {
    public:
        App();
        ~App();

        void run();
        void requestChangeWorld(WorldType type);

    private:
        void init();
        void quit();
        void update();
        void changeWorld(WorldType type);

        bool m_isRunning;
        WorldType m_targetWorld = WorldType::Empty;

        std::unique_ptr<sf::RenderWindow> m_window;
        std::unique_ptr<sf::View> m_worldView;
        std::unique_ptr<sf::View> m_uiView;
        std::unique_ptr<ResourceManager> m_res;
        std::unique_ptr<World> m_currentWorld;
        std::unique_ptr<EntityBuilder> m_builder;

        GameContext m_sharedContext;
    };
}
