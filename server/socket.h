#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <SDL2/SDL_net.h>

#include <thread>
#include <mutex>
#include <vector>

#include "packet_data.h"
#include "components_serial.h"
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

enum packet_type {
	PACKET_NONE,
	PACKET_SLICED,

	PACKET_EDITLOG,
	PACKET_SERVERMSG,

	PACKET_USER_CONNECT,
	PACKET_USER_COMMAND,
	PACKET_USER_INPUT
};

class server_socket {
public:
	server_socket() {
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

	UDPsocket sock = NULL;
	SDLNet_SocketSet sock_set;

	std::thread serv_thread;

	Uint32 maxPid;

	struct client_info {
		IPaddress address;
		Uint32 last_seen;
		userinput input;
	};

	std::vector<client_info> clients_connected;
	std::mutex c_mutex;

	void received(UDPpacket &packet);
	void addClient(IPaddress address);

	void parseCommand(client_info &sender, packet_data_in &reader);
	void parseInput(client_info &sender, packet_data_in &reader);

	void stateClient(client_info &sender);
	void pingClient(client_info &sender);
	void delClient(client_info &sender);

	void testClients();
};

}

#endif