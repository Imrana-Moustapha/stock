CXX = g++
CXXFLAGS = -I../include -I. -Wall -std=c++23

SRC = $(wildcard *.cpp) vendor/catch_amalgamated.cpp ../src/utils/saisie.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = tests

.PHONY: run clean

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET)

clean:
	rm -f *.o vendor/*.o ../src/utils/saisie.o $(TARGET)