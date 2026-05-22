# Big Fish Game

A terminal-based "big fish eats small fish" game built with C++17 and FTXUI.

Navigate the ocean as a growing predator — eat smaller fish to grow, avoid bigger fish to survive, and try to become the biggest fish in the sea.

## Preview

```
  ~  ~       ~  ~    ,>     ~  ~     ~       ~  ~
~    =[#]> ~    ~  ~        ~~>      ~   ~    ~~~~>
    ~     ~       ~  ~    ~     ~  ~     ~  ~
  ~    <~     ~  ~     ~       ~   ~     ~      ~
```

## Requirements

- **C++17** or later
- **CMake** 3.16+
- A terminal with **true-color** support (recommended)

## Quick Start

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Run (Linux / macOS / WSL)
./build/big_fish_game

# Run (Windows MSVC)
./build/Release/big_fish_game.exe
```

FTXUI is fetched automatically via CMake `FetchContent` — no manual dependency installation needed.

### Run in VSCode

#### Method 1 — CMake Tools Extension (Recommended)

1. Install the **[CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)** extension.
2. Open the project folder in VSCode.
3. Press `Ctrl+Shift+P` and run **CMake: Configure**. Select a kit (GCC, Clang, or MSVC).
4. Press `Ctrl+Shift+P` and run **CMake: Build**.
5. Open a terminal in VSCode (`` Ctrl+` ``) and run:

```bash
# Linux / macOS / WSL
./build/big_fish_game

# Windows (MSVC)
.\build\Release\big_fish_game.exe
```

> **Note:** VSCode's integrated terminal fully supports the game's fullscreen mode.

#### Method 2 — Terminal Only

1. Open the project folder in VSCode.
2. Open the integrated terminal (`` Ctrl+` ``).
3. Run the build commands directly:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

4. Execute the game as shown above.
5. Press `Q` to quit the game at any time.

---

### Run in GitHub Codespaces

1. Open your repository in GitHub Codespaces (click **Code → Codespaces → Create codespace on main**).
2. Once the workspace loads, install the required build tools in the terminal:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake
```

3. Configure and build the project:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

4. Run the game:

```bash
./build/big_fish_game
```

5. Press `Q` to quit.

> **Tip:** For the best visual experience in Codespaces, maximize the terminal panel or open it in a new browser tab via the terminal's dropdown menu.

## How to Play

| Key | Action |
|-----|--------|
| `W` / `↑` | Move up |
| `S` / `↓` | Move down |
| `A` / `←` | Move left |
| `D` / `→` | Move right |
| `Q` | Quit |
| `R` | Restart (after game over) |
| `Enter` / `Space` | Start game |

### Rules

- Eat fish **smaller** than you to gain points and grow.
- Fish **larger** than you will eat you — game over.
- Reach **size 10** to win the game.
- The ocean gets more dangerous as your score increases.

### Fish Types

| Type | Size | Speed | Color | Rarity |
|------|------|-------|-------|--------|
| Sardine | 1–3 | Fast | White | Common |
| Clownfish | 2–4 | Medium | Orange | Common |
| Tuna | 4–6 | Slow | Steel Blue | Uncommon |
| Shark | 6–8 | Medium | Grey | Rare |
| Whale | 8–10 | Very Slow | Dark Blue | Very Rare |

### Scoring

- Points per fish = fish size × 10
- Growth thresholds: 10, 25, 50, 80, 120, 180, 250, 350, 500, 700 points

## Project Structure

```
big_fish_game/
├── include/
│   └── Game.hpp          # Game class and struct declarations
├── src/
│   ├── Game.cpp          # Game logic and rendering
│   └── main.cpp          # Entry point and FTXUI event loop
├── CMakeLists.txt         # Build configuration
├── .gitignore
└── README.md
```

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/cool-thing`
3. Commit your changes: `git commit -m "Add cool thing"`
4. Push to the branch: `git push origin feature/cool-thing`
5. Open a pull request

### Ideas for Contributions

- Power-ups (speed boost, invincibility, magnet)
- High-score persistence
- Additional fish types and behaviors
- Multiplayer mode
- Sound effects
- Color theme customization

## License

This project is provided for educational and personal use. Feel free to modify and share.
