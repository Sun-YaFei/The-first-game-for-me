#include "Game.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace bfg {

// ── helpers ──────────────────────────────────────────────────────────────────

static const int kGrowthThresholds[] = {0, 10, 25, 50, 80, 120, 180, 250, 350, 500, 700};

int Game::GrowthThreshold(int size) const {
    if (size < 0) return 0;
    if (size >= 10) return kGrowthThresholds[10];
    return kGrowthThresholds[size];
}

FishType Game::RandomFishType() const {
    int roll = std::uniform_int_distribution<>(1, 100)(rng_);
    if (roll <= 45) return FishType::Sardine;
    if (roll <= 75) return FishType::Clownfish;
    if (roll <= 92) return FishType::Tuna;
    if (roll <= 99) return FishType::Shark;
    return FishType::Whale;
}

int Game::RandomSizeFor(FishType type) const {
    switch (type) {
    case FishType::Sardine:  return std::uniform_int_distribution<>(1, 3)(rng_);
    case FishType::Clownfish: return std::uniform_int_distribution<>(2, 4)(rng_);
    case FishType::Tuna:     return std::uniform_int_distribution<>(4, 6)(rng_);
    case FishType::Shark:    return std::uniform_int_distribution<>(6, 8)(rng_);
    case FishType::Whale:    return std::uniform_int_distribution<>(8, 10)(rng_);
    }
    return 1;
}

static float typeBaseSpeed(FishType t) {
    switch (t) {
    case FishType::Sardine:  return 0.7f;
    case FishType::Clownfish: return 0.55f;
    case FishType::Tuna:     return 0.45f;
    case FishType::Shark:    return 0.55f;
    case FishType::Whale:    return 0.25f;
    }
    return 0.5f;
}

// ── art ──────────────────────────────────────────────────────────────────────

std::string Game::FishArt(const Fish& f) const {
    int len = f.size;
    if (len < 1) len = 1;
    if (len > 10) len = 10;
    std::string tail(len, '~');
    if (f.dir == Direction::Right)
        return tail + ">";
    else
        return "<" + tail;
}

std::string Game::PlayerArt() const {
    int len = playerSize_;
    if (len < 1) len = 1;
    if (len > 10) len = 10;
    std::string body(len - 1, (len >= 5 ? '#' : '='));
    if (playerDir_ == Direction::Right)
        return body + "[#]>";
    else
        return "<[#]" + body;
}

ftxui::Color Game::FishColor(const Fish& f) const {
    if (!f.alive) return ftxui::Color::Grey30;
    if (f.size < playerSize_) return ftxui::Color::Green;
    else                        return ftxui::Color::Red;
}

// ── init ─────────────────────────────────────────────────────────────────────

void Game::Init(int canvasW, int canvasH) {
    cw_ = canvasW;
    ch_ = canvasH;
    state_ = GameState::Title;
    score_ = 0;
    playerSize_ = 2;
    px_ = static_cast<float>(cw_) / 2.0f;
    py_ = static_cast<float>(ch_) / 2.0f;
    playerDir_ = Direction::Right;
    tick_ = 0;
    difficulty_ = 1;
    spawnCooldown_ = 30;
    fishes_.clear();
    particles_.clear();
}

// ── spawning ─────────────────────────────────────────────────────────────────

void Game::SpawnFish() {
    if (static_cast<int>(fishes_.size()) >= maxFishes_) return;

    Fish f;
    f.type = RandomFishType();
    f.size = RandomSizeFor(f.type);
    f.speed = typeBaseSpeed(f.type) * (0.8f + std::uniform_real_distribution<>(0.0f, 0.4f)(rng_));
    f.alive = true;
    f.wobblePhase = std::uniform_int_distribution<>(0, 360)(rng_);

    int edge = std::uniform_int_distribution<>(0, 3)(rng_);
    float margin = 2.0f;
    switch (edge) {
    case 0:
        f.x = std::uniform_real_distribution<>(margin, cw_ - margin)(rng_);
        f.y = -margin;
        f.dir = (f.x < cw_ / 2.0f) ? Direction::Right : Direction::Left;
        break;
    case 1:
        f.x = std::uniform_real_distribution<>(margin, cw_ - margin)(rng_);
        f.y = static_cast<float>(ch_) + margin;
        f.dir = (f.x < cw_ / 2.0f) ? Direction::Right : Direction::Left;
        break;
    case 2:
        f.x = -margin;
        f.y = std::uniform_real_distribution<>(margin, ch_ - margin)(rng_);
        f.dir = Direction::Right;
        break;
    case 3:
        f.x = static_cast<float>(cw_) + margin;
        f.y = std::uniform_real_distribution<>(margin, ch_ - margin)(rng_);
        f.dir = Direction::Left;
        break;
    }

    fishes_.push_back(f);
}

