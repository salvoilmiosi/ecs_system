CXX = g++
LD = g++
CFLAGS = -Wall --std=c++1z `sdl2-config --cflags`

LDFLAGS =
LIBS = `sdl2-config --libs`

INCLUDE = include
BIN_DIR = bin
OBJ_DIR = obj

ifeq ($(BUILD),release)
	CFLAGS += -O2 -s
	OBJ_DIR = release/obj
	BIN_DIR = release/bin
else
	CFLAGS += -g
endif

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
debug: all

$(BIN_DIR)/$(OUT_BIN): $(OBJECTS)
	$(LD) -o $(BIN_DIR)/$(OUT_BIN) $(OBJECTS) $(LDFLAGS) $(LIBS)

$(OBJ_DIR)/%.o : src/%.cpp
$(OBJ_DIR)/%.o : src/%.cpp $(OBJ_DIR)/%.d
	$(CXX) $(DEPFLAGS) $(CFLAGS) -c -I $(INCLUDE) -o $@ $<
	@mv -f $(OBJ_DIR)/$*.Td $(OBJ_DIR)/$*.d

$(OBJ_DIR)/%.d: ;
.PRECIOUS: $(OBJ_DIR)/%.d

-include $(patsubst src/%,$(OBJ_DIR)/%.d,$(basename $(SOURCES)))

release:
	make "BUILD=release"

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(OBJ_DIR)/*.d
	rm -f $(BIN_DIR)/$(OUT_BIN)
	rmdir $(OBJ_DIR)
	rmdir $(BIN_DIR)

clean_debug: clean

clean_release:
	make "BUILD=release" clean
	rmdir release

.PHONY: debug release clean clean_release
