#include "socket.h"

#include <algorithm>
#include <iostream>

namespace socket {

void packet_joiner::add(UDPpacket pack) {
	packet p;
	p.len = pack.len - sizeof(p.header);
	p.time_added = SDL_GetTicks();

	std::copy(pack.data, pack.data + sizeof(p.header), reinterpret_cast<uint8_t *>(&p.header));
	p.data.assign(pack.data + sizeof(p.header), pack.data + pack.len);

	packets.push_front(p);

	findJoin(p.header.pid);

	if (!packets.empty() && SDL_GetTicks() - packets.back().time_added > CLIENT_TIMEOUT) {
		packets.pop_back();
	}
}

void packet_joiner::findJoin(Uint32 pid) {
	std::vector<packet_it> sameId;
	for (auto it = packets.begin(); it != packets.end(); ++it) {
		if (it->header.pid == pid) {
			sameId.push_back(it);
			if (sameId.size() >= it->header.slices) {
				join(sameId);
				break;
			}
		}
	}
}

void packet_joiner::join(std::vector<packet_it> &sameId) {
	std::sort(sameId.begin(), sameId.end(), [](auto &a, auto &b) {
		return a->header.count < b->header.count;
	});

	packet_data data;
	for (auto &x : sameId) {
		data.insert(data.end(), x->data.begin(), x->data.end());
		packets.erase(x);
	}
	joined.push_back(data);
}

bool client_socket::connect(IPaddress addr) {
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

void client_socket::close() {
	if (sock) {
		disconnect();
		SDLNet_UDP_DelSocket(sock_set, sock);
		SDLNet_UDP_Close(sock);
		sock = NULL;
	}
}

void client_socket::disconnect() {
	sendChar('d');
	memset(&server_addr, 0, sizeof(server_addr));
}

bool client_socket::send(const Uint8 *data, size_t len) {
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

void client_socket::run() {
	client_thread = std::thread([this]() {
		while (sock) {
			sendChar('p');

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

}