// ── movement ─────────────────────────────────────────────────────────────────

void Game::MoveFishes() {
    for (auto& f : fishes_) {
        if (!f.alive) continue;

        float dx = (f.dir == Direction::Right) ? f.speed : -f.speed;
        f.x += dx;

        float margin = 20.0f;
        if (f.x < -margin || f.x > cw_ + margin || f.y < -margin || f.y > ch_ + margin) {
            f.alive = false;
        }
    }

    fishes_.erase(
        std::remove_if(fishes_.begin(), fishes_.end(),
                       [](const Fish& f) { return !f.alive; }),
        fishes_.end());
}

// ── eating / collision ───────────────────────────────────────────────────────

void Game::CheckEating() {
    for (auto& f : fishes_) {
        if (!f.alive) continue;

        float dx = f.x - px_;
        float dy = f.y - py_;
        float dist = std::sqrt(dx * dx + dy * dy);

        float eatRadius = 3.0f;
        if (dist < eatRadius) {
            if (playerSize_ > f.size) {
                f.alive = false;
                int points = f.size * 10;
                score_ += points;

                for (int i = 0; i < 4; i++) {
                    Particle p;
                    p.x = f.x + std::uniform_real_distribution<>(-2, 2)(rng_);
                    p.y = f.y + std::uniform_real_distribution<>(-1, 1)(rng_);
                    p.life = 8;
                    p.text = "*";
                    p.color = ftxui::Color::YellowLight;
                    particles_.push_back(p);
                }
                Particle scoreP;
                scoreP.x = f.x;
                scoreP.y = f.y - 1;
                scoreP.life = 15;
                scoreP.text = "+" + std::to_string(points);
                scoreP.color = ftxui::Color::Gold1;
                particles_.push_back(scoreP);

                int newSize = playerSize_;
                for (int s = playerSize_ + 1; s <= 10; s++) {
                    if (score_ >= GrowthThreshold(s)) newSize = s;
                }
                if (newSize > playerSize_ && newSize <= 10) {
                    playerSize_ = newSize;
                    Particle growP;
                    growP.x = px_;
                    growP.y = py_ - 2;
                    growP.life = 20;
                    growP.text = "GROW!";
                    growP.color = ftxui::Color::GreenLight;
                    particles_.push_back(growP);
                }
                if (playerSize_ >= 10) {
                    state_ = GameState::Won;
                    if (score_ > highScore_) highScore_ = score_;
                }
            } else {
                state_ = GameState::GameOver;
                if (score_ > highScore_) highScore_ = score_;
                return;
            }
        }
    }
}

// ── particles ────────────────────────────────────────────────────────────────

void Game::UpdateParticles() {
    for (auto& p : particles_) {
        p.life--;
        p.y -= 0.3f;
    }
    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(),
                       [](const Particle& p) { return p.life <= 0; }),
        particles_.end());
}

// ── update ───────────────────────────────────────────────────────────────────

void Game::Update() {
    if (state_ != GameState::Playing) return;

    tick_++;

    difficulty_ = 1 + tick_ / 800;
    maxFishes_ = 20 + difficulty_ * 2;
    if (maxFishes_ > 50) maxFishes_ = 50;

    spawnCooldown_--;
    int spawnInterval = std::max(5, 18 - difficulty_);
    if (spawnCooldown_ <= 0) {
        SpawnFish();
        spawnCooldown_ = spawnInterval + std::uniform_int_distribution<>(0, 8)(rng_);
    }

    MoveFishes();
    CheckEating();
    UpdateParticles();
}

// ── rendering helpers ────────────────────────────────────────────────────────

// Utility to style both foreground and background on a pixel
static auto FgBg(ftxui::Color fg, ftxui::Color bg) {
    return [fg, bg](ftxui::Pixel& p) {
        p.foreground_color = fg;
        p.background_color = bg;
    };
}

// Canvas uses a coordinate system where DrawText(x,y) maps to cell (x/2, y/4),
// and the terminal cell grid is 1:1 with cells. So to draw at terminal (tx,ty):
//   DrawText(tx*2, ty*4, ...)
static void Put(ftxui::Canvas& c, int tx, int ty, const std::string& text,
                const ftxui::Canvas::Stylizer& s) {
    c.DrawText(tx * 2, ty * 4, text, s);
}

