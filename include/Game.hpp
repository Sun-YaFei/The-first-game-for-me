#pragma once

#include <ftxui/component/event.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>
#include <random>
#include <string>
#include <vector>

namespace bfg {

enum class Direction { Left, Right };

enum class FishType { Sardine, Clownfish, Tuna, Shark, Whale };

struct Fish {
    float x = 0, y = 0;
    float speed = 0.5f;
    int size = 1;
    Direction dir = Direction::Right;
    FishType type = FishType::Sardine;
    bool alive = true;
    int wobblePhase = 0;
};

struct Particle {
    float x = 0, y = 0;
    int life = 0;
    std::string text;
    ftxui::Color color;
};

enum class GameState { Title, Playing, GameOver, Won };

class Game {
public:
    Game() = default;

    void Init(int canvasW, int canvasH);
    void Update();
    ftxui::Element Render() const;
    void OnKey(const ftxui::Event& event);

    bool IsGameOver() const { return state_ == GameState::GameOver; }
    bool HasWon() const { return state_ == GameState::Won; }
    int GetScore() const { return score_; }
    int GetPlayerSize() const { return playerSize_; }
    GameState GetState() const { return state_; }

private:
    int cw_ = 80;
    int ch_ = 24;

    GameState state_ = GameState::Title;
    int score_ = 0;
    int highScore_ = 0;
    int playerSize_ = 2;
    float px_ = 40, py_ = 12;
    Direction playerDir_ = Direction::Right;
    int tick_ = 0;
    int difficulty_ = 1;

    std::vector<Fish> fishes_;
    int maxFishes_ = 20;
    int spawnCooldown_ = 0;

    std::vector<Particle> particles_;

    mutable std::mt19937 rng_{std::random_device{}()};

    void SpawnFish();
    void MoveFishes();
    void CheckEating();
    void UpdateParticles();

    std::string FishArt(const Fish& f) const;
    std::string PlayerArt() const;
    ftxui::Color FishColor(const Fish& f) const;

    int GrowthThreshold(int size) const;
    FishType RandomFishType() const;
    int RandomSizeFor(FishType type) const;

    void DrawWater(ftxui::Canvas& c) const;
    void DrawFish(ftxui::Canvas& c, const Fish& f) const;
    void DrawPlayer(ftxui::Canvas& c) const;
    void DrawParticles(ftxui::Canvas& c) const;
    void DrawHUD(ftxui::Canvas& c) const;
    void DrawTitle(ftxui::Canvas& c) const;
    void DrawGameOver(ftxui::Canvas& c) const;
    void DrawWin(ftxui::Canvas& c) const;
};

} // namespace bfg
