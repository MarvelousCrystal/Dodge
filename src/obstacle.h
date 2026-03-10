#ifndef __OBSTACLE_AGENT__H
#define __OBSTACLE_AGENT__H

#include "enviro.h"
#include <math.h>
using namespace enviro;

/**
 * @brief Controller for obstacle agents that rush toward the center of the arena.
 * 
 * Each obstacle calculates its direction toward the center at spawn time,
 * then moves in a straight line at fixed speed. Obstacles emit a player_hit
 * event on collision with the player, and remove themselves after a set
 * lifetime or when a game_restart event is received.
 */
class ObstacleController : public Process, public AgentInterface {
    public:

    /**
     * @brief Constructor. Initializes state flags, tick counter, and velocity.
     */
    ObstacleController() : Process(), AgentInterface(),
        active(true), ticks(0), vx(0), vy(0), removing(false) {}

    /**
     * @brief Sets up collision detection and restart event handler.
     * 
     * - Collision with Player: emits player_hit event
     * - game_restart: safely removes this obstacle agent
     */
    void init() {
        // Notify the game when this obstacle hits the player
        notice_collisions_with("Player", [&](Event &e) {
            if (active) emit(Event("player_hit"));
        });

        // Remove self when game restarts (with double-removal guard)
        watch("game_restart", [&](Event &e) {
            if (active && !removing) {
                removing = true;
                active = false;
                remove_agent(id());
            }
        });
    }

    /**
     * @brief Called once at spawn. Calculates fixed velocity vector toward center (0, 0).
     * 
     * Direction is computed once so the obstacle moves in a straight line
     * without jitter from per-frame recalculation.
     */
    void start() {
        double x = position().x, y = position().y;
        double dist = sqrt(x*x + y*y);
        if (dist > 0) {
            double speed = 60; ///< Units per second toward center
            vx = -x / dist * speed;
            vy = -y / dist * speed;
        }
    }

    /**
     * @brief Called every simulation tick. Moves obstacle and handles lifetime expiry.
     * 
     * Uses omni_track_velocity to maintain constant speed. Removes the agent
     * after 250 ticks (~5 seconds) to prevent arena clutter.
     */
    void update() {
        if (!active) return;
        ticks++;
        omni_track_velocity(vx, vy, 5);

        // Remove after ~5 seconds if not already removed
        if (ticks > 250 && !removing) {
            removing = true;
            active = false;
            remove_agent(id());
        }
    }

    void stop() {}

    bool active;    ///< Whether this obstacle is still in play
    bool removing;  ///< Guard flag to prevent double remove_agent() calls
    int ticks;      ///< Tick counter used for lifetime expiry
    double vx;      ///< Fixed horizontal velocity component
    double vy;      ///< Fixed vertical velocity component
};

/**
 * @brief Obstacle agent. Spawned dynamically by the Coordinator.
 * 
 * Each instance is an omni-directional dynamic agent that moves
 * in a straight line from the arena edge toward the center.
 */
class Obstacle : public Agent {
    public:
    Obstacle(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }
    private:
    ObstacleController c;
};

DECLARE_INTERFACE(Obstacle)
#endif