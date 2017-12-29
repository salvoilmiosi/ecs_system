#ifndef __SERVER_H__
#define __SERVER_H__

#include <SDL2/SDL_net.h>

#include <thread>
#include <shared_mutex>
#include <vector>

#include "ecs/world_io.h"

#include "game/userinput.h"

#include "socket.h"

namespace net {

class server_socket {
public:
	server_socket(ecs::world_io<MyComponents> &wld) : wld(wld) {
		sock_set = SDLNet_AllocSocketSet(1);
	}

	~server_socket() {
		close();
	}

	bool open(uint16_t port = PORT);

	void close();

	void sendServerMsg(const std::string &msg);

	void send(const packet_data &packet, IPaddress addr);
	void sendAll(const packet_data &packet);

private:
	void sendRaw(packet_data packet, IPaddress addr);
	void sendSliced(const packet_data &packet, IPaddress addr);

	ecs::world_io<MyComponents> &wld;

	UDPsocket sock = nullptr;
	SDLNet_SocketSet sock_set;

	std::thread serv_thread;

	Uint32 maxPid;

	struct client_info {
		IPaddress address;
		Uint32 last_seen;
		game::userinput::handler input;
	};

	std::vector<client_info> clients_connected;
	std::shared_mutex c_mutex;
	
	void received(UDPpacket &packet);

	void addClient(IPaddress address);

	void parseCommand(client_info &sender, packet_reader &reader);
	void parseInput(client_info &sender, packet_reader &reader);

	void stateClient(client_info &sender);
	void pingClient(client_info &sender);
	void delClient(client_info &sender);

	void testClients();
};

}

#endif