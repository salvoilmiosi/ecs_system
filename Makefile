CXX = g++
LD = g++
CFLAGS = -g -Wall --std=c++1z `sdl2-config --cflags`

LDFLAGS =
LIBS = `sdl2-config --libs`

INCLUDE = include
BIN_DIR = bin
OBJ_DIR = obj

OUT_BIN = ecs_system

ifeq ($(OS),Windows_NT)
	OUT_BIN := $(OUT_BIN).exe
endif

$(shell mkdir -p $(BIN_DIR) >/dev/null)
$(shell mkdir -p $(OBJ_DIR) >/dev/null)

DEPFLAGS = -MT $@ -MMD -MP -MF $(OBJ_DIR)/$*.Td

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(patsubst src/%,$(OBJ_DIR)/%.o,$(basename $(SOURCES)))

all: $(BIN_DIR)/$(OUT_BIN)

clean:
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)

$(BIN_DIR)/$(OUT_BIN): $(OBJECTS)
	$(LD) -o $(BIN_DIR)/$(OUT_BIN) $(OBJECTS) $(LDFLAGS) $(LIBS)

$(OBJ_DIR)/%.o : src/%.cpp
$(OBJ_DIR)/%.o : src/%.cpp $(OBJ_DIR)/%.d
	$(CXX) $(DEPFLAGS) $(CFLAGS) -c -I $(INCLUDE) -o $@ $<
	@mv -f $(OBJ_DIR)/$*.Td $(OBJ_DIR)/$*.d

$(OBJ_DIR)/%.d: ;
.PRECIOUS: $(OBJ_DIR)/%.d

-include $(patsubst src/%,$(OBJ_DIR)/%.d,$(basename $(SOURCES)))
