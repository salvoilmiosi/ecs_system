export CFLAGS
export LDFLAGS
export LIBS
export BUILD

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
	$(MAKE) -f Makefile.generic "OUT=ecs_system_server" "SRC=server"

client:
	$(MAKE) -f Makefile.generic "OUT=ecs_system_client" "SRC=client"

clean:
	rm -rf bin
	rm -rf obj

.PHONY: debug release server client clean
