#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <memory>
#include <string>
#include <stdexcept>

namespace Bocchi {

    enum class ResID {
        NONE,
        head_maodie,
        head_maodie_o,
        head_maodie_h,
        head_shantianliang,
        head_xiduoyudai,

        body_maodie,
        body_shantianliang,
        
        food_huotuichang,
        food_pingguohe,
        food_bocchi,

        eat_sound_maodie,
        eat_sound_maodie_h,
 
    };

    class ResourceManager {
    public:
        ResourceManager() = default;
        

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        void loadAll();
        void unloadAll();


        template <typename T>
        void add(ResID id, const std::string& path);

        template <typename T>
        T& get(ResID id);

    private:
        std::unordered_map<ResID, std::unique_ptr<sf::Texture>>     m_textures;
        std::unordered_map<ResID, std::unique_ptr<sf::Font>>        m_fonts;
        std::unordered_map<ResID, std::unique_ptr<sf::SoundBuffer>> m_soundBuffers;
    };

    // 图像特化
    template <>
    inline void ResourceManager::add<sf::Texture>(ResID id, const std::string& path) {
        auto tex = std::make_unique<sf::Texture>();
        if (tex->loadFromFile(path)) {
            m_textures[id] = std::move(tex);
        }
    }

    template <>
    inline sf::Texture& ResourceManager::get<sf::Texture>(ResID id) {
        return *m_textures.at(id);
    }

    // 字体特化
    template <>
    inline void ResourceManager::add<sf::Font>(ResID id, const std::string& path) {
        auto font = std::make_unique<sf::Font>();
        if (font->loadFromFile(path)) {
            m_fonts[id] = std::move(font);
        }
    }

    template <>
    inline sf::Font& ResourceManager::get<sf::Font>(ResID id) {
        return *m_fonts.at(id);
    }

    // 音效特化
    template <>
    inline void ResourceManager::add<sf::SoundBuffer>(ResID id, const std::string& path) {
        auto sb = std::make_unique<sf::SoundBuffer>();
        if (sb->loadFromFile(path)) {
            m_soundBuffers[id] = std::move(sb);
        }
    }

    template <>
    inline sf::SoundBuffer& ResourceManager::get<sf::SoundBuffer>(ResID id) {
        return *m_soundBuffers.at(id);
    }
}