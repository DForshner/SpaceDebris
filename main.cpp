// Copyright (c) 2014 David Forshner

#include <SFML/Graphics.hpp>
#include "entities.h"
#include <iostream>
#include <algorithm>

constexpr unsigned windowWidth{800};
constexpr unsigned windowHeight{800};

// Return true if two entities are intersecting
template<typename T1, typename T2>
bool isIntersecting(const T1& a, const T2& b) noexcept {
  return a.right() >= b.left()
    && a.left() <= b.right()
    && a.bottom() >= b.top()
    && a.top() <= b.bottom();
}

void solveBallBrickCollision(Brick& brick, Ball& ball) noexcept {
  if (!isIntersecting(brick, ball)) {
    return;
  }
  brick.destroyed = true;
}

int main() {
  std::cout << "Setting Up" << std::endl;

  Player player{windowWidth / 2.f, windowHeight - 50, windowHeight};

  std::vector<Brick> bricks;
  for (int iX{0}; iX < 3; ++iX) {
    for (int iY{0}; iY < 3; ++iY) {
      float x{(iX + 1) * (Brick::defaultWidth + 5)};
      float y{(iY + 1) * (Brick::defaultHeight + 5)};

      bricks.emplace_back(5 + x, y);
    }
  }

  std::vector<Ball> balls;

  sf::RenderWindow window{{windowWidth, windowHeight}, "Wall Bowling"};
  window.setFramerateLimit(60);

  std::cout << "Starting Game Loop" << std::endl;
  while(true) {

    window.clear(sf::Color::Black);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
      break;
    }

    player.update();

    for (auto& brick: bricks) {
      brick.update();
      //solveBallBrickCollision(brick, ball);
    }

    for (auto& ball: balls) {
      ball.update();
      //solveBallBrickCollision(brick, ball);
    }

    player.draw(window);

    for (auto& brick : bricks) {
      brick.draw(window);
    }

    for (auto& ball: balls) {
      ball.draw(window);
    }

    // Handle player shooting
    if (player.isShooting) {
        std::cout << "Shooting!" << std::endl;
        balls.emplace_back(player.x(), player.y());
    }

    // Remove balls have have gone off screen (Erase-Remove Idiom)
    auto ballsToRemove = std::remove_if(balls.begin(), balls.end(), [](const Ball& ball){return ball.destroyed; });
    balls.erase(ballsToRemove, balls.end());

    // Remove bricks that were destoryed (Erase-Remove Idiom)
    auto bricksToRemove = std::remove_if(bricks.begin(), bricks.end(), [](const Brick& brick){return brick.destroyed; });
    bricks.erase(bricksToRemove, bricks.end());

    window.display();
  }

  std::cout << "Exiting" << std::endl;
  return 0;
}
