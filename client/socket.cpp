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

	sendCommand("connect");

	SDLNet_UDP_AddSocket(sock_set, sock);

	client_thread = std::thread([&]() {
		while (sock) {
			sendCommand("ping");

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
	return true;
}

void client_socket::disconnect() {
	if (sock) {
		sendCommand("disconnect");

		SDLNet_UDP_DelSocket(sock_set, sock);
		SDLNet_UDP_Close(sock);
		sock = NULL;
	}
	if (client_thread.joinable()) {
		client_thread.join();
	}
}

bool client_socket::sendCommand(const std::string &cmd) {
	packet_data data;
	data.push_back(COMMAND_HANDLE);
	data.insert(data.end(), cmd.begin(), cmd.end());
	return send(data);
}

bool client_socket::sendEvent(const SDL_Event &e) {
	packet_data_out data;
	struct {
		uint8_t handler;
		SDL_Event event;
	} s_input = {INPUT_HANDLE, e};
	data.write(&s_input, sizeof(s_input));
	return send(data.data());
}

bool client_socket::send(packet_data data) {
	UDPpacket packet;

	packet.channel = 0;
	packet.data = data.data();
	packet.maxlen = PACKET_SIZE;
	packet.len = data.size();

	return SDLNet_UDP_Send(sock, packet.channel, &packet);
}

void client_socket::run() {
}

}