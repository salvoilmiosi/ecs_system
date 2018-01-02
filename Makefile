CXX = g++
LD = g++
MAKE = make
CFLAGS = -Wall --std=c++1z
LDFLAGS =
LIBS = -lSDL2 -lSDL2_net -lSDL2_ttf -pthread
CLIENT_LIBS = 
SRC = src
OUT = out
INC_DIR = include $(SRC)
BIN_DIR = bin
OBJ_DIR = obj
RES_DIR = resource
MAINS = server client
BUILDS = debug release
OUT = ecs_system

ifdef BUILD
	BIN_DIR := $(BUILD)/$(BIN_DIR)
	OBJ_DIR := $(BUILD)/$(OBJ_DIR)
endif

ifeq ($(BUILD),release)
	CFLAGS += -O2
	LDFLAGS += -s
else
	CFLAGS += -g
endif

ifeq ($(TARGET),win32)
	CXX := i686-w64-mingw32-g++
	LD := i686-w64-mingw32-g++
endif

ifeq ($(TARGET),win64)
	CXX := x86_64-w64-mingw32-g++
	LD := x86_64-w64-mingw32-g++
endif

ifeq ($(OS),Windows_NT)
	MAKE := mingw32-make
	LIBS := -lmingw32 -lSDL2main $(LIBS)
	BIN_EXT := .exe
	CLIENT_LIBS += -mwindows
endif

all: $(MAINS) copy_resources

$(BUILDS):
	$(MAKE) "BUILD=$@"

RESOURCES = $(wildcard $(RES_DIR)/*)
OUT_RESOURCES = $(patsubst $(RES_DIR)/%,$(BIN_DIR)/%,$(RESOURCES))

copy_resources: $(OUT_RESOURCES)
$(OUT_RESOURCES): $(RESOURCES)
	cp $(RESOURCES) $(OUT_RESOURCES)

DEPFLAGS = -MT $@ -MMD -MP -MF $(OBJ_DIR)/$*.Td

SOURCES = $(wildcard $(SRC)/*.cpp) $(wildcard $(SRC)/**/*.cpp)
SOURCES_NOMAINS = $(filter-out $(patsubst %,$(SRC)/main_%.cpp,$(MAINS)),$(SOURCES))

OBJECTS = $(patsubst $(SRC)/%,$(OBJ_DIR)/%.o,$(basename $(SOURCES)))
OBJECTS_NOMAINS = $(patsubst $(SRC)/%,$(OBJ_DIR)/%.o,$(basename $(SOURCES_NOMAINS)))

DEPS = $(patsubst $(SRC)/%,$(OBJ_DIR)/%.d,$(basename $(SOURCES)))

BINARIES = $(patsubst %,$(BIN_DIR)/$(OUT)_%$(BIN_EXT),$(MAINS))

server: $(BIN_DIR)/$(OUT)_server$(BIN_EXT)
$(BIN_DIR)/$(OUT)_server$(BIN_EXT): $(OBJECTS_NOMAINS) $(OBJ_DIR)/main_server.o
	@mkdir -p $(dir $@)
	$(LD) -o $@ $^ $(LDFLAGS) $(LIBS)

client: $(BIN_DIR)/$(OUT)_client$(BIN_EXT)
$(BIN_DIR)/$(OUT)_client$(BIN_EXT): $(OBJECTS_NOMAINS) $(OBJ_DIR)/main_client.o
	@mkdir -p $(dir $@)
	$(LD) -o $@ $^ $(LDFLAGS) $(LIBS) $(CLIENT_LIBS)

$(OBJ_DIR)/%.o: $(SRC)/%.cpp $(OBJ_DIR)/%.d
	@mkdir -p $(dir $@)
	$(CXX) -c -o $@ $(DEPFLAGS) $(CFLAGS) $(addprefix -I,$(INC_DIR)) $<
	@mv -f $(OBJ_DIR)/$*.Td $(OBJ_DIR)/$*.d

$(OBJ_DIR)/%.d: ;

.SECONDARY: $(OBJECTS) $(DEPS)

-include $(DEPS)

clean:
	rm -f $(OBJECTS)
	rm -f $(DEPS)
	rm -f $(BINARIES)
	rm -f $(OUT_RESOURCES)

.PHONY: $(MAINS) $(BUILDS) clean