void Game::DrawWater(ftxui::Canvas& c) const {
    using ftxui::Color;

    for (int y = 0; y < ch_; y++) {
        Color bg;
        if (y < ch_ / 4)            bg = Color::Cyan;
        else if (y < ch_ / 2)       bg = Color::SteelBlue;
        else if (y < ch_ * 3 / 4)  bg = Color::Blue;
        else                        bg = Color::NavyBlue;

        for (int x = 0; x < cw_; x++) {
            int h = (x * 7 + y * 13 + tick_ / 2) % 23;
            Color fg = Color::Cyan;
            std::string ch(1, ' ');

            if (h == 0)      { ch = "~"; fg = Color::CyanLight; }
            else if (h == 1) { ch = "."; fg = Color::Grey70; }
            else if (h == 2) { ch = "."; fg = Color::CyanLight; }
            else if (h == 3) { ch = "~"; fg = Color::Cyan; }

            int bubble = ((x * 17 + y * 31 + tick_ / 3) % 67);
            if (bubble == 0) { ch = "o"; fg = Color::White; }
            else if (bubble == 1) { ch = "\xC2\xB0"; fg = Color::Grey85; } // °

            Put(c,x, y, ch, FgBg(fg, bg));
        }
    }
}

void Game::DrawFish(ftxui::Canvas& c, const Fish& f) const {
    if (!f.alive) return;
    std::string art = FishArt(f);
    auto color = FishColor(f);
    auto bg = ftxui::Color::Blue;
    int ix = static_cast<int>(std::round(f.x));
    int iy = static_cast<int>(std::round(f.y));

    if (f.dir == Direction::Right) {
        for (size_t i = 0; i < art.size(); i++) {
            int cx = ix + static_cast<int>(i);
            if (cx >= 0 && cx < cw_ && iy >= 0 && iy < ch_)
                Put(c,cx, iy, std::string(1, art[i]), FgBg(color, bg));
        }
    } else {
        for (size_t i = 0; i < art.size(); i++) {
            int cx = ix - static_cast<int>(i);
            if (cx >= 0 && cx < cw_ && iy >= 0 && iy < ch_)
                Put(c,cx, iy, std::string(1, art[i]), FgBg(color, bg));
        }
    }
}

void Game::DrawPlayer(ftxui::Canvas& c) const {
    std::string art = PlayerArt();
    auto fg = ftxui::Color::Red;
    auto bg = ftxui::Color::Blue;
    int ix = static_cast<int>(std::round(px_));
    int iy = static_cast<int>(std::round(py_));

    if (playerDir_ == Direction::Right) {
        for (size_t i = 0; i < art.size(); i++) {
            int cx = ix + static_cast<int>(i);
            if (cx >= 0 && cx < cw_ && iy >= 0 && iy < ch_)
                Put(c,cx, iy, std::string(1, art[i]), FgBg(fg, bg));
        }
    } else {
        for (size_t i = 0; i < art.size(); i++) {
            int cx = ix - static_cast<int>(i);
            if (cx >= 0 && cx < cw_ && iy >= 0 && iy < ch_)
                Put(c,cx, iy, std::string(1, art[i]), FgBg(fg, bg));
        }
    }
}

void Game::DrawParticles(ftxui::Canvas& c) const {
    for (const auto& p : particles_) {
        int ix = static_cast<int>(std::round(p.x));
        int iy = static_cast<int>(std::round(p.y));
        if (ix >= 0 && ix < cw_ && iy >= 0 && iy < ch_) {
            for (size_t i = 0; i < p.text.size(); i++) {
                int cx = ix + static_cast<int>(i);
                if (cx >= 0 && cx < cw_)
                    Put(c,cx, iy, std::string(1, p.text[i]), FgBg(p.color, ftxui::Color::Blue));
            }
        }
    }
}

void Game::DrawHUD(ftxui::Canvas& c) const {
    using ftxui::Color;

    // Top bar
    for (int x = 0; x < cw_; x++)
        Put(c,x, 0, " ", FgBg(Color::Black, Color::Grey7));

    std::ostringstream oss;
    oss << " Score: " << score_
        << " | Size: ";
    for (int i = 1; i <= 10; i++) oss << (i <= playerSize_ ? '#' : '.');
    oss << " " << playerSize_ << "/10"
        << " | Fish: " << fishes_.size();
    if (highScore_ > 0) oss << " | Best: " << highScore_;

    std::string info = oss.str();
    int startX = std::max(0, (cw_ - static_cast<int>(info.size())) / 2);
    Put(c,startX, 0, info, FgBg(Color::White, Color::Grey7));
}

