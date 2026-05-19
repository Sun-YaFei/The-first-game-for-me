#pragma once

#include <ftxui/dom/elements.hpp>

#include <random>
#include <vector>

struct Point {
  int x = 0;
  int y = 0;
};

struct Fish {
  Point position;
  int size = 1;
  int dx = 1;
  int dy = 0;
  int think_delay = 0;
};

enum class GameState {
  Playing,
  Paused,
  GameOver,
};

class Game {
 public:
  Game(int width = 42, int height = 18);

  void Reset();
  void Tick();
  void MovePlayer(int dx, int dy);
  void TogglePause();

  ftxui::Element Render() const;

 private:
  int width_;
  int height_;
  Point player_;
  int player_size_;
  int score_;
  int eaten_count_;
  int tick_count_;
  GameState state_;
  std::vector<Fish> fishes_;
  std::mt19937 rng_;

  Fish CreateFish();
  void UpdateFishAI(Fish& fish);
  void ResolveCollisions();
  const Fish* FishAt(int x, int y) const;
  ftxui::Element RenderCell(int x, int y) const;
};