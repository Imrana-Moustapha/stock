CXX = g++
CXXFLAGS = -Iinclude -Iexceptions -Imodels -Iobservers -Irepositories -Iservices -Wall -std=c++23 -MMD -MP

# Fichiers sources basés sur ton arborescence réelle
SRC = main.cpp $(wildcard menu/*.cpp) $(wildcard utils/*.cpp)
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
	rm -f *.o menu/*.o utils/*.o *.d menu/*.d utils/*.d
	rm -f $(TARGET)