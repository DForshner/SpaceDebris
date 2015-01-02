// Copyright (c) 2014 David Forshner

#include "game.h"
#include <iostream>

int main() {
  Game game;

  std::cout << "Setting Up" << std::endl;
  game.restartGame();

  std::cout << "Starting Game Loop" << std::endl;
  game.runGameLoop();

  std::cout << "Exiting" << std::endl;
  return 0;
}
