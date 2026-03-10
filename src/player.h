#ifndef __PLAYER_AGENT__H
#define __PLAYER_AGENT__H

#include "enviro.h"
using namespace enviro;

/**
 * @brief Controller for the player-controlled character.
 * 
 * Handles arrow key movement, HP tracking, hit cooldown,
 * and responds to game_restart and player_hit events.
 */
class PlayerController : public Process, public AgentInterface {
    public:

    /**
     * @brief Constructor. Initializes velocity, HP, and state flags.
     */
    PlayerController() : Process(), AgentInterface(),
        vx(0), vy(0), alive(true), hp(100), hit_cooldown(0) {}

    /**
     * @brief Sets up all event watchers for keyboard input and game events.
     * 
     * - keydown/keyup: set velocity based on arrow key presses
     * - player_hit: reduce HP by 20; trigger game_over if HP reaches 0
     * - game_restart: reset HP, position, and state for a new round
     */
    void init() {
        // Set velocity on arrow key press
        watch("keydown", [&](Event &e) {
            if (!alive) return;
            auto k = e.value()["key"].get<std::string>();
            if      (k == "ArrowLeft")  vx = -150;
            else if (k == "ArrowRight") vx =  150;
            else if (k == "ArrowUp")    vy = -150;
            else if (k == "ArrowDown")  vy =  150;
        });

        // Stop movement when arrow key is released
        watch("keyup", [&](Event &e) {
            auto k = e.value()["key"].get<std::string>();
            if (k == "ArrowLeft" || k == "ArrowRight") vx = 0;
            if (k == "ArrowUp"   || k == "ArrowDown")  vy = 0;
        });

        // Handle being hit by an obstacle
        watch("player_hit", [&](Event &e) {
            if (!alive || hit_cooldown > 0) return; // Ignore if dead or invincible
            hit_cooldown = 1.0; // 1 second of invincibility after each hit
            hp -= 20;
            if (hp <= 0) {
                // Player is dead
                hp = 0;
                alive = false;
                vx = 0; vy = 0;
                decorate("");
                emit(Event("game_over"));
                label("YOU DIED", -30, -35);
            } else {
                // Still alive - show updated HP and flash red
                label("HP: " + std::to_string(hp), -15, -35);
                decorate("<circle r='22' style='fill:rgba(255,0,0,0.4)'></circle>");
            }
        });

        // Reset state when the game restarts
        watch("game_restart", [&](Event &e) {
            alive = true;
            hp = 100;
            hit_cooldown = 1.5; // Brief invincibility on spawn
            vx = 0; vy = 0;
            teleport(0, 0, 0);
            label("HP: 100", -15, -35);
            decorate("<circle r='22' style='fill:rgba(255,255,0,0.4)'></circle>");
        });
    }

    /**
     * @brief Called once when the agent starts. Sets initial HP label and spawn invincibility.
     */
    void start() {
        label("HP: 100", -15, -35);
        hit_cooldown = 1.5;
        decorate("<circle r='22' style='fill:rgba(255,255,0,0.4)'></circle>");
    }

    /**
     * @brief Called every simulation tick.
     * 
     * Counts down the hit cooldown timer and removes the invincibility
     * decoration when it expires. Moves the player using omni velocity
     * tracking, or damps movement if dead.
     */
    void update() {
        if (hit_cooldown > 0) {
            hit_cooldown -= 0.02;
            // Remove hit flash decoration exactly once when cooldown ends
            if (hit_cooldown <= 0 && alive) {
                hit_cooldown = 0;
                decorate("");
            }
        }
        if (alive) omni_track_velocity(vx, vy, 20);
        else omni_damp_movement();
    }

    void stop() {}

    double vx;          ///< Current horizontal velocity
    double vy;          ///< Current vertical velocity
    double hit_cooldown;///< Seconds of invincibility remaining after a hit
    int hp;             ///< Current hit points (starts at 100, game over at 0)
    bool alive;         ///< Whether the player is currently alive
};

/**
 * @brief Player agent. Wraps PlayerController as an omni-directional dynamic agent.
 */
class Player : public Agent {
    public:
    Player(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }
    private:
    PlayerController c;
};

DECLARE_INTERFACE(Player)
#endif