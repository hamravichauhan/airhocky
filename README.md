# Air Hockey — C / raylib LAN Multiplayer Game

## Problem Definition

Most 2-player games require shared hardware or dedicated servers. This project
provides a real-time air hockey game in pure C that supports both local
(same-keyboard) and LAN (same-WiFi) play, demonstrating physics simulation
and TCP network programming.

## Objectives

1. Build an interactive air hockey game with accurate 2D physics
2. Support two players on the same keyboard (local mode)
3. Support two players over LAN via TCP sockets (host/join)
4. Provide a clean UI with menus, countdown, scoring, and win detection

## Features

- Full 2D paddle movement with acceleration and friction
- Realistic puck physics: friction, restitution, spin, sub-stepped collision
- "Flick shot" mechanic — fast paddle movement transfers momentum to the puck
- 3-2-1-GO countdown between rounds
- Goal flash + screen shake on score
- Host-authoritative networking (zero desync)
- Cross-platform: macOS + Linux (POSIX sockets)

## Controls

| Mode   | Player 1 (blue)          | Player 2 (red)                |
| ------ | ------------------------ | ----------------------------- |
| Local  | W A S D                  | Arrow keys                    |
| Host   | W A S D (mouse optional) | (client controls via network) |
| Client | W A S D (mouse optional) | —                             |

- ENTER to select menus
- ESC to cancel / return to menu

## Build

### macOS

```bash
brew install raylib
make
./airhockey
```
