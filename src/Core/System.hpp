#pragma once
#include "ResourceManager.h"
#include "Context.hpp"
#include "entt/entt.hpp"
#include <SFML/Graphics.hpp>

namespace Bocchi{
    class System {
    public:
        virtual ~System() = default;
        virtual void update(entt::registry& registry) = 0;
    };
}