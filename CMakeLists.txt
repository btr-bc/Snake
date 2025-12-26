cmake_minimum_required(VERSION 3.15)
project(SnakeGame VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


# 自动包含源码
file(GLOB_RECURSE SOURCES "src/*.cpp")

# SFML 路径配置
set(SFML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rd/SFML/lib/cmake/SFML")
find_package(SFML 2.6 COMPONENTS graphics window system audio REQUIRED)

# 生成可执行文件
add_executable(SnakeGame ${SOURCES})
target_include_directories(SnakeGame PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/3rd/entt/src)

# 链接库
target_link_libraries(SnakeGame PRIVATE
    sfml-graphics
    sfml-window
    sfml-audio
    sfml-system
)

# Windows 下自动拷贝 DLL
if(WIN32)
    add_custom_command(TARGET SnakeGame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/3rd/SFML/bin"
        $<TARGET_FILE_DIR:SnakeGame>
        COMMENT "Copying SFML DLLs to execution directory..."
    )
endif()

# 自动拷贝 assets 文件夹到可执行文件所在目录
add_custom_command(TARGET SnakeGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:SnakeGame>/assets"
    COMMENT "Copying assets to execution directory..."
)