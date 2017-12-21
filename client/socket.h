#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <SDL2/SDL_net.h>

#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <list>
#include <string>

#include "packet_data.h"
#include "userinput.h"

namespace socket {

static const uint16_t PORT = 2345;
static const int PACKET_SIZE = 1024;
static const int CHECK_TIMEOUT = 1000;
static const int CLIENT_TIMEOUT = 5000;

static const uint8_t COMMAND_HANDLE = 0xec;
static const uint8_t INPUT_HANDLE = 0xc5;

class packet_joiner {
public:
	void add(UDPpacket pack);
	
	template<typename Func>
	void forEachJoined(Func func) {
		while (!joined.empty()) {
			func(joined.front());
			joined.pop_front();
		}
	}

private:
	struct packet {
		Uint32 pid;
		Uint8 count;
		Uint8 slices;
		Uint32 time_added;
		packet_data data;
		size_t len;
	};

	std::list<packet> packets;

	typedef std::list<packet>::iterator packet_it;

	std::deque<packet_data> joined;

	void findJoin(Uint32 time);

	void join(std::vector<packet_it> &sameId);
};

class client_socket {
public:
	client_socket() {
		sock_set = SDLNet_AllocSocketSet(1);

		receiver.channel = -1;
		receiver.data = pack_data;
		receiver.maxlen = PACKET_SIZE;
	}

	~client_socket() {
		disconnect();
	}

	bool connect(IPaddress addr);

	void disconnect();

	bool sendCommand(const std::string &cmd);

	bool sendInputCommand(const userinput::command &e);

	bool send(packet_data data);
	
	template<typename Func>
	void forEachPacket(Func func) {
		std::lock_guard lock(j_mutex);
		joiner.forEachJoined(func);
	}

private:
	UDPsocket sock = NULL;

	SDLNet_SocketSet sock_set;

	IPaddress server_addr;
	std::thread client_thread;
	
	UDPpacket receiver;
	Uint8 pack_data[PACKET_SIZE];

	packet_joiner joiner;

	std::mutex j_mutex;

	void received() {
		std::lock_guard lock(j_mutex);
		joiner.add(receiver);
	}
};

}

#endif // __SOCKET_H__