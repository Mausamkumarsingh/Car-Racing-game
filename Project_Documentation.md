# C++ Pseudo-3D Racing Game - Project Documentation

## 1. Project Overview
**Project Name:** AI Assisted Pseudo-3D Racing Game  
**Type:** Arcade Racing Game  
**Language:** C++17  

This project is a high-performance, arcade-style racing game built from scratch using C++. It creates a retro "Pseudo-3D" visual effect similar to classic games like *OutRun*. The game features an infinite road, a dynamic day/night cycle, intelligent traffic AI, and a high-score system. The player controls a car, dodging traffic and navigating a world that changes from day to night in real-time.

---

## 2. Technology Stack

This project was built using industry-standard tools for performance and portability.

| Technology | Purpose | Why it was chosen? |
| :--- | :--- | :--- |
| **C++ (ISO C++17)** | Core Logic | Provides direct memory management and high performance required for the game loop and rendering calculations. |
| **SFML (Simple & Fast Multimedia Library)** | I/O & Graphics | abstracted the complex low-level OS calls. Used for creating the window, capturing keyboard input, loading textures, playing audio, and drawing 2D primitives. |
| **CMake** | Build System | Ensures the project is cross-platform and can be compiled on Windows, Linux, and macOS without changing the build commands. |

---

## 3. Project Architecture

The codebase is modular, separating the Game Loop, World Rendering, and Entity Management.

### **File Structure & Roles**

*   **`main.cpp`**
    *   **Entry Point**: Creates the `Game` instance and starts the run loop. Kept intentionally minimal.

*   **`Game.cpp / .h` (The Engine)**
    *   **Game Loop**: Implements the `processEvents` -> `update` -> `render` cycle.
    *   **State Management**: Handles switching between `Menu`, `Playing`, and `GameOver` states.
    *   **Time Management**: Uses a "Fixed Time Step" (60 updates/second) to ensure physics behave the same on fast and slow computers.

*   **`World.cpp / .h` (The Environment)**
    *   **Rendering Core**: Handles the mathematical projection of the road segments.
    *   **Environment**: Manages the Day/Night cycle, background gradients, and prop spawning (trees, lights).

*   **`Entities/PlayerCar.cpp` (The Player)**
    *   **Input Handling**: Maps WASD/Arrow keys to movement coordinates.
    *   **Physics**: implementation of simple velocity and position updates.

*   **`Entities/EnemyCar.cpp` (The AI)**
    *   **Behaviors**: Can spawn as "Oncoming" (moving fast towards player) or "SameWay" (traffic being overtaken).
    *   **AI Integration**: delegates decision making to the `AIController`.

*   **`AI/AIController.cpp` (The Brains)**
    *   **Logic**: Detects obstacles in front of the enemy car and initiates lane changes to avoid collisions.

*   **`ResourceManager.cpp` (Optimization)**
    *   **Singleton Pattern**: Loads heavy assets (Images/Sounds) only once into memory and serves pointers to any object that needs them, drastically reducing memory usage.

---

## 4. Key Technical Concepts (The "Hard Parts")

### **A. Pseudo-3D Perspective Projection**
The game creates a 3D illusion without using a 3D engine (like OpenGL 3D or Unity). It uses **Perspective Projection** on 2D shapes.

**The Math:**
```cpp
// Transforms a World (x, y) coordinate to a Screen (x, y) coordinate
static sf::Vector2f project(float x, float y) {
  // 1. Calculate Scale: Objects higher on screen (further away) are smaller
  float scale = (y - horizonY) / (cameraY - horizonY);

  // 2. Project X: Pull x towards the center based on scale (Vanishing Point)
  float screenX = centerX + (x - centerX) * scale;
  
  return sf::Vector2f(screenX, y);
}
```
*Result*: Parallel lines (sides of the road) appear to converge at a vanishing point on the horizon.

### **B. Infinite Scrolling Road**
The road is not infinite in memory. It is a loop of coordinates.
*   **Technique**: The variable `mScrollY` increases as the player drives.
*   **Rendering**: The road is drawn as horizontal strips. The color of each strip is determined by `(mScrollY + segmentIndex) % 2`. This creates the "moving stripes" effect that gives the sensation of speed.

### **C. Relative Speed System**
To simulate movement, we move the *world*, not the player.
*   **Player Position**: Fixed at the bottom of the screen.
*   **World Movement**: Trees and Road move down at `playerSpeed`.
*   **Traffic Movement**: 
    *   Same Way Traffic Speed = `CarSpeed - PlayerSpeed`.
    *   Oncoming Traffic Speed = `CarSpeed + PlayerSpeed`.

### **D. Dynamic Day/Night Cycle**
A timer `mDayTime` runs from 0.0 to 1.0.
*   **Sky**: Colors are interpolated (blended) between Blue -> Orange -> Black -> Purple.
*   **Lighting**: Street lights check this value. If `mDayTime` is in the "Night" range, they render a glowing halo effect.

---

## 5. Summary
This project demonstrates proficiency in **Object-Oriented Programming (OOP)**, **Vector Mathematics**, **Real-time Simulation**, and **Resource Management**. It successfully creates a complex visual experience using fundamental computer graphics algorithms.
