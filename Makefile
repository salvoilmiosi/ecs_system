export CFLAGS = -g -Wall --std=c++1z
export LIBS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_net -lpthread

ifeq ($(OS),Windows_NT)
	MAKE := mingw32-make
else
	MAKE := make
endif

all: client server

server:
	$(MAKE) -f Makefile.generic "OUT=ecs_system_server" "SRC=server"

client:
	$(MAKE) -f Makefile.generic "OUT=ecs_system_client" "SRC=client"

clean:
	rm -rf bin
	rm -rf obj

.PHONY: server client clean