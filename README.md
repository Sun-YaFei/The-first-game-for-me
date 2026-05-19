# The-first-game-for-me
# Big Fish Eat Small Fish

## 1. Project Introduction

This project is a terminal-based mini game developed with C++ and the FTXUI library. The game is called **Big Fish Eat Small Fish**. The player controls a fish in an ocean map, eats fish that are smaller than or equal to itself, gains points, and gradually grows larger. If the player touches a fish that is larger than itself, the game ends.

The project is written in C++17 and uses FTXUI to create a colorful terminal user interface, including a game map, status panel, growth progress bar, pause function, restart function, and game-over message.

This project is suitable for a C++ course assignment or an AI-assisted programming practice project. The code structure is clear and includes a CMake build file, header file, source files, and a README document.

---

## 2. Project Features

1. **Developed with C++**

   The project uses C++17 and is suitable for learning classes, structs, functions, random numbers, game loops, and basic project organization.

2. **Terminal UI with FTXUI**

   Compared with simple `cout` output, FTXUI provides a more attractive terminal interface with colors, borders, layouts, and keyboard interaction.

3. **Fish-shaped character design**

   Fish are displayed using character combinations, such as:

   - `><>`: small fish
   - `<><`: small fish facing the opposite direction
   - `>O>`: medium fish
   - `>#>`: large fish

4. **Color distinction**

   Different objects are displayed with different colors:

   - Yellow background: player fish
   - Cyan background: small fish that can be eaten
   - Green background: same-size fish that can be eaten
   - Red background: dangerous large fish

5. **Opening protection stage**

   During the first 5 seconds, no other fish will appear on the map. Only the player fish is shown, giving the player time to prepare.

6. **Faster player movement**

   The player fish moves faster than other fish. The player moves 2 grid cells at a time.

7. **Horizontal fish movement**

   Other fish enter from the left or right side of the map and only move horizontally. They do not chase the player or escape from the player.

8. **Basic game functions**

   The game includes:

   - Score system
   - Fish growth system
   - Pause and continue
   - Restart
   - Game-over detection
   - Keyboard control

---

## 3. Project Structure

```text
big_fish_game/
├── CMakeLists.txt
├── README.md
├── include/
│   └── Game.hpp
└── src/
    ├── Game.cpp
    └── main.cpp
