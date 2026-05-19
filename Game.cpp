#include "Game.hpp"

#include <algorithm>
#include <cmath>
#include <string>

using namespace ftxui;

namespace {
int Sign(int value) {
  if (value > 0) {
    return 1;
  }
  if (value < 0) {
    return -1;
  }
  return 0;
}

bool SamePosition(const Point& a, const Point& b) {
  return a.x == b.x && a.y == b.y;
}
}

Game::Game(int width, int height)
    : width_(width),
      height_(height),
      player_size_(2),
      score_(0),
      eaten_count_(0),
      tick_count_(0),
      state_(GameState::Playing),
      rng_(std::random_device{}()) {
  Reset();
}

void Game::Reset() {
  player_ = {width_ / 2, height_ / 2};
  player_size_ = 2;
  score_ = 0;
  eaten_count_ = 0;
  tick_count_ = 0;
  state_ = GameState::Playing;
  fishes_.clear();

  for (int i = 0; i < 14; ++i) {
    fishes_.push_back(CreateFish());
  }
}

Fish Game::CreateFish() {
  Fish fish;

  std::uniform_int_distribution<int> chance(1, 100);
  std::uniform_int_distribution<int> small_size(1, std::max(1, player_size_));
  std::uniform_int_distribution<int> big_size(player_size_ + 1, player_size_ + 3);

  if (chance(rng_) <= 35) {
    fish.size = std::min(9, big_size(rng_));
  } else {
    fish.size = std::max(1, small_size(rng_));
  }

  std::uniform_int_distribution<int> side_dist(0, 3);
  std::uniform_int_distribution<int> x_dist(0, width_ - 1);
  std::uniform_int_distribution<int> y_dist(0, height_ - 1);
  int side = side_dist(rng_);

  if (side == 0) {
    fish.position = {0, y_dist(rng_)};
    fish.dx = 1;
    fish.dy = 0;
  } else if (side == 1) {
    fish.position = {width_ - 1, y_dist(rng_)};
    fish.dx = -1;
    fish.dy = 0;
  } else if (side == 2) {
    fish.position = {x_dist(rng_), 0};
    fish.dx = 0;
    fish.dy = 1;
  } else {
    fish.position = {x_dist(rng_), height_ - 1};
    fish.dx = 0;
    fish.dy = -1;
  }

  std::uniform_int_distribution<int> think_dist(1, 4);
  fish.think_delay = think_dist(rng_);

  return fish;
}

void Game::MovePlayer(int dx, int dy) {
  if (state_ != GameState::Playing) {
    return;
  }

  player_.x = std::clamp(player_.x + dx, 0, width_ - 1);
  player_.y = std::clamp(player_.y + dy, 0, height_ - 1);

  ResolveCollisions();
}

void Game::TogglePause() {
  if (state_ == GameState::GameOver) {
    return;
  }

  if (state_ == GameState::Playing) {
    state_ = GameState::Paused;
  } else {
    state_ = GameState::Playing;
  }
}

void Game::Tick() {
  if (state_ != GameState::Playing) {
    return;
  }

  ++tick_count_;

  for (auto& fish : fishes_) {
    UpdateFishAI(fish);

    fish.position.x += fish.dx;
    fish.position.y += fish.dy;

    bool outside = fish.position.x < 0 || fish.position.x >= width_ ||
                   fish.position.y < 0 || fish.position.y >= height_;

    if (outside) {
      fish = CreateFish();
    }
  }

  if (tick_count_ % 15 == 0 && fishes_.size() < 24) {
    fishes_.push_back(CreateFish());
  }

  ResolveCollisions();
}

