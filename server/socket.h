#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <SDL2/SDL_net.h>

#include <thread>
#include <mutex>
#include <list>

namespace socket {

const char *ipString(const IPaddress &ip);

inline bool operator == (const IPaddress &a, const IPaddress &b) {
	return a.host == b.host && a.port == b.port;
}

static const uint16_t PORT = 2345; 
static const int PACKET_SIZE = 1024;
static const int CHECK_TIMEOUT = 1000;
static const int CLIENT_TIMEOUT = 5000;

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
		serv_thread.join();
	}

	bool open(uint16_t port = PORT);

	void close();

	void run();

	void sendTo(Uint8 *data_ptr, int len, IPaddress addr);
	void sendAll(Uint8 *data_ptr, int len);

private:
	UDPsocket sock = NULL;
	SDLNet_SocketSet sock_set;

	std::thread serv_thread;

	UDPpacket receiver;
	Uint8 pack_data[PACKET_SIZE];

	struct client_info {
		IPaddress address;
		Uint32 last_seen;
	};

	std::list<client_info> clients_connected;

	std::mutex c_mutex;

	void received();

	auto findClient();
	void addClient();
	void stateClient();
	void pingClient();
	void delClient();

	void testClients();
};

}

#endif