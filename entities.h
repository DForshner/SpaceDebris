// Copyright (c) 2014 David Forshner

#pragma once

#include <memory>
#include <vector>
#include <iostream>
#include <map>
#include <typeinfo>
#include <SFML/Graphics.hpp>
#include "math.h"

// Entity interface that all game entities derive from.
class Entity {
  public:
    // True if entity is destroyed
    bool destroyed{false};

    virtual ~Entity() { }
    virtual void update() = 0;
    virtual void draw(sf::RenderWindow& target) = 0;
};

// Responsible for managing the lifecycle of entities in the game.  
// Handles creating, updating and removing them from memory.
class EntityManager {
  public:
    // Create entities with constructor signature
    // 'T' game object type
    // 'TArgs' constructor arguments types as template parameters.
    // Returns reference to heap allocated object.
    template<typename T, typename... TArgs>
    T& create(TArgs&&... mArgs) {
      static_assert(std::is_base_of<Entity, T>::value,
          "T must be derived from type 'Entity'");

      // Use perfect forwarding for T's constructor arguments.
      auto owning(std::make_unique<T>(std::forward<TArgs>(mArgs)...));

      // Add a non-owning pointer to instance in the entity lookup table.
      auto nonOwning(owning.get());
      auto key = typeid(T).hash_code(); // Bug: hash_code() will not guarantee unique value per type.
      _entityLookup[key].emplace_back(nonOwning);

      // Move the unique pointer to the entity store.
      _entities.emplace_back(std::move(owning));

      return *nonOwning;
    }

    // Removes all destroyed entities from memory.
    void clearDestroyed() {

      // Remove non-owning pointers to any destroyed entities.  This is only deleting
      // the non-owning pointer stored in the lookup not the heap allocated entity.
      for(auto& pair : _entityLookup) {
        auto& entities(pair.second);

        entities.erase( // Note: Erase-remove idiom
            std::remove_if(std::begin(entities), std::end(entities),
              [](const auto entity){ return entity->destroyed; }), // Note: C++14 generic lambda
            std::end(entities)
        );
      }

      // Delete the smart pointer that holds the heap allocated entity
      // which will automatically free the memory.
      _entities.erase( // Note: Erase-remove idiom
          std::remove_if(std::begin(_entities), std::end(_entities),
            [](const auto& entity){ return entity->destroyed; }), // Note C++14 generic lambda
          std::end(_entities)
      );
    }

    // Destroy all entities
    void clearAll() {
      _entityLookup.clear();
      _entities.clear(); // Deletes smart pointers which will auto free memory.
    }

    // Returns all instances of this entity type.
    template<typename T> // Template method
    auto& find() { // Note: C++14 automatic function return type deduction
      auto key = typeid(T).hash_code();
      return _entityLookup[key];
    }

    // Apply arbitary code on each entity of a type
    template<typename T, typename TFunc>
    void forEach(const TFunc& func) {
      auto& entities(find<T>());

      for(auto entity : entities) {
        // Cast the instance pointer to its "real" type.
        auto instance = reinterpret_cast<T*>(entity);
        // Call function with dereferenced casted pointer.
        func(*instance);
      }
    }

  void updateEntities() {
    for (auto& entity : _entities) {
      entity->update();
    }
  }

  void drawEntities(sf::RenderWindow& target) {
    for (auto& entity : _entities) {
      entity->draw(target);
    }
  }

  private:
    std::vector<std::unique_ptr<Entity>> _entities;

    // A map of entity 'typeid' hashes to a vector of non-owning pointers
    // to instances of that type.
    std::map<std::size_t, std::vector<Entity*>> _entityLookup;
};

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

class Player : public Entity, public Rectangle {
  public:
    static const sf::Color defaultColor;
    static constexpr float defaultWidth{20.f};
    static constexpr float defaultHeight{20.f};
    static constexpr float defaultVelocity {8.f};
    static constexpr unsigned defaultCooldown{10};

    bool isShooting{false};

