#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <SDL2/SDL_net.h>

#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <map>
#include <string>

#include "ecs/world_io.h"
#include "ecs/packet_data.h"

#include "game/userinput.h"

#include "socket.h"

namespace net {

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
		SDLNet_FreeSocketSet(sock_set);
	}

	bool connect(IPaddress addr, const std::string &username);

	bool connect(const std::string &addr, const std::string &username);

	void disconnect();

	bool is_open() {
		return sock != nullptr;
	}

	bool sendDisconnect(const std::string &msg);
	bool sendStatePacket();
	bool sendPing();
	bool sendMessage(const std::string &msg);
	bool sendInputCommand(const game::userinput::command &e);

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
	UDPsocket sock = nullptr;

	SDLNet_SocketSet sock_set;

	IPaddress server_addr;
	std::thread client_thread;
	
	UDPpacket receiver;
	packet_data recv_data;

	std::mutex j_mutex;

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