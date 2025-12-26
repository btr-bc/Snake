#include "ResourceManager.h"
#include <iostream>
#include <filesystem>

namespace Bocchi {

    void ResourceManager::loadAll(){
        // 工作路径检测
        // std::cout << "Current Working Directory: " << std::filesystem::current_path() << std::endl;  
        // if (std::filesystem::exists("assets")) {
        //     std::cout << "Success: 'assets' folder found!" << std::endl;
        // } else {
        //     std::cout << "Error: 'assets' folder NOT found in current directory." << std::endl;
        // }
        add<sf::Texture>(ResID::head_maodie, "assets/textures/head_maodie.png");
        add<sf::Texture>(ResID::head_maodie_o, "assets/textures/head_maodie_o.png");
        add<sf::Texture>(ResID::head_shantianliang, "assets/textures/head_shantianliang.png");
        add<sf::Texture>(ResID::head_maodie_h, "assets/textures/head_maodie_h.png");
        add<sf::Texture>(ResID::head_xiduoyudai, "assets/textures/head_xiduoyudai.jpg");
        

        add<sf::Texture>(ResID::body_maodie, "assets/textures/body_maodie.png");
        add<sf::Texture>(ResID::body_shantianliang, "assets/textures/body_shantianliang.png");

        add<sf::Texture>(ResID::food_huotuichang, "assets/textures/food_huotuichang.png");
        add<sf::Texture>(ResID::food_pingguohe, "assets/textures/food_pingguohe.png");
        add<sf::Texture>(ResID::food_bocchi, "assets/textures/food_bocchi.png");

        add<sf::SoundBuffer>(ResID::eat_sound_maodie, "assets/sounds/eat_sound_maodie.mp3");
        add<sf::SoundBuffer>(ResID::eat_sound_maodie_h, "assets/sounds/eat_sound_maodie_h.wav");
    }

    void ResourceManager::unloadAll(){
        // 清理
        m_textures.clear();
        m_fonts.clear();
        m_soundBuffers.clear();
    }
 
}