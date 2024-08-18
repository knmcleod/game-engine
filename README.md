# game-engine

## About
Game Engine is an early-stage rendering engine for Windows.
The goal of Game Engine is to learn the processes involved in creating a game engine and environment.

### Materials Used
Game Engine is written by me, Kerra McLeod, starting in August 2023, inspired by the works of Jason Gregorys "Game Engine Architecture", Yan Chernikov (The Cherno) "Game Engine", and others in the game development community.

## Get Started
Visual Studio 2022 is recommended.
Premake5 is required.

Game Engine is untested on development environments other than Windows builds.

Start by cloning the repository with `git clone --recursive https://github.com/knmcleod/game-engine`.
Locate Win-GenerateProjects.bat in scripts, and run.
Next, locate game-engine.sln in main directory, and build.

To use Game Editor Application, 
    navigate to `bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/editor/editor.exe`
    NOTE: %{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture} is specified by premake5.lua

## Features 
Features are subject to change.
### Current:
- Application Window using GLFW
- Input & Event system
- Project System, including Asset Management
- Text Asset Registry using filesystem for Asset Management
- 2D Rendering using OpenGL (sprites, quads, circles & rectangle/lines)
- Text Font using MSDF-atlas-gen
- Scene Asset using Entities
- Entity Component system using entt
- Native C++ Scripting, extending Entity
- C# Scripting using Mono
- 2D Physics system using box2d
- Editor Application for game development using Game Engine

### Future:
- More Editor application Tools
- Binary Asset Pack for Asset Management
- Audio system
- Project Exportation
- 3D Physics
- 2D Animation
- Lighting
- Artificial Intelligence
- Networking
- User Interface : Accessibility Settings
- Platform Support (Mac, Linux, Android & iOS) : (DirectX, Vulkan, Metal)
- Optimization
    - Updated Profiling
    - Memory Management
    - Multi-threading
- Distribution
    - Custom formats
    - Localization
    - Stripping Symbols

- Demo application showcasing classic games like Checkers, Minesweeper & Bumper Cars using Game Engine
- More Games using Game Engine