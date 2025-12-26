#include "App.h"
#include "ResourceManager.h"
#include "Config.h"
#include "Game/Worlds/TestWorld.h"
#include <SFML/Window.hpp>

namespace Bocchi {

    namespace {
        std::unique_ptr<World> createWorld(WorldType type) {
            switch(type) {
                case WorldType::MainMenu:
                    // TODO: hook real main menu world
                    return std::make_unique<TestWorld>();
                case WorldType::ClassicMode:
                    return std::make_unique<TestWorld>();
                default:
                    return std::make_unique<TestWorld>();
            }
        }
    }

    App::App() : m_isRunning(false) {}
    App::~App() = default;

    void App::init() {
        const auto& config = Config::getInstance();
        m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(config.windowWidth, config.windowHeight), config.windowTitle);
        m_window->setFramerateLimit(60);
        m_res = std::make_unique<ResourceManager>();
        m_builder = std::make_unique<EntityBuilder>();
        m_worldView = std::make_unique<sf::View>(sf::FloatRect(0, 0, config.windowWidth, config.windowHeight));
        m_uiView = std::make_unique<sf::View>(sf::FloatRect(0, 0, config.windowWidth, config.windowHeight));
        m_worldView->setCenter(config.windowWidth / 2.f, config.windowHeight / 2.f);
        m_uiView->setCenter(config.windowWidth / 2.f, config.windowHeight / 2.f);
        
        m_sharedContext = {};
        m_sharedContext.services.app = this;
        m_sharedContext.services.res = m_res.get();
        m_sharedContext.services.builder = m_builder.get();
        m_sharedContext.window.window = m_window.get();
        m_sharedContext.window.worldView = m_worldView.get();
        m_sharedContext.window.uiView = m_uiView.get();
        m_sharedContext.window.windowSize = sf::Vector2f(config.windowWidth, config.windowHeight);
        m_sharedContext.window.mapSize = sf::Vector2f(config.mapWidth, config.mapHeight);
        m_sharedContext.window.cameraPos = sf::Vector2f(config.windowWidth / 2.f, config.windowHeight / 2.f);

        m_res->loadAll();
        // 窗口图标
        // sf::Image icon;
        // if (icon.loadFromFile("assets/textures/head_maodie.png")) {
        //     m_window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
        // }

        m_currentWorld = createWorld(WorldType::ClassicMode);
        m_currentWorld->init(m_sharedContext);

        m_isRunning = true;
    }

    void App::run() {
        init();
        sf::Clock clock;
        while (m_isRunning && m_window->isOpen()) {
            m_sharedContext.time.dt = clock.restart().asSeconds();
            m_sharedContext.time.frameCount++;
            update();
        }
        quit();
    }

    void App::update() {
        auto& ctx = m_currentWorld->context();
        ctx.window.window = m_window.get();
        ctx.window.worldView = m_worldView.get();
        ctx.window.uiView = m_uiView.get();
        ctx.services.res = m_res.get();
        ctx.services.builder = m_builder.get();
        ctx.services.app = this;
        ctx.time.dt = m_sharedContext.time.dt;
        ctx.time.frameCount = m_sharedContext.time.frameCount;
        m_window->clear(sf::Color::Black);

        sf::Event event;
        while (m_window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                m_isRunning = false;
            }
            if (event.type == sf::Event::Resized) {
                float w = static_cast<float>(event.size.width);
                float h = static_cast<float>(event.size.height);          
                m_worldView->setSize(w, h);
                m_uiView->setSize(w, h);
                m_uiView->setCenter(w / 2.f, h / 2.f);
                ctx.window.windowSize = sf::Vector2f(w, h);
            }
        }

        if (m_currentWorld) m_currentWorld->update();

        if (m_targetWorld != WorldType::Empty) {
            changeWorld(m_targetWorld);
            m_targetWorld = WorldType::Empty;
        }

        m_window->display();
    }

    void App::changeWorld(WorldType type) {
        GameContext nextCtx = m_currentWorld ? m_currentWorld->context() : m_sharedContext;

        nextCtx.food.foodSystem = nullptr;
        if (m_currentWorld) m_currentWorld->quit();
        
        m_currentWorld = createWorld(type);
        m_currentWorld->init(nextCtx);
    }

    void App::requestChangeWorld(WorldType type) {
        m_targetWorld = type;
    }

    void App::quit() {
        if (m_currentWorld) m_currentWorld->quit();
        if (m_res) m_res->unloadAll();
        if (m_window) m_window->close();
    }
}
