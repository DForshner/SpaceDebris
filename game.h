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
      window.setFramerateLimit(60);
    }

    void restartGame() {
      std::cout << "Restarting" << std::endl;

      _state == GameStates::InProgress;

      _player = Player{windowWidth / 2.f, windowHeight - World::defaultHeight - Player::defaultHeight, windowHeight};
      _world = World{windowWidth / 2.f, windowHeight - World::defaultHeight};
      _junk.clear();
      _bullets.clear();

      for (int iX{0}; iX < 3; ++iX) {
        for (int iY{0}; iY < 3; ++iY) {
          float x{(iX + 1) * (Debris::defaultWidth + 5)};
          float y{(iY + 1) * (Debris::defaultHeight + 5)};
          _junk.emplace_back(5 + x, y);
        }
      }
    }

    void runGameLoop() {
      while(true) {
        window.clear(sf::Color::Black);

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

        // If game is paused don't update the entities
        if (_state == GameStates::InProgress) {
          updateEntities();
          handleEntityInteractions();
        }

        if (_world.destroyed) {
          restartGame();
        }

        drawEntities();

        window.display();
      }
    }

  private:
    enum class GameStates{Paused, InProgress};
    GameStates _state{GameStates::InProgress};

    bool _pausedLastFrame{false};

    Player _player{windowWidth / 2.f, windowHeight - World::defaultHeight - Player::defaultHeight, windowHeight};
    World _world{windowWidth / 2.f, windowHeight - World::defaultHeight};
    std::vector<Debris> _junk;
    std::vector<Ball> _bullets;

    sf::RenderWindow window{{windowWidth, windowHeight}, "Space Debris!"};

    void updateEntities() {
      _world.update();

      _player.update();

      for (auto& junk: _junk) {
        junk.update();
      }

      for (auto& bullet: _bullets) {
        bullet.update();
      }
    }

    void drawEntities() {
        _world.draw(window);

        _player.draw(window);

        for (auto& junk: _junk) {
          junk.draw(window);
        }

        for (auto& bullet: _bullets) {
          bullet.draw(window);
        }
    }

    void handleEntityInteractions() {
      // Handle player shooting
      if (_player.isShooting) {
          _bullets.emplace_back(_player.x(), _player.y());
      }

      // O(log(n^2)) :-(
      for (auto& junk: _junk) {
        solveJunkHittingWorld(junk, _world);

        for (auto& bullet: _bullets) {
          solveBulletJunkCollision(junk, bullet);
        }
      }

      // Remove bullets have have gone off screen (Erase-Remove Idiom)
      auto bulletsToRemove = std::remove_if(_bullets.begin(), _bullets.end(),
          [](const Ball& bullet){return bullet.destroyed; });
      _bullets.erase(bulletsToRemove, _bullets.end());

      // Remove debris that was destoryed (Erase-Remove Idiom)
      auto junkToRemove = std::remove_if(_junk.begin(), _junk.end(),
          [](const Debris& junk){return junk.destroyed; });
      _junk.erase(junkToRemove, _junk.end());
    }

    // Return true if two entities are intersecting
    template<typename T1, typename T2>
    bool isIntersecting(const T1& a, const T2& b) noexcept {
      return a.right() >= b.left()
        && a.left() <= b.right()
        && a.bottom() >= b.top()
        && a.top() <= b.bottom();
    }

    void solveBulletJunkCollision(Debris& junk, Ball& bullet) noexcept {
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
};
