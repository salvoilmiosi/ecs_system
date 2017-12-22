#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <SDL2/SDL_net.h>

#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <map>
#include <string>

#include "packet_data.h"
#include "userinput.h"

namespace socket {

static const uint16_t PORT = 2345;
static const int PACKET_SIZE = 1024;
static const int CHECK_TIMEOUT = 1000;
static const int CLIENT_TIMEOUT = 5000;

enum packet_type {
	PACKET_NONE,
	PACKET_SLICED,
	
	PACKET_EDITLOG,
	PACKET_SERVERMSG,

	PACKET_USER_CONNECT,
	PACKET_USER_COMMAND,
	PACKET_USER_INPUT
};

class client_socket {
public:
	client_socket() : recv_data(PACKET_SIZE) {
		sock_set = SDLNet_AllocSocketSet(1);

		receiver.channel = -1;
		receiver.data = recv_data.data();
		receiver.maxlen = PACKET_SIZE;
	}

	~client_socket() {
		disconnect();
	}

	bool connect(IPaddress addr);

	void close(const char *msg);
	void disconnect();

	bool is_open() {
		return sock != NULL;
	}

	bool sendCommand(const std::string &cmd);

	bool sendInputCommand(const userinput::command &e);

	bool send(packet_data data);
	
	template<typename Func>
	void forEachPacket(const Func &func) {
		std::lock_guard lock(j_mutex);
		while (!joined.empty()) {
			func(joined.front());
			joined.pop_front();
		}
	}

private:
	UDPsocket sock = NULL;

	SDLNet_SocketSet sock_set;

	IPaddress server_addr;
	std::thread client_thread;
	
	UDPpacket receiver;
	packet_data recv_data;

	std::mutex j_mutex, s_mutex;

	struct recv_packet {
		uint32_t pid;
		uint8_t count;
		uint8_t slices;

		packet_data data;

		uint32_t time_added;
	};

	std::map<uint32_t, std::vector<recv_packet>> joining;

	std::deque<packet_data> joined;

	void received();
};

}

#endif // __SOCKET_H__