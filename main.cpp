//
// Compiled using GCC 4.82
// Requires SFML library

#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>

constexpr unsigned windowWidth{800};
constexpr unsigned windowHeight{800};

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

class Player : public Rectangle {
  public:
    static const sf::Color defaultColor;
    static constexpr float defaultWidth{20.f};
    static constexpr float defaultHeight{20.f};
    static constexpr float defaultVelocity {8.f};

    bool isShooting{false};

    Player(float x, float y) {
      _body.setPosition(x, y);
      _body.setSize({defaultWidth, defaultHeight});
      _body.setFillColor(defaultColor);
      _body.setOrigin(defaultWidth / 2.f, defaultHeight / 2.f);
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

    void processPlayerInput() {
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) && left() > 0) {
        std::cout << "Left: " << x() << " / 0" << std::endl;
        _velocity.x = -defaultVelocity;
      } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) && right() < windowWidth){
        std::cout << "Right: " << x() << " / " << windowWidth << std::endl;
        _velocity.x = defaultVelocity;
      } else {
        _velocity.x = 0.f;
      }

      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
        isShooting = true;
      } else {
        isShooting = false;
      }
    }
};

const sf::Color Player::defaultColor{sf::Color::Blue};

class Ball
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

    float x() const noexcept { return _body.getPosition().x; }
    float y() const noexcept { return _body.getPosition().y; }
    float left() const noexcept { return x() - _body.getRadius(); }
    float right() const noexcept { return x() + _body.getRadius(); }
    float top() const noexcept { return y() - _body.getRadius(); } // Remember sfml y coord are flipped
    float bottom() const noexcept { return y() + _body.getRadius(); }

  private:
    sf::CircleShape _body;
    sf::Vector2f _velocity{0.f, -defaultVelocity};

    // Handle ball going off screen
    void solveOffScreen() noexcept {
      if (top() < 0) {
        _velocity.y = 0.f;
        destroyed = true;
      }
    }
};

const sf::Color Ball::defaultColor{sf::Color::White};

class Brick : public Rectangle {
  public:
    static const sf::Color defaultColor;
    static constexpr float defaultWidth{80.f};
    static constexpr float defaultHeight{30.f};
    static constexpr float defaultVelocity {10.f};

    bool destroyed{false};

    Brick(float x, float y) {
      _body.setPosition(x, y);
      _body.setSize({defaultWidth, defaultHeight});
      _body.setFillColor(defaultColor);
      _body.setOrigin(defaultWidth / 2.f, defaultHeight / 2.f);
    }

    void update() { }

    void draw(sf::RenderWindow& target) {
      target.draw(_body);
    }

  private:
    sf::Vector2f _velocity;
};

const sf::Color Brick::defaultColor{sf::Color::Red};

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

  Player player{windowWidth / 2.f, windowHeight - 50};

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
