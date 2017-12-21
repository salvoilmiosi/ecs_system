#include "socket.h"

#include <algorithm>
#include <iostream>

namespace socket {

void packet_joiner::add(UDPpacket pack) {
	packet_data_in in(pack);

	static const size_t HEAD_SIZE = 6;

	packet p;
	p.pid = readBinary<uint32_t>(in);
	p.count = readBinary<uint8_t>(in);
	p.slices = readBinary<uint8_t>(in);
	p.time_added = SDL_GetTicks();
	p.data.assign(pack.data + HEAD_SIZE, pack.data + pack.len);
	p.len = pack.len - HEAD_SIZE;

	packets.push_front(p);

	findJoin(p.pid);

	if (!packets.empty() && SDL_GetTicks() - packets.back().time_added > CLIENT_TIMEOUT) {
		packets.pop_back();
	}
}

void packet_joiner::findJoin(Uint32 pid) {
	std::vector<packet_it> sameId;
	for (auto it = packets.begin(); it != packets.end(); ++it) {
		if (it->pid == pid) {
			sameId.push_back(it);
			if (sameId.size() >= it->slices) {
				join(sameId);
				break;
			}
		}
	}
}

void packet_joiner::join(std::vector<packet_it> &sameId) {
	std::sort(sameId.begin(), sameId.end(), [](auto &a, auto &b) {
		return a->count < b->count;
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

bool client_socket::sendInputCommand(const userinput::command &cmd) {
	if (cmd.cmd == userinput::CMD_NONE) return false;

	packet_data_out data;
	writeBinary<uint8_t>(data, INPUT_HANDLE);
	writeBinary<uint8_t>(data, cmd.cmd);
	writeBinary<position>(data, cmd.pos);
	return send(data.data());
}

bool client_socket::send(packet_data data) {
	UDPpacket packet;

	packet.channel = 0;
	packet.data = data.data();
	packet.len = data.size();
	packet.maxlen = PACKET_SIZE;

	return SDLNet_UDP_Send(sock, packet.channel, &packet);
}

}