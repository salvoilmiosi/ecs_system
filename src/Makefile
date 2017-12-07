all: ecs_system.exe

ecs_system.exe: main.o systems.o ecs.o
	g++ -std=c++17 -oecs_system main.o systems.o ecs.o `sdl2-config --libs`

main.o: main.cpp
	g++ -std=c++17 -g -Wall -c -omain.o main.cpp `sdl2-config --cflags`

systems.o: systems.cpp
	g++ -std=c++17 -g -Wall -c -osystems.o systems.cpp `sdl2-config --cflags`

ecs.o: ecs.cpp
	g++ -std=c++17 -g -Wall -c -oecs.o ecs.cpp `sdl2-config --cflags`

