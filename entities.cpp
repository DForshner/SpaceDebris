// Copyright (c) 2014 David Forshner

#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include "entities.h"

const sf::Color Player::defaultColor{sf::Color::White};

const sf::Color Bullet::defaultColor{sf::Color::White};

const sf::Color Debris::defaultColorHighMass{255, 0, 0, 255};
const sf::Color Debris::defaultColorMedMass{255, 0, 0, 200};
const sf::Color Debris::defaultColorLowMass{255, 0, 0, 100};

const sf::Color World::defaultColorLowDamage{0, 0, 255, 255};
const sf::Color World::defaultColorHighDamage{0, 0, 255, 100};
