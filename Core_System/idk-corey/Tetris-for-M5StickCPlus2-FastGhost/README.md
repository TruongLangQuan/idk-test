# FAST Tetris for M5StickC Plus2

## Overview
FAST Tetris combines the best performance optimizations and features from both the Original Enhanced Tetris and Halloween Tetris projects to create the fastest, most responsive Tetris experience on the M5StickC Plus2.

## Key Performance Features
- **Ultra-Fast Game Loop**: Optimized rendering pipeline for maximum FPS
- **Aggressive Compiler Optimizations**: -O3, fast-math, loop unrolling
- **Efficient Memory Management**: Minimal allocations during gameplay
- **Streamlined WiFi Scanning**: Non-blocking beacon detection
- **Optimized Collision Detection**: Fast tetromino placement algorithms
- **Reduced Screen Updates**: Only redraw changed areas

## Combined Features from Both Projects
- **WiFi Beacon Scanner**: Real-time network detection with minimal overhead
- **Joystick Support**: External joystick with fast input processing  
- **Visual Enhancements**: Best visual elements optimized for speed
- **IR Remote Control**: Universal remote functionality
- **Ghost Piece**: Fast shadow piece calculation

## Performance Specifications
- Target: 60+ FPS during active gameplay
- WiFi scan frequency: Reduced for gaming performance
- Input latency: <10ms response time
- Memory usage: Optimized for continuous play

## Controls
- **Button A**: Rotate piece
- **Button B**: Drop piece / Start game
- **Power Button**: Menu navigation
- **External Joystick**: Full directional control (if connected)

## Installation
1. Use PlatformIO for fastest compilation and flashing
2. Connect external joystick if desired
3. Power on and experience lightning-fast Tetris!

## Build Optimizations
- Compiler: -O3 optimization level
- Fast math operations enabled
- Loop unrolling for critical paths
- Debug symbols removed for speed

Built for maximum performance and responsiveness on M5StickC Plus2.