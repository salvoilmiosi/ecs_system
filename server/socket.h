#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <SDL2/SDL_net.h>

#include <thread>
#include <mutex>
#include <iostream>

namespace socket {

constexpr uint16_t ntohs(uint16_t num) {
	return ((num & 0xff00) >> 8) | ((num & 0x00ff) << 8);
}

const char *ipString(const IPaddress &ip) {
	static char addr_name[128];
	memset(addr_name, 0, 128);
	uint32_t full_ip = ip.host;
	uint8_t addr1 = (full_ip & 0x000000ff) >> (0 * 8);
	uint8_t addr2 = (full_ip & 0x0000ff00) >> (1 * 8);
	uint8_t addr3 = (full_ip & 0x00ff0000) >> (2 * 8);
	uint8_t addr4 = (full_ip & 0xff000000) >> (3 * 8);
	sprintf(addr_name, "%d.%d.%d.%d:%d", addr1, addr2, addr3, addr4, ntohs(ip.port));
	return addr_name;
}

bool operator == (const IPaddress &a, const IPaddress &b) {
	return a.host == b.host && a.port == b.port;
}

static const uint16_t PORT = 2345; 
static const int PACKET_SIZE = 1024;
static const int CHECK_TIMEOUT = 1000;
static const int CLIENT_TIMEOUT = 5000;

class server_socket {
private:
	UDPsocket sock = NULL;

	SDLNet_SocketSet sock_set;

	std::thread serv_thread;

public:
	server_socket() {
		sock_set = SDLNet_AllocSocketSet(1);

		memset(&receiver, 0, sizeof(receiver));

		receiver.channel = -1;
		receiver.data = pack_data;
		receiver.maxlen = PACKET_SIZE;
	}

	server_socket(uint16_t port) {
		open(port);
	}

	~server_socket() {
		close();
		serv_thread.join();
	}

	bool open(uint16_t port = PORT) {
		sock = SDLNet_UDP_Open(port);

		if (!sock) {
			std::cerr << "Could not open socket: " << SDLNet_GetError() << std::endl;
			return false;
		}

		SDLNet_UDP_AddSocket(sock_set, sock);
		return true;
	}

	void close() {
		if (sock) {
			SDLNet_UDP_Close(sock);
			SDLNet_UDP_DelSocket(sock_set, sock);
			sock = NULL;
		}
	}

	void run() {
		serv_thread = std::thread([this]() {
			while (sock) {
				testClients();

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

	void sendAll(Uint8 *data_ptr, int len) {
		std::lock_guard lock(c_mutex);

		Uint8 data_copy[PACKET_SIZE];

		// Divide the packet in PACKET_SIZE sized slices.
		// Add a header to all packets so they can be reconstructed in order when received
		struct {
			Uint32 time;
			Uint8 count;
			Uint8 slices;
		} header;

		header.time = SDL_GetTicks();
		header.count = 0;
		header.slices = len / (PACKET_SIZE - sizeof(header)) + 1;

		while (len > 0) {
			UDPpacket packet;
			memset(&packet, 0, sizeof(packet));

			packet.channel = -1;
			packet.data = data_copy;
			packet.maxlen = PACKET_SIZE;
			packet.len = PACKET_SIZE > (len + sizeof(header)) ? (len + sizeof(header)) : PACKET_SIZE;

			memset(data_copy, 0, PACKET_SIZE);
			memcpy(data_copy, &header, sizeof(header));
			memcpy(data_copy + sizeof(header), data_ptr, packet.len - sizeof(header));

			for (auto &c : clients_connected) {
				packet.address = c.address;
				if (!SDLNet_UDP_Send(sock, packet.channel, &packet)) {
					std::cout << "Lost packet" << std::endl;
				}
			}
			data_ptr += PACKET_SIZE;
			len -= PACKET_SIZE - sizeof(header);

			++header.count;
		}
	}

private:
	UDPpacket receiver;
	Uint8 pack_data[PACKET_SIZE];

	struct client_info {
		IPaddress address;
		Uint32 last_seen;
	};

	std::vector<client_info> clients_connected;

	std::mutex c_mutex;

	void received() {
		std::lock_guard lock(c_mutex);

		std::cout << ipString(receiver.address) << ": " << pack_data << std::endl;
		// if (strcmp(pack_data, "CONNECT") == 0) {

		// testing with netcat
		switch (pack_data[0]) {
		case 'c': // Connect
			addClient();
			break;
		case 'p': // Ping
			pingClient();
			break;
		case 'd': // Disconnect
			delClient();
			break;
		default:
			break;
		}
	}

	auto findClient() {
		return std::find_if(clients_connected.begin(), clients_connected.end(), [this](auto &c) {
			return c.address == receiver.address;
		});
	}

	void addClient() {
		if (findClient() == clients_connected.end()) {
			// If none are found create one
			client_info c;
			c.address = receiver.address;
			c.last_seen = SDL_GetTicks();

			clients_connected.push_back(c);
			std::cout << ipString(receiver.address) << " connected" << std::endl;

			//SDLNet_UDP_Bind(sock, 0, &c.address);
		}
	}

	void pingClient() {
		if (auto it = findClient(); it != clients_connected.end()) {
			it->last_seen = SDL_GetTicks();
		}
	}

	void delClient() {
		clients_connected.erase(std::remove_if(clients_connected.begin(), clients_connected.end(), [this](auto &c) {
			if (c.address == receiver.address) {
				std::cout << ipString(c.address) << " disconnected" << std::endl;
				return true;
			} else {
				return false;
			}
		}), clients_connected.end());
	}

	void testClients() {
		Uint32 now = SDL_GetTicks();

		clients_connected.erase(std::remove_if(clients_connected.begin(), clients_connected.end(), [&](auto &c) {
			if (now - c.last_seen > CLIENT_TIMEOUT) {
				std::cout << ipString(c.address) << " timed out" << std::endl;
				return true;
			} else {
				return false;
			}
		}), clients_connected.end());
	}
};

}

#endif