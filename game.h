// Copyright (c) 2014 David Forshner

#pragma once

#include <SFML/Graphics.hpp>
#include "entities.h"
#include <algorithm>
#include <iostream>

constexpr unsigned windowWidth{800};
constexpr unsigned windowHeight{800};

// Game keeps track of the game's entities and state.
class Game {
  public:

    Game() {
      _window.setFramerateLimit(60);
    }

    void restartGame() {
      std::cout << "Restarting" << std::endl;

      _state == GameStates::InProgress;

      _entityManager.clearAll();

      _entityManager.create<Player>(windowWidth / 2.f, windowHeight - World::defaultHeight - Player::defaultHeight, windowHeight);
      _entityManager.create<World>(windowWidth / 2.f, windowHeight - World::defaultHeight);

      for (int iX{0}; iX < 3; ++iX) {
        for (int iY{0}; iY < 3; ++iY) {
          float x{(iX + 1) * (Debris::defaultWidth + 5)};
          float y{(iY + 1) * (Debris::defaultHeight + 5)};
          _entityManager.create<Debris>(10 + x, y);
        }
      }
    }

    void runGameLoop() {
      while(true) {
        _window.clear(sf::Color::Black);

        // ESC - Quit game
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
          break;
        }

        // P - Pause game
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P)) {
          // Debounce the pause key
          if (!_pausedLastFrame) {
            if (_state == GameStates::Paused) {
              _state = GameStates::InProgress;
            } else {
              _state = GameStates::Paused;
            }
          }
          _pausedLastFrame = true;
        } else {
          _pausedLastFrame = false;
        }

        // R - Restart game
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
          restartGame();
        }

        bool restart{false};

        // If game is paused don't update the entities
        if (_state == GameStates::InProgress) {
          _entityManager.updateEntities();

          _entityManager.forEach<Player>([this](auto& player) {
            if (player.isShooting) {
              _entityManager.create<Bullet>(player.x(), player.y());
            }
          });

          _entityManager.forEach<World>([this, &restart](auto& world) {
            _entityManager.forEach<Debris>([this, &world](auto& debris) {
              solveJunkHittingWorld(debris, world);
                _entityManager.forEach<Bullet>([this, &debris](auto& bullet) {
                  solveBulletJunkCollision(debris, bullet);
                });
            });

            if (world.destroyed) {
              restart = true;
            }
          });

          _entityManager.clearDestroyed();
        }

        if (restart) {
          restartGame();
        }

        _entityManager.drawEntities(_window);
        _window.display();
      }
    }

  private:
    enum class GameStates{Paused, InProgress};
    GameStates _state{GameStates::InProgress};

    bool _pausedLastFrame{false};

    sf::RenderWindow _window{{windowWidth, windowHeight}, "Space Debris!"};

    EntityManager _entityManager;

};

// Return true if two entities are intersecting
template<typename T1, typename T2>
bool isIntersecting(const T1& a, const T2& b) noexcept {
  return a.right() >= b.left()
    && a.left() <= b.right()
    && a.bottom() >= b.top()
    && a.top() <= b.bottom();
}

void solveBulletJunkCollision(Debris& junk, Bullet& bullet) noexcept {
  if (!isIntersecting(junk, bullet)) {
    return;
  }
  junk.destroyed = true;
  bullet.destroyed = true;
}

void solveJunkHittingWorld(Debris& junk, World& world) noexcept {
  if (junk.bottom() >= windowHeight) {
    junk.destroyed = true;
    world.destroyed = true;
  }
}