    Player(float x, float y, float trackWidth) {
      _body.setPosition(x, y);
      _body.setSize({defaultWidth, defaultHeight});
      _body.setFillColor(defaultColor);
      _body.setOrigin(defaultWidth / 2.f, defaultHeight / 2.f);
      _trackWidth = trackWidth;
      _coolDown = 0;
    }

    void update() override { // Note: C++11 override ensures overriding a virtual method.
      processPlayerInput();
      _body.move(_velocity);
    }

    void draw(sf::RenderWindow& target) override { // Note: C++11 override ensures overriding a virtual method.
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

class Bullet: public Entity, public Circle
{
  public:
    static const sf::Color defaultColor;
    static constexpr float defaultRadius{5.f};
    static constexpr float defaultVelocity{10.f};

    Bullet(float x, float y) {
      _body.setPosition(x, y);
      _body.setRadius(defaultRadius);
      _body.setFillColor(defaultColor);
      _body.setOrigin(defaultRadius, defaultRadius);
    }

    void update() override { // Note: override ensures overriding a virtual method.
      _body.move(_velocity);
      solveOffScreen();
    }

    void draw(sf::RenderWindow& target) override { // Note: override ensures overriding a virtual method.
      target.draw(_body);
    }

  private:
    sf::Vector2f _velocity{0.f, -defaultVelocity};

    // Handle bullet going off screen
    void solveOffScreen() noexcept {
      if (top() < 0) {
        _velocity.y = 0.f;
        destroyed = true;
      }
    }
};

class Debris: public Entity, public Rectangle {
  public:
    static const sf::Color defaultColorHighMass;
    static const sf::Color defaultColorMedMass;
    static const sf::Color defaultColorLowMass;
    static constexpr float defaultWidth{80.f};
    static constexpr float defaultHeight{30.f};
    static constexpr float defaultVelocity {2.f};

    enum class MassStates{High, Med, Low};
    MassStates mass{MassStates::Low};

    Debris(float x, float y, MassStates startingMass) {
      mass = startingMass;
      _body.setFillColor(defaultColorLowMass);
      _body.setPosition(x, y);
      _body.setSize({defaultWidth, defaultHeight});
      _body.setOrigin(defaultWidth / 2.f, defaultHeight / 2.f);
    }

    void update() override {
      _body.move(_velocity);

      if (mass == MassStates::High) {
        _body.setFillColor(defaultColorHighMass);
      } else if (mass == MassStates::Med) {
        _body.setFillColor(defaultColorMedMass);
      } else { // mass == MassStates::Low
        _body.setFillColor(defaultColorLowMass);
      }
    }

    void draw(sf::RenderWindow& target) override {
      target.draw(_body);
    }

  private:
    sf::Vector2f _velocity{0.f, +defaultVelocity};
};

class World: public Entity, public Rectangle {
  public:
    static const sf::Color defaultColorLowDamage;
    static const sf::Color defaultColorHighDamage;
    static constexpr float defaultWidth{800.f};
    static constexpr float defaultHeight{30.f};
    static constexpr int defaultHitPoints{10};

    int hitPoints{defaultHitPoints};
    int hitPointsLastFrame{defaultHitPoints};

    World(float x, float y) {
      _body.setPosition(x, y);
      _body.setSize({defaultWidth, defaultHeight});
      _body.setFillColor(defaultColorLowDamage);
      _body.setOrigin(defaultWidth / 2.f, defaultHeight / 2.f);
    }

    void update() override {
      if (hitPoints == 0) {
        destroyed = true;
      } else if (hitPoints != hitPointsLastFrame) {
        auto intensity = ((float)hitPoints / (float)defaultHitPoints) * 255;
        std::cout << "Int: " << intensity << "/255" << std::endl;
        _body.setFillColor({0, 0, 255, sf::Uint8(intensity)});
      }

      hitPointsLastFrame = hitPoints;
    }

    void draw(sf::RenderWindow& target) override {
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
