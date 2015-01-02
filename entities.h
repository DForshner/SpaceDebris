// Copyright (c) 2014 David Forshner

#pragma once

#include <SFML/Graphics.hpp>
#include "math.h"

class Rectangle {
  public:
    float x() const noexcept { return _body.getPosition().x; }
    float y() const noexcept { return _body.getPosition().y; }
    float width() const noexcept { return _body.getSize().x; }
    float height() const noexcept { return _body.getSize().y; }
    float left() const noexcept { return x() - width() / 2.f; }
    float right() const noexcept { return x() + width() / 2.f; }
    float top() const noexcept { return y() - height() / 2.f; } // Remember sfml y coord are flipped
    float bottom() const noexcept { return y() + height() / 2.f; }

  protected:
    sf::RectangleShape _body;
};

class Circle {
  public:
    float x() const noexcept { return _body.getPosition().x; }
    float y() const noexcept { return _body.getPosition().y; }
    float left() const noexcept { return x() - _body.getRadius(); }
    float right() const noexcept { return x() + _body.getRadius(); }
    float top() const noexcept { return y() - _body.getRadius(); } // Remember sfml y coord are flipped
    float bottom() const noexcept { return y() + _body.getRadius(); }
  protected:
    sf::CircleShape _body;
};

class Player : public Rectangle {
  public:
    static const sf::Color defaultColor;
    static constexpr float defaultWidth{20.f};
    static constexpr float defaultHeight{20.f};
    static constexpr float defaultVelocity {8.f};
    static constexpr unsigned defaultCooldown{15};

    bool isShooting{false};

    Player(float x, float y, float trackWidth) {
      _body.setPosition(x, y);
      _body.setSize({defaultWidth, defaultHeight});
      _body.setFillColor(defaultColor);
      _body.setOrigin(defaultWidth / 2.f, defaultHeight / 2.f);
      _trackWidth = trackWidth;
      _coolDown = 0;
    }

    void update() {
      processPlayerInput();
      _body.move(_velocity);
    }

    void draw(sf::RenderWindow& target) {
      target.draw(_body);
    }

  private:
    sf::Vector2f _velocity;
    unsigned _trackWidth; 
    unsigned _coolDown; 

    void processPlayerInput() {
      // Handle moving left/right
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) && left() > 0) {
        _velocity.x = -defaultVelocity;
      } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) && right() < _trackWidth){
        _velocity.x = defaultVelocity;
      } else {
        _velocity.x = 0.f;
      }

      // Handle fireing
      if (_coolDown > 0) {
        _coolDown--;
        isShooting = false;
      } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
        _coolDown = defaultCooldown;
        isShooting = true;
      }
    }
};

class Ball : public Circle 
{
  public:
    static const sf::Color defaultColor;
    static constexpr float defaultRadius{5.f};
    static constexpr float defaultVelocity{10.f};

    Ball(float x, float y) {
      _body.setPosition(x, y);
      _body.setRadius(defaultRadius);
      _body.setFillColor(defaultColor);
      _body.setOrigin(defaultRadius, defaultRadius);
    }

    bool destroyed{false};

    void update() {
      _body.move(_velocity);
      solveOffScreen();
    }

    void draw(sf::RenderWindow& target) {
      target.draw(_body);
    }

  private:
    sf::Vector2f _velocity{0.f, -defaultVelocity};

    // Handle ball going off screen
    void solveOffScreen() noexcept {
      if (top() < 0) {
        _velocity.y = 0.f;
        destroyed = true;
      }
    }
};

class Brick : public Rectangle {
  public:
    static const sf::Color defaultColor;
    static constexpr float defaultWidth{80.f};
    static constexpr float defaultHeight{30.f};
    static constexpr float defaultVelocity {2.f};

    bool destroyed{false};

    Brick(float x, float y) {
      _body.setPosition(x, y);
      _body.setSize({defaultWidth, defaultHeight});
      _body.setFillColor(defaultColor);
      _body.setOrigin(defaultWidth / 2.f, defaultHeight / 2.f);
    }

    void update() {
      _body.move(_velocity);
    }

    void draw(sf::RenderWindow& target) {
      target.draw(_body);
    }

  private:
    sf::Vector2f _velocity{0.f, +defaultVelocity};
};

class World: public Rectangle {
  public:
    static const sf::Color defaultColor;
    static constexpr float defaultWidth{800.f};
    static constexpr float defaultHeight{30.f};

    World(float x, float y) {
      _body.setPosition(x, y);
      _body.setSize({defaultWidth, defaultHeight});
      _body.setFillColor(defaultColor);
      _body.setOrigin(defaultWidth / 2.f, defaultHeight / 2.f);
    }

    bool destroyed{false};

    void update() {
    }

    void draw(sf::RenderWindow& target) {
      target.draw(_body);
    }
};

inline double squareRoot(const double a) {
  double b = sqrt(a);
  if(b != b) { // nan check
    return -1.0;
  } else {
    return sqrt(a);
  }
}
