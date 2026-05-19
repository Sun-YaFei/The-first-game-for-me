#include "Game.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <atomic>
#include <chrono>
#include <thread>

int main() {
  using namespace ftxui;
  using namespace std::chrono_literals;

  Game game;

  auto screen = ScreenInteractive::Fullscreen();

  std::atomic<bool> running = true;

  auto renderer = Renderer([&] {
    return game.Render();
  });

  auto component = CatchEvent(renderer, [&](Event event) {
    if (event == Event::Custom) {
      game.Tick();
      return true;
    }

    if (event == Event::ArrowUp || event == Event::Character('w') ||
        event == Event::Character('W')) {
      game.MovePlayer(0, -1);
      return true;
    }

    if (event == Event::ArrowDown || event == Event::Character('s') ||
        event == Event::Character('S')) {
      game.MovePlayer(0, 1);
      return true;
    }

    if (event == Event::ArrowLeft || event == Event::Character('a') ||
        event == Event::Character('A')) {
      game.MovePlayer(-1, 0);
      return true;
    }

    if (event == Event::ArrowRight || event == Event::Character('d') ||
        event == Event::Character('D')) {
      game.MovePlayer(1, 0);
      return true;
    }

    if (event == Event::Character(' ')) {
      game.TogglePause();
      return true;
    }

    if (event == Event::Character('r') || event == Event::Character('R')) {
      game.Reset();
      return true;
    }

    if (event == Event::Character('q') || event == Event::Character('Q') ||
        event == Event::Escape) {
      running = false;
      screen.ExitLoopClosure()();
      return true;
    }

    return false;
  });

  std::thread ticker([&] {
    while (running) {
      std::this_thread::sleep_for(120ms);
      screen.PostEvent(Event::Custom);
    }
  });

  screen.Loop(component);

  running = false;

  if (ticker.joinable()) {
    ticker.join();
  }

  return 0;
}