#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <SDL2/SDL_net.h>

#include <thread>
#include <deque>
#include <vector>
#include <list>

namespace socket {

static const uint16_t PORT = 2345;
static const int PACKET_SIZE = 1024;
static const int CHECK_TIMEOUT = 1000;
static const int CLIENT_TIMEOUT = 5000;

class packet_joiner {
public:
	void add(UDPpacket pack) {
		packet p;
		p.len = pack.len - sizeof(p.header);
		p.time_added = SDL_GetTicks();

		memset(p.data, 0, PACKET_SIZE);
		memcpy(&p.header, pack.data, sizeof(p.header));
		memcpy(p.data, pack.data + sizeof(p.header), p.len);

		packets.push_back(p);

		findJoin(p.header.time);

		if (!packets.empty() && SDL_GetTicks() - packets.front().time_added > CLIENT_TIMEOUT) {
			packets.pop_front();
		}
	}
	
	template<typename Func>
	void forEachJoined(Func func) {
		while (!joined.empty()) {
			func(joined.front().data);
			joined.pop_front();
		}
	}

private:
	struct packet {
		struct {
			Uint32 time;
			Uint8 count;
			Uint8 slices;
		} header;
		Uint32 time_added;
		Uint8 data[PACKET_SIZE];
		size_t len;
	};

	struct packet_joined {
		Uint32 time;
		std::string data;
	};

	std::list<packet> packets;

	typedef decltype(packets)::iterator packet_it;

	std::deque<packet_joined> joined;

	void findJoin(Uint32 time) {
		std::vector<packet_it> sameTime;
		for (auto it = packets.rbegin(); it != packets.rend(); ++it) {
			if (it->header.time == time) {
				sameTime.push_back(std::next(it).base());
				if (sameTime.size() >= it->header.slices) {
					join(sameTime);
					break;
				}
			}
		}
	}

	void join(std::vector<packet_it> &sameTime) {
		std::sort(sameTime.begin(), sameTime.end(), [](auto &a, auto &b) {
			return a->header.count < b->header.count;
		});

		packet_joined pj;
		pj.time = sameTime.front()->header.time;
		for (auto &x : sameTime) {
			pj.data.append((char *)x->data, x->len);
			packets.erase(x);
		}

		joined.push_back(pj);
		std::sort(joined.begin(), joined.end(), [](auto &a, auto &b) {
			return a.time < b.time;
		});
	}
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
		close();
		client_thread.join();
	}

	bool connect(IPaddress addr) {
		server_addr = addr;

		sock = SDLNet_UDP_Open(0);
		SDLNet_UDP_Bind(sock, 0, &addr);

		if (!sock) {
			std::cerr << "Could not open socket: " << SDLNet_GetError() << std::endl;
			return false;
		}

		sendChar('c');

		SDLNet_UDP_AddSocket(sock_set, sock);
		return true;
	}

	void close() {
		if (sock) {
			disconnect();
			SDLNet_UDP_DelSocket(sock_set, sock);
			SDLNet_UDP_Close(sock);
			sock = NULL;
		}
	}

	void disconnect() {
		sendChar('d');
		memset(&server_addr, 0, sizeof(server_addr));
	}

	bool sendChar(const char c) {
		return send(reinterpret_cast<const Uint8*>(&c), 1);
	}

	bool send(const Uint8 *data, size_t len) {
		UDPpacket packet;
		memset(&packet, 0, sizeof(packet));

		Uint8 data_copy[PACKET_SIZE];

		packet.channel = 0;
		packet.data = data_copy;
		packet.maxlen = PACKET_SIZE;
		packet.len = len;

		memset(data_copy, 0, PACKET_SIZE);
		memcpy(data_copy, data, len);

		if (!SDLNet_UDP_Send(sock, packet.channel, &packet)) {
			std::cout << "Packet lost" << std::endl;
			return false;
		}
		return true;
	}

	void run() {
		client_thread = std::thread([this]() {
			while (sock) {
				int numready = SDLNet_CheckSockets(sock_set, CHECK_TIMEOUT);

				if (numready > 0) {
					memset(pack_data, 0, PACKET_SIZE);
					if (SDLNet_UDP_Recv(sock, &receiver)) {
						received();
					} else {
						// Server disconnected
						break;
					}
				}
			}
		});
	}
	
	template<typename Func>
	void forEachPacket(Func func) {
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

	void received() {
		joiner.add(receiver);
	}
};

}

#endif // __SOCKET_H__