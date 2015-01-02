// Copyright (c) 2014 David Forshner

#pragma once

#include <SFML/Graphics.hpp>
#include "entities.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <string>

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

      _state = GameStates::InProgress;

      _entityManager.clearAll();

      _entityManager.create<Player>(windowWidth / 2.f, windowHeight - World::defaultHeight - Player::defaultHeight, windowHeight);
      _entityManager.create<World>(windowWidth / 2.f, windowHeight - World::defaultHeight);

      createLargeDebris();
    }

    void runGameLoop() {
      while(true) {
        _frameCounter++;
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

        // Only update game entities if the game is not currently paused.
        if (_state == GameStates::InProgress) {
          _entityManager.updateEntities();

          _entityManager.forEach<Player>([this](auto& player) {
            if (player.isShooting) {
              _entityManager.create<Bullet>(player.x(), player.y());
            }
          });

          _entityManager.forEach<World>([this](auto& world) {
            _entityManager.forEach<Debris>([this, &world](auto& debris) {
              this->solveDebrisHittingWorld(debris, world);
                _entityManager.forEach<Bullet>([this, &debris](auto& bullet) {
                  this->solveBulletDebrisCollision(debris, bullet);
                });
            });

            if (world.destroyed) {
              _state = GameStates::GameOver;
            }
          });

          if (_frameCounter % (60 * 5) == 0) {
            createLargeDebris();
          } else if (_frameCounter % 60 == 0) {
            createSmallDebris();
          }

          _entityManager.clearDestroyed();
        } else if (_state == GameStates::GameOver) {
          restartGame();
        }

        _entityManager.drawEntities(_window);

        _window.display();
      }
    }

  private:
    enum class GameStates{Paused, InProgress, GameOver};
    GameStates _state{GameStates::InProgress};

    bool _pausedLastFrame{false};

    unsigned _frameCounter{0};

    sf::RenderWindow _window{{windowWidth, windowHeight}, "Space Debris!"};

    EntityManager _entityManager;

    std::default_random_engine _positionGenerator;
    std::uniform_int_distribution<int> _xPositionDistro{0, windowWidth};

    std::default_random_engine _debrisSizeGenerator;
    std::uniform_int_distribution<int> _debrisSizeDistro{1, 3};

    // Creates a small piece of debris
    void createSmallDebris() noexcept {
      auto x = getRandomXPosition();
      _entityManager.create<Debris>(x, 0, Debris::MassStates::Med);
    }

    // Creates a debris field
    void createLargeDebris() noexcept {
      constexpr float spacingX{2.f};
      constexpr float spacingY{2.f};

      auto numDebrisX = getRandomDebrisSize();
      auto numDebrisY = getRandomDebrisSize();

      // Get an starting point where all the bricks will fit on screen.
      auto xLeftLimit = Debris::defaultWidth;
      auto xRightLimit = windowWidth - (Debris::defaultWidth + spacingX) * numDebrisX;
      auto xOffset = 0;
      while (xOffset < xLeftLimit || xOffset > xRightLimit) {
          xOffset = getRandomXPosition();
      }

      for (int iX{0}; iX <= numDebrisX; ++iX) {
        for (int iY{0}; iY < numDebrisY; ++iY) {
          float x{(iX) * (Debris::defaultWidth + spacingX)};
          float y{(iY) * (Debris::defaultHeight + spacingY)};
          _entityManager.create<Debris>(xOffset + x, y, Debris::MassStates::Low);
        }
      }
    }

    int getRandomDebrisSize() noexcept {
      return _debrisSizeDistro(_debrisSizeGenerator);
    }

    // Returns a random x position on screen.
    int getRandomXPosition() noexcept {
      return _xPositionDistro(_positionGenerator);
    }

    // Return true if two entities are intersecting
    bool isIntersecting(const auto& a, const auto& b) noexcept {
      return a.right() >= b.left()
        && a.left() <= b.right()
        && a.bottom() >= b.top()
        && a.top() <= b.bottom();
    }

    void solveDebrisHittingWorld(Debris& debris, World& world) noexcept {
      if (debris.bottom() <= world.top()) {
        return;
      }
      debris.destroyed = true;
      --world.hitPoints;
    }

    void solveBulletDebrisCollision(Debris& debris, Bullet& bullet) noexcept {
      if (!isIntersecting(debris, bullet)) {
        return;
      }

      if (debris.mass == Debris::MassStates::High) {
        debris.mass = Debris::MassStates::Med;
        bullet.destroyed = true;
      } else if (debris.mass == Debris::MassStates::Med) {
        debris.mass = Debris::MassStates::Low;
        bullet.destroyed = true;
      } else { // debris.mass == Debris::MassStates::Low
        debris.destroyed = true;
        bullet.destroyed = true;
      }
    }
};
