# Comprehensive Project Report: AI Assisted Pseudo-3D Racing Game
**Date:** January 09, 2026
**Language:** C++17
**Framework:** SFML 2.5+

---

## 1. Executive Summary

This project is a detailed implementation of a classic "Pseudo-3D" arcade racing game, similar to 1980s hits like *OutRun*. Unlike modern 3D games that use true 3D geometry (mesh, vertices, matrices), this project uses **Perspective Projection** mathematics to trick the human eye into perceiving depth on a 2D plane. 

The project demonstrates advanced C++ concepts including **Object-Oriented Programming (OOP)**, **Memory Management (RAII)**, **Real-Time Physics Simulation**, and **Artificial Intelligence**.

---

## 2. In-Depth Library Analysis

The project is built on top of **SFML (Simple and Fast Multimedia Library)**. Here is exactly why and where each module is used:

### **A. SFML System (`<SFML/System.hpp>`)**
*   **`sf::Time` & `sf::Clock`**: 
    *   *Usage*: Used in `Game.cpp` for the "Delta Time" calculation. 
    *   *Why*: Computers run at different speeds. If we moved the car `1 pixel` per frame, a fast computer (1000 FPS) would move the car 1000 pixels/sec, while a slow one (60 FPS) would move it 60 pixels/sec. We measure the time taken for a frame (e.g., `0.016s`) and multiply movement by this time.
    *   *Code Reference*: `mPlayer->move(speed * dt.asSeconds())`.

### **B. SFML Window (`<SFML/Window.hpp>`)**
*   **`sf::RenderWindow`**: 
    *   *Usage*: Creates the OS window where the game lives.
    *   *Why*: Managing Windows/Mac/Linux window managers is hard. SFML abstracts this into a simple class.
*   **`sf::Event`**:
    *   *Usage*: Captures keyboard presses, window close buttons, etc.
    *   *Why*: We need to know when the user presses 'W' or 'A' to drive.

### **C. SFML Graphics (`<SFML/Graphics.hpp>`)**
*   **`sf::Sprite` & `sf::Texture`**:
    *   *Usage*: Sprites are the "objects" (cars, trees). Textures are the "images" loaded from disk (`.png`).
    *   *Why*: `ResourceManager.cpp` loads the Texture once (into VRAM), and we create thousands of light-weight Sprites that point to that one Texture.
*   **`sf::ConvexShape`**:
    *   *Usage*: Used in `World.cpp` to draw the Trapezoids for the road.
    *   *Why*: The road isn't a rectangle; it's wide at the bottom and narrow at the top (perspective). `ConvexShape` lets us define these 4 arbitrary points.

---

## 3. Deep Dive: Code Architecture & Logic

### **Module 1: The Engine (`Game.cpp`)**

This is the central nervous system of the game.

*   **Fixed Time Step Loop**: 
    *   *Logic*: The game loop doesn't just "run". It accumulates time.
    ```cpp
    while (timeSinceLastUpdate > TimePerFrame) {
        timeSinceLastUpdate -= TimePerFrame;
        processEvents();
        update(TimePerFrame);
    }
    ```
    *   *Depth*: This guarantees that **physics simulation** happens exactly 60 times per second, regardless of rendering speed. This prevents "tunneling" (objects passing through each other at high speeds).

### **Module 2: The Pseudo-3D World (`World.cpp`)**

This is the most mathematically complex part.

*   **The Projection Formula**:
    How do we turn a 3D world coordinate `(x, y, z)` into a simplified 2D screen coordinate?
    In this game, `y` acts as our depth (Z).
    ```cpp
    // Calculate how "deep" into the screen the object is (0.0 to 1.0)
    float scale = (y - horizonY) / (camY - horizonY);
    
    // Move X towards the center based on depth (Vanishing Point effect)
    float screenX = centerX + (worldX - centerX) * scale;
    ```
    *   *Result*: As `y` approaches the horizon, `scale` becomes smaller. This shrinks objects and pulls them to the center, creating the illusion of distance.

*   **The Infinite Loop**:
    We don't create an infinite road. We create `mScrollY`.
    ```cpp
    float yPixelOffset = std::fmod(mScrollY, segmentHeight);
    ```
    *   *Depth*: We draw the grid of road segments and simply "slide" the texture coordinates. When `mScrollY` exceeds a segment's height, it wraps around. To the user, it looks like endless movement.

*   **Day/Night Interpolation**:
    We don't switch from "Day" to "Night"; we *morph*.
    ```cpp
    // Linear Interpolation (Lerp)
    sf::Uint8 r = c1.r + (c2.r - c1.r) * t;
    ```
    *   *Depth*: We calculate the Red, Green, and Blue values mathematically between `SkyBlue` and `PitchBlack` based on the game time.

### **Module 3: Artificial Intelligence (`AIController.cpp`)**

The enemies aren't just moving blocks; they think (rudimentarily).

*   **Lane Detection**:
    The AI divides the road into 3 hypothetical lanes: 150, 400, 650.
*   **Raycasting Logic**:
    *   *Depth*: The AI projects an invisible rectangle ("Ray") in front of itself.
    *   `if (ray.intersects(otherCar))` -> **BLOCKED**.
*   **Decision Tree**:
    1.  Am I blocked?
    2.  Check Left Lane: Is it empty? -> **Merge Left**.
    3.  Check Right Lane: Is it empty? -> **Merge Right**.
    4.  Both Blocked? -> **Brake**.

### **Module 4: Resource Management (`ResourceManager.cpp`)**

A critical optimization pattern.

*   **The Singleton Pattern**:
    *   *Concept*: There should only ever be **one** `ResourceManager`.
    *   *Implementation*: `static ResourceManager& getInstance()`.
*   **Texture Caching**:
    *   *Problem*: `texture.loadFromFile("car.png")` is slow (hard drive access).
    *   *Solution*: We use a `std::map<string, sf::Texture>`.
    *   *Logic*:
        1.  User asks for "car".
        2.  Check map. Exists? Return it.
        3.  Doesn't exist? Load it, Save to map, Return it.
    *   *Benefit*: No matter how many cars we spawn, we only load the PNG once.

---

## 4. Coding "Tricks" Explained

*   **Screen Shake**:
    We add energy to the camera when a crash happens.
    ```cpp
    offsetX = sin(time * 50) * shakeIntensity;
    ```
    Using a sine wave creates a violent vibration that decays over time.

*   **Relative Speed**:
    The Player Car never actually moves up the screen Y-axis (it stays at Y=500).
    Instead, the **World moves down**.
    *   Oncoming Traffic Speed = `EnemySpeed + PlayerSpeed`.
    *   This creates the *Theory of Relativity* in game code. 

---

## 5. Summary
This project is a complete graphics simulation. It manages physics, rendering pipelines, AI decision making, and asset memory handling, all written in raw C++ without the aid of a pre-made game engine like Unity or Unreal.
