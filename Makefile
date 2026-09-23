CXX = g++
CXXFLAGS = -Iinclude -Wall -std=c++23 -MMD -MP
SRC = $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
OBJ = $(SRC:.cpp=.o)
DEP = $(OBJ:.o=.d)
BIN_DIR = bin
TARGET = $(BIN_DIR)/mon_programme

.PHONY: all build run clean

# "make" tout court compile (si besoin) puis lance l'exécutable
all: run

# Compile sans lancer le programme
build: $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CXX) $(OBJ) -o $(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

-include $(DEP)

clean:
	rm -f src/*.o src/*/*.o src/*.d src/*/*.d
	rm -f $(TARGET)