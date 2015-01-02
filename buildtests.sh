# Builds tests

# Compile
g++ -std=c++1y -c tests.cpp

# Link
g++ tests.o -o runtests -lgtest -lpthread