void Game::UpdateFishAI(Fish& fish) {
  --fish.think_delay;

  if (fish.think_delay > 0) {
    return;
  }

  std::uniform_int_distribution<int> delay_dist(2, 5);
  fish.think_delay = delay_dist(rng_);

  int diff_x = player_.x - fish.position.x;
  int diff_y = player_.y - fish.position.y;
  int distance = std::abs(diff_x) + std::abs(diff_y);

  bool fish_is_bigger = fish.size > player_size_;
  int ai_range = fish_is_bigger ? 12 : 8;

  if (distance <= ai_range) {
    int move_x = Sign(diff_x);
    int move_y = Sign(diff_y);

    if (!fish_is_bigger) {
      move_x = -move_x;
      move_y = -move_y;
    }

    if (std::abs(diff_x) >= std::abs(diff_y)) {
      fish.dx = move_x;
      fish.dy = 0;
    } else {
      fish.dx = 0;
      fish.dy = move_y;
    }
  } else {
    std::uniform_int_distribution<int> dir_dist(0, 3);
    int dir = dir_dist(rng_);

    if (dir == 0) {
      fish.dx = 1;
      fish.dy = 0;
    } else if (dir == 1) {
      fish.dx = -1;
      fish.dy = 0;
    } else if (dir == 2) {
      fish.dx = 0;
      fish.dy = 1;
    } else {
      fish.dx = 0;
      fish.dy = -1;
    }
  }

  if (fish.dx == 0 && fish.dy == 0) {
    fish.dx = 1;
  }
}

void Game::ResolveCollisions() {
  for (auto& fish : fishes_) {
    if (!SamePosition(player_, fish.position)) {
      continue;
    }

    if (fish.size <= player_size_) {
      score_ += fish.size * 10;
      ++eaten_count_;

      if (eaten_count_ % 3 == 0) {
        player_size_ = std::min(9, player_size_ + 1);
      }

      fish = CreateFish();
    } else {
      state_ = GameState::GameOver;
    }
  }
}

const Fish* Game::FishAt(int x, int y) const {
  const Fish* best = nullptr;

  for (const auto& fish : fishes_) {
    if (fish.position.x != x || fish.position.y != y) {
      continue;
    }

    if (best == nullptr || fish.size > best->size) {
      best = &fish;
    }
  }

  return best;
}

Element Game::RenderCell(int x, int y) const {
  if (player_.x == x && player_.y == y) {
    return text("@") | color(Color::YellowLight) | bold;
  }

  const Fish* fish = FishAt(x, y);

  if (fish == nullptr) {
    return text(" ");
  }

  std::string icon;

  if (fish->size <= 2) {
    icon = ".";
  } else if (fish->size <= 4) {
    icon = "o";
  } else if (fish->size <= 6) {
    icon = "O";
  } else {
    icon = "#";
  }

  if (fish->size > player_size_) {
    return text(icon) | color(Color::RedLight) | bold;
  }

  if (fish->size == player_size_) {
    return text(icon) | color(Color::GreenLight) | bold;
  }

  return text(icon) | color(Color::CyanLight);
}

Element Game::Render() const {
  Elements rows;

  for (int y = 0; y < height_; ++y) {
    Elements cells;

    for (int x = 0; x < width_; ++x) {
      cells.push_back(RenderCell(x, y));
    }

    rows.push_back(hbox(cells));
  }

  Element board = vbox(rows) | border | color(Color::BlueLight);

  std::string info = "分数: " + std::to_string(score_) +
                     "   体型: " + std::to_string(player_size_) +
                     "   已吃: " + std::to_string(eaten_count_);

  Element state_text = text("进行中") | color(Color::GreenLight) | bold;

  if (state_ == GameState::Paused) {
    state_text = text("暂停") | color(Color::YellowLight) | bold;
  }

  if (state_ == GameState::GameOver) {
    state_text = text("游戏结束") | color(Color::RedLight) | bold;
  }

  Elements page;

  page.push_back(text(" 大鱼吃小鱼 / Big Fish Eat Small Fish ") |
                 bold | hcenter | color(Color::CyanLight));

  page.push_back(separator());
  page.push_back(hbox({text(info), filler(), state_text}));
  page.push_back(board);
  page.push_back(separator());

  page.push_back(text("操作：WASD/方向键移动 | 空格暂停 | R重新开始 | Q退出") | dim);

  page.push_back(text("规则：吃掉体型不大于自己的鱼，碰到更大的鱼会失败。红色鱼危险，青色/绿色鱼可以吃。") | dim);

  if (state_ == GameState::Paused) {
    page.push_back(text("已暂停：按空格继续游戏。") |
                   hcenter | color(Color::YellowLight));
  }

  if (state_ == GameState::GameOver) {
    page.push_back(text("你被更大的鱼吃掉了！按 R 重新开始，按 Q 退出。") |
                   hcenter | color(Color::RedLight) | bold);
  }

  return vbox(page) | border | bgcolor(Color::Black);
}