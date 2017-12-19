#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <SDL2/SDL_net.h>

#include <thread>
#include <mutex>
#include <list>

#include "packet_data.h"
#include "userinput.h"

namespace socket {

const char *ipString(const IPaddress &ip);

inline bool operator == (const IPaddress &a, const IPaddress &b) {
	return a.host == b.host && a.port == b.port;
}

static const uint16_t PORT = 2345; 
static const int PACKET_SIZE = 1024;
static const int CHECK_TIMEOUT = 1000;
static const int CLIENT_TIMEOUT = 5000;

static const uint8_t COMMAND_HANDLE = 0xec;
static const uint8_t INPUT_HANDLE = 0xc5;

class server_socket {
public:
	server_socket() {
		sock_set = SDLNet_AllocSocketSet(1);

		memset(&receiver, 0, sizeof(receiver));

		receiver.channel = -1;
		receiver.data = pack_data;
		receiver.maxlen = PACKET_SIZE;
	}

	~server_socket() {
		close();
	}

	bool open(uint16_t port = PORT);

	void close();

	void sendTo(const packet_data &packet, IPaddress addr);
	void sendAll(const packet_data &packet);

private:
	UDPsocket sock = NULL;
	SDLNet_SocketSet sock_set;

	std::thread serv_thread;

	UDPpacket receiver;
	Uint8 pack_data[PACKET_SIZE];
	Uint32 maxPid;

	struct client_info {
		IPaddress address;
		Uint32 last_seen;
		userinput input;
	};

	std::list<client_info> clients_connected;

	std::mutex c_mutex;

	void received() {
		std::lock_guard lock(c_mutex);

		switch (pack_data[0]) {
		case COMMAND_HANDLE:
			parseCommand();
			break;
		case INPUT_HANDLE:
			parseInput();
			break;
		}
	}

	auto findClient();

	void parseCommand();
	void parseInput();

	void addClient();
	void stateClient();
	void pingClient();
	void delClient();

	void testClients();
};

}

#endif