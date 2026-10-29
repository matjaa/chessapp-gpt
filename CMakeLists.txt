cmake_minimum_required(VERSION 3.28)

project(Chess
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW TRUE
)

set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SFML)

add_executable(Chess
    src/main.cpp

    src/chess/Board.cpp
    src/chess/MoveValidator.cpp

    src/ai/ComputerPlayer.cpp

    src/graphics/Renderer.cpp
)

target_include_directories(Chess PRIVATE
    include
)

target_link_libraries(Chess PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)