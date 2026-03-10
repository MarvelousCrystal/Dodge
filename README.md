# Dodge! 

A single-player survival arcade game built with [Enviro](https://github.com/klavinslab/enviro) and [Elma](https://github.com/klavinslab/elma).

Survive as long as possible while dodging obstacles that spawn from the edges of the arena. Each hit costs you HP — lose all 5 and it's game over!

---

## Overview

**Dodge!** is a real-time survival game implemented as a multi-agent simulation using the Enviro framework. The player controls a blue circle using the arrow keys. Orange obstacles spawn at random positions along the arena boundary and fly straight toward the center. The longer you survive, the more frequently obstacles spawn — making the game progressively harder.

Key features:
- Smooth arrow key movement (omni-directional)
- Dynamic multi-agent obstacle spawning and removal via `add_agent()` / `remove_agent()`
- HP system: player starts with 100 HP, each hit deducts 20
- Hit invincibility frames to prevent instant death from multiple collisions
- Increasing difficulty: spawn interval shrinks as time goes on
- Restart button for instant replay without restarting the server

---

## Agents

| Agent | Type | Role |
|-------|------|------|
| `Player` | dynamic omni | Player-controlled character; tracks HP and handles movement |
| `Obstacle` | dynamic omni | Spawned at arena edge, flies straight toward center |
| `Coordinator` | invisible | Manages game loop: spawning, difficulty scaling, and restart |

---

## Controls

| Input | Action |
|-------|--------|
| `Arrow Up` | Move up |
| `Arrow Down` | Move down |
| `Arrow Left` | Move left |
| `Arrow Right` | Move right |
| `Restart` button | Start a new round |

---

## Key Challenges & How They Were Addressed

**1. Dynamic agent spawning and safe removal**

Obstacles are created at runtime using `add_agent()` and removed using `remove_agent()`. A key challenge was preventing double-removal crashes — if `game_restart` and the lifetime timer both tried to remove the same agent, Enviro would throw a runtime error. This was solved by introducing a `removing` boolean flag that ensures `remove_agent()` is only ever called once per obstacle.

**2. Obstacle movement stability (no jitter)**

Early versions used `omni_move_toward(0, 0, speed)` every tick, which caused visible jitter as the physics engine received conflicting forces. The fix was to compute the direction vector toward center exactly once in `start()`, then use `omni_track_velocity()` with that fixed vector every tick — resulting in smooth, straight-line motion.

**3. Difficulty scaling**

Spawn interval decreases linearly from 1.5 seconds to a minimum of 0.8 seconds as elapsed time increases, using `fmax(0.8, 1.5 - elapsed/30.0)`. This gives players time to learn at the start while creating genuine pressure over time.

**4. Hit invincibility frames**

Without invincibility, one obstacle could trigger multiple collision events in successive ticks, draining all HP instantly. A `hit_cooldown` timer (1.0 second) blocks further damage until it expires, giving the player time to react after each hit.

**5. Restart without server restart**

Restarting required clearing all live obstacle agents and resetting player state. This is handled by the Coordinator emitting a `game_restart` event, which each active obstacle listens to (triggering safe self-removal) and the player listens to (resetting HP and teleporting back to center).

**6. Correct Enviro config structure**

A significant debugging challenge was discovering that invisible agents must go in the `"invisibles"` array, dynamically-spawned agent types must be declared in `"references"`, and button events use `e.value()["value"]` (not `["name"]`) to identify which button was clicked. These were discovered by reading Enviro source examples directly.

---

## Installation & Running

### Prerequisites
- [Docker](https://www.docker.com/) installed on your machine

### Steps

```bash
# 1. Clone this repository
git clone https://github.com/MarvelousCrystal/Dodge.git
cd dodge

# 2. Start a Docker container with port mapping
docker run -p 80:80 -p 8765:8765 -v "$(pwd)":/workdir -it klavins/enviro:v1.5 bash

# 3. Inside the container, build the project
cd /workdir
make

# 4. Start the enviro manager and run the simulation
esm start
enviro
```

Then open your browser and go to: **http://localhost**

> **Note:** Port 80 must be mapped (`-p 80:80`) for the browser frontend to load. Port 8765 carries the game state data.

---

## How to Play

1. Open **http://localhost** in your browser after running `enviro`
2. Click inside the arena to focus the window
3. Use the **arrow keys** to move your blue character around the arena
4. Avoid the **orange obstacles** that spawn from the edges and fly toward the center
5. Each hit costs **20 HP** — you start with 100 HP (5 hits total)
6. A yellow glow indicates spawn invincibility; red flash means you just got hit
7. If your HP reaches 0, **YOU DIED** appears on screen
8. Click **Restart** to play again immediately

---

## Project Structure

```
dodge/
├── config.json          # Enviro configuration: agents, buttons, arena walls
├── Makefile             # Build system
├── defs/
│   ├── player.json      # Player agent definition (omni, dynamic)
│   ├── obstacle.json    # Obstacle agent definition (omni, dynamic)
│   └── coordinator.json # Coordinator agent definition (invisible)
├── src/
│   ├── player.h         # Player controller: movement, HP, hit handling
│   ├── player.cc        # Player agent entry point
│   ├── obstacle.h       # Obstacle controller: straight-line movement, removal
│   ├── obstacle.cc      # Obstacle agent entry point
│   ├── coordinator.h    # Game manager: spawning, difficulty, restart
│   └── coordinator.cc   # Coordinator agent entry point
├── lib/                 # Compiled .so files (generated by make)
├── LICENSE
└── README.md
```

---

## Acknowledgements & Sources

- [Enviro](https://github.com/klavinslab/enviro) — multi-agent simulation framework by Klavins Lab
- [Elma](https://github.com/klavinslab/elma) — event-driven process manager by Klavins Lab
- Enviro examples referenced: `multiuser` (invisible coordinator pattern, `add_agent` usage), `avoiders` (button_click event structure), `virus` (dynamic agent spawning)
- Course lecture notes and documentation, EE/CSE, University of Washington

---

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
