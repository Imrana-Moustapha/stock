CXX = g++
CXXFLAGS = -Iinclude -Wall -std=c++23 -MMD -MP
SRC = $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
OBJ = $(SRC:.cpp=.o)
DEP = $(OBJ:.o=.d)
TARGET = mon_programme

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET)

-include $(DEP)

clean:
	rm -f src/*.o src/*/*.o src/*.d src/*/*.d $(TARGET)