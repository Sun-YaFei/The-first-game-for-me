cmake_minimum_required(VERSION 3.20)

project(BigFishEatSmallFish VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v6.1.9
)

FetchContent_MakeAvailable(ftxui)

add_executable(big_fish
  src/main.cpp
  src/Game.cpp
)

target_include_directories(big_fish PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(big_fish PRIVATE
  ftxui::screen
  ftxui::dom
  ftxui::component
)