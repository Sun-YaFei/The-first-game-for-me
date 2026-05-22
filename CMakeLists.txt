cmake_minimum_required(VERSION 3.16)
project(BigFishGame VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── FTXUI via FetchContent ──────────────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG        v5.0.0
)
FetchContent_MakeAvailable(ftxui)

# ── Sources ─────────────────────────────────────────────────────────────
add_executable(big_fish_game
    src/main.cpp
    src/Game.cpp
)

target_include_directories(big_fish_game PRIVATE include)

target_link_libraries(big_fish_game PRIVATE
    ftxui::dom
    ftxui::component
    ftxui::screen
)

# Platform: link pthread on Unix
if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(big_fish_game PRIVATE Threads::Threads)
endif()

# Windows: set console subsystem
if(WIN32)
    set_target_properties(big_fish_game PROPERTIES
        WIN32_EXECUTABLE OFF
    )
endif()
