# Quick and dirty build script

# Compile
g++ -std=c++1y -c main.cpp entities.cpp

# Link
g++ main.o entities.o -o spacedebrisgame -lsfml-graphics -lsfml-window -lsfml-system
