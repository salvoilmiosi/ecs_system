export CFLAGS
export LDFLAGS
export LIBS
export BUILD

export MAINS = src/server_main.cpp src/client_main.cpp
export OUT = ecs_system

ifeq ($(OS),Windows_NT)
	MAKE := mingw32-make
	LIBS := -lmingw32 -lSDL2main -lSDL2 -lSDL2_net -pthread
else
	MAKE := make
	LIBS := -lSDL2 -lSDL2_net -pthread
endif

ifeq ($(BUILD),release)
	CFLAGS := -O2 -Wall --std=c++1z
	LDFLAGS := -s
else
	CFLAGS := -g -Wall --std=c++1z
endif

all: server client

debug:
	$(MAKE) "BUILD=debug"

release:
	$(MAKE) "BUILD=release"

server:
	$(MAKE) -f Makefile.generic "MAIN=server"

client:
	$(MAKE) -f Makefile.generic "MAIN=client"

clean:
	rm -rf bin
	rm -rf obj

.PHONY: debug release server client clean
