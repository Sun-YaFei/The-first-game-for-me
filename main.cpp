#include "Game.hpp"

#include <atomic>
#include <chrono>
#include <thread>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>

int main() {
    using namespace ftxui;
    using namespace std::chrono_literals;

    auto dims = Terminal::Size();
    int cw = dims.dimx > 10 ? dims.dimx : 80;
    int ch = dims.dimy > 5  ? dims.dimy : 24;

    bfg::Game game;
    game.Init(cw, ch);

    auto screen = ScreenInteractive::Fullscreen();

    std::atomic<bool> running{true};

    // Game tick thread at ~12 FPS
    std::thread timer([&] {
        while (running) {
            std::this_thread::sleep_for(80ms);
            screen.Post(Event::Custom);
        }
    });

    auto quit = screen.ExitLoopClosure();

    auto renderer = Renderer([&] {
        return game.Render();
    });

    auto component = CatchEvent(renderer, [&](Event event) -> bool {
        // Quit: Q works everywhere
        if (event == Event::Character('q') || event == Event::Character('Q')) {
            quit();
            return true;
        }

        // Game tick
        if (event == Event::Custom) {
            game.Update();
            return true;
        }

        game.OnKey(event);
        return false;
    });

    screen.Loop(component);

    running = false;
    timer.join();

    return 0;
}
