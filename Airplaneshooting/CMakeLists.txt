cmake_minimum_required(VERSION 3.10)
project(AirplaneShooterSFML)
set(CMAKE_CXX_STANDARD 17)

find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)

# First executable (main.cpp)
add_executable(AirplaneShooterSFML main.cpp)
target_link_libraries(AirplaneShooterSFML
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)
