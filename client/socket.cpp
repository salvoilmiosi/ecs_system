#include "socket.h"

#include <algorithm>
#include <iostream>

namespace socket {

void packet_joiner::add(UDPpacket pack) {
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

void packet_joiner::findJoin(Uint32 time) {
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

void packet_joiner::join(std::vector<packet_it> &sameTime) {
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
			int numready = SDLNet_CheckSockets(sock_set, CHECK_TIMEOUT);

			sendChar('p');

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