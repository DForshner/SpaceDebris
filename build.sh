# Quick and dirty build script

# Compile
g++ -std=c++1y -c main.cpp

# Link
g++ main.o -o game -lsfml-graphics -lsfml-window -lsfml-system