void Game::DrawTitle(ftxui::Canvas& c) const {
    using ftxui::Color;
    auto bg = Color::NavyBlue;

    for (int y = 0; y < ch_; y++)
        for (int x = 0; x < cw_; x++)
            Put(c,x, y, " ", FgBg(Color::Cyan, bg));

    const char* title[] = {
        "  ____  _       __  __ _     _      _____ _     _       ",
        " |  _ \\(_)     / _|/ _(_)   | |    / ____(_)   | |      ",
        " | |_) |_  __ | |_| |_ _ ___| |__ | (___  _ ___| |__    ",
        " |  _ <| |/ _` |  _|  _| / __| '_ \\ \\___ \\| / __| '_ \\   ",
        " | |_) | | (_| | | | | | \\__ \\ | | |____) | \\__ \\ | | |  ",
        " |____/|_|\\__, |_| |_| |_|___/_| |_|_____/|_|___/_| |_|  ",
        "           __/ |                                          ",
        "          |___/   v2 - EAT or be eaten!                  ",
    };
    int n = sizeof(title) / sizeof(title[0]);
    int startY = (ch_ - n - 4) / 2;
    for (int i = 0; i < n; i++) {
        std::string line(title[i]);
        int sx = (cw_ - static_cast<int>(line.size())) / 2;
        Put(c,sx, startY + i, line, FgBg(Color::Gold1, bg));
    }

    {
        std::string prompt = "Press ENTER or SPACE to start";
        int sx = (cw_ - static_cast<int>(prompt.size())) / 2;
        Put(c,sx, startY + n + 2, prompt, FgBg(Color::White, bg));
    }
    {
        std::string controls = "WASD/Arrows: Move  |  Eat smaller fish  |  Avoid bigger fish  |  Q: Quit";
        int sx = (cw_ - static_cast<int>(controls.size())) / 2;
        Put(c,sx, startY + n + 3, controls, FgBg(Color::Grey70, bg));
    }
}

void Game::DrawGameOver(ftxui::Canvas& c) const {
    using ftxui::Color;
    auto bg = Color::DarkRed;

    for (int y = 0; y < ch_; y++)
        for (int x = 0; x < cw_; x++)
            if ((x + y) % 3 == 0)
                Put(c,x, y, " ", FgBg(Color::Red, bg));

    const char* go[] = {
        "  _____                         ____                 ",
        " / ____|                       / __ \\                ",
        "| |  __  __ _ _ __ ___   ___  | |  | |_   _____ _ __ ",
        "| | |_ |/ _` | '_ ` _ \\ / _ \\ | |  | \\ \\ / / _ \\ '__|",
        "| |__| | (_| | | | | | |  __/ | |__| |\\ V /  __/ |   ",
        " \\_____|\\__,_|_| |_| |_|\\___|  \\____/  \\_/ \\___|_|   ",
    };
    int n = sizeof(go) / sizeof(go[0]);
    int startY = (ch_ - n - 3) / 2;
    for (int i = 0; i < n; i++) {
        std::string line(go[i]);
        int sx = (cw_ - static_cast<int>(line.size())) / 2;
        Put(c,sx, startY + i, line, FgBg(Color::White, bg));
    }
    {
        std::string s = "Final Score: " + std::to_string(score_);
        int sx = (cw_ - static_cast<int>(s.size())) / 2;
        Put(c,sx, startY + n + 1, s, FgBg(Color::Gold1, bg));
    }
    {
        std::string s = "Press R to restart  |  Q to quit";
        int sx = (cw_ - static_cast<int>(s.size())) / 2;
        Put(c,sx, startY + n + 2, s, FgBg(Color::White, bg));
    }
}

