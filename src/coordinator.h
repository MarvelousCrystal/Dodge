#ifndef __COORDINATOR_AGENT__H
#define __COORDINATOR_AGENT__H

#include "enviro.h"
#include <math.h>
#include <stdlib.h>
using namespace enviro;

/**
 * @brief Invisible game manager agent that controls spawning, difficulty, and restart.
 * 
 * The Coordinator runs as an invisible agent and manages the overall game loop:
 * - Spawns Obstacle agents at random positions on the arena boundary
 * - Scales spawn rate over time to increase difficulty
 * - Listens for button_click to trigger game restart
 * - Listens for game_over to pause spawning
 */
class CoordinatorController : public Process, public AgentInterface {
    public:

    /**
     * @brief Constructor. Initializes timing and game state variables.
     */
    CoordinatorController() : Process(), AgentInterface(),
        elapsed(0), spawn_timer(0), spawn_interval(1.5), running(false) {}

    /**
     * @brief Sets up event watchers for button clicks and game over.
     * 
     * - button_click with value "restart": resets timers and emits game_restart
     * - game_over: pauses obstacle spawning
     */
    void init() {
        watch("button_click", [&](Event &e) {
            if (e.value()["value"] == "restart") {
                // Reset all timing and restart the game
                elapsed = 0;
                spawn_timer = spawn_interval; // Spawn an obstacle immediately
                running = true;
                emit(Event("game_restart"));
            }
        });

        // Stop spawning when player dies
        watch("game_over", [&](Event &e) {
            running = false;
        });
    }

    /**
     * @brief Called once at simulation start. Begins spawning immediately.
     */
    void start() {
        running = true;
        spawn_timer = spawn_interval;
    }

    /**
     * @brief Called every tick. Manages spawn timing and difficulty scaling.
     * 
     * Spawn interval decreases from 1.5s to a minimum of 0.8s as elapsed
     * time increases, making the game progressively harder.
     * Obstacles are spawned at random positions along the arena boundary.
     */
    void update() {
        if (!running) return;

        elapsed += 0.02;     // Each tick is ~20ms
        spawn_timer += 0.02;

        // Scale difficulty: reduce spawn interval over time (min 0.8s)
        spawn_interval = fmax(0.8, 1.5 - (elapsed / 30.0));

        if (spawn_timer >= spawn_interval) {
            spawn_timer = 0;
            spawn_obstacle();
        }
    }

    void stop() {}

    private:

    /**
     * @brief Spawns a new Obstacle agent at a random position on the arena boundary.
     * 
     * Picks one of four edges (top, bottom, left, right) at random,
     * then places the obstacle just outside that edge.
     */
    void spawn_obstacle() {
        const double W = 450; ///< Half-width of the arena
        const double H = 260; ///< Half-height of the arena
        double ox, oy;

        int side = rand() % 4;
        switch(side) {
            case 0: ox = (rand()%(int)(2*W))-W; oy = -H; break; // Top edge
            case 1: ox = (rand()%(int)(2*W))-W; oy =  H; break; // Bottom edge
            case 2: ox = -W; oy = (rand()%(int)(2*H))-H; break; // Left edge
            default:ox =  W; oy = (rand()%(int)(2*H))-H; break; // Right edge
        }

        add_agent("Obstacle", ox, oy, 0, {{"fill","#e67e22"},{"stroke","#d35400"}});
    }

    double elapsed;        ///< Total seconds elapsed since game start
    double spawn_timer;    ///< Seconds since last obstacle was spawned
    double spawn_interval; ///< Current interval between spawns (decreases over time)
    bool running;          ///< Whether the game is currently active
};

/**
 * @brief Coordinator agent. Invisible game manager with no physical presence.
 */
class Coordinator : public Agent {
    public:
    Coordinator(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }
    private:
    CoordinatorController c;
};

DECLARE_INTERFACE(Coordinator)
#endif