void Game::DrawWin(ftxui::Canvas& c) const {
    using ftxui::Color;
    auto bg = Color::DarkGreen;

    for (int y = 0; y < ch_; y++)
        for (int x = 0; x < cw_; x++)
            if ((x + y) % 3 == 0)
                Put(c,x, y, " ", FgBg(Color::YellowLight, bg));

    const char* win[] = {
        " __     __          __          ___       _ ",
        " \\ \\   / /          \\ \\        / (_)     | |",
        "  \\ \\_/ /__  _   _   \\ \\  /\\  / / _ _ __ | |",
        "   \\   / _ \\| | | |   \\ \\/  \\/ / | | '_ \\| |",
        "    | | (_) | |_| |    \\  /\\  /  | | | | |_|",
        "    |_|\\___/ \\__,_|     \\/  \\/   |_|_| |_(_)",
    };
    int n = sizeof(win) / sizeof(win[0]);
    int startY = (ch_ - n - 3) / 2;
    for (int i = 0; i < n; i++) {
        std::string line(win[i]);
        int sx = (cw_ - static_cast<int>(line.size())) / 2;
        Put(c,sx, startY + i, line, FgBg(Color::White, bg));
    }
    {
        std::string s = "Final Score: " + std::to_string(score_);
        int sx = (cw_ - static_cast<int>(s.size())) / 2;
        Put(c,sx, startY + n + 1, s, FgBg(Color::Gold1, bg));
    }
    {
        std::string s = "Press R to restart  |  Q to quit";
        int sx = (cw_ - static_cast<int>(s.size())) / 2;
        Put(c,sx, startY + n + 2, s, FgBg(Color::White, bg));
    }
}

// ── full render ──────────────────────────────────────────────────────────────

ftxui::Element Game::Render() const {
    auto c = ftxui::Canvas(cw_ * 2, ch_ * 4);

    switch (state_) {
    case GameState::Title:
        DrawTitle(c);
        break;
    case GameState::Playing: {
        DrawWater(c);
        // Sort by size descending so large fish draw first (smaller on top)
        std::vector<const Fish*> sorted;
        for (const auto& f : fishes_)
            if (f.alive) sorted.push_back(&f);
        std::sort(sorted.begin(), sorted.end(),
                  [](const Fish* a, const Fish* b) { return a->size > b->size; });
        for (const auto* f : sorted)
            DrawFish(c, *f);
        DrawParticles(c);
        DrawPlayer(c);
        DrawHUD(c);
        break;
    }
    case GameState::GameOver:
        DrawWater(c);
        DrawGameOver(c);
        DrawHUD(c);
        break;
    case GameState::Won:
        DrawWater(c);
        DrawWin(c);
        DrawHUD(c);
        break;
    }

    return ftxui::canvas(std::move(c));
}

// ── input ────────────────────────────────────────────────────────────────────

void Game::OnKey(const ftxui::Event& event) {
    using ftxui::Event;

    if (state_ == GameState::Title) {
        if (event == Event::Return || event.character() == " ") {
            Init(cw_, ch_);
            state_ = GameState::Playing;
            return;
        }
        return;
    }

    if (state_ == GameState::GameOver || state_ == GameState::Won) {
        if (event.character() == "r" || event.character() == "R") {
            Init(cw_, ch_);
            state_ = GameState::Playing;
            return;
        }
        return;
    }

    // Playing state
    float playerSpeed = 1.6f - playerSize_ * 0.1f;
    if (playerSpeed < 0.4f) playerSpeed = 0.4f;

    float dx = 0, dy = 0;

    if (event == Event::ArrowUp    || event.character() == "w" || event.character() == "W") dy = -playerSpeed;
    if (event == Event::ArrowDown  || event.character() == "s" || event.character() == "S") dy =  playerSpeed;
    if (event == Event::ArrowLeft  || event.character() == "a" || event.character() == "A") {
        dx = -playerSpeed;
        playerDir_ = Direction::Left;
    }
    if (event == Event::ArrowRight || event.character() == "d" || event.character() == "D") {
        dx =  playerSpeed;
        playerDir_ = Direction::Right;
    }

    px_ += dx;
    py_ += dy;

    int margin = 1;
    int artLen = static_cast<int>(PlayerArt().size());
    if (playerDir_ == Direction::Right) {
        if (px_ < margin) px_ = static_cast<float>(margin);
        if (px_ + artLen > cw_ - margin) px_ = static_cast<float>(cw_ - margin - artLen);
    } else {
        if (px_ - artLen < margin) px_ = static_cast<float>(margin + artLen);
        if (px_ > cw_ - margin) px_ = static_cast<float>(cw_ - margin);
    }
    if (py_ < margin + 1) py_ = static_cast<float>(margin + 1);
    if (py_ > ch_ - 1) py_ = static_cast<float>(ch_ - 1);
}

} // namespace bfg
