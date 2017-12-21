#include "socket.h"

#include <algorithm>
#include <iostream>

namespace socket {

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
	packet_data_out out;
	writeByte(out, COMMAND_HANDLE);
	writeString(out, cmd);
	return send(out.data());
}

bool client_socket::sendInputCommand(const userinput::command &cmd) {
	if (cmd.cmd == userinput::CMD_NONE) return false;

	packet_data_out out;
	writeByte(out, INPUT_HANDLE);
	writeByte(out, cmd.cmd);
	writeBinary<position>(out, cmd.pos);
	return send(out.data());
}

bool client_socket::send(packet_data data) {
	UDPpacket packet;

	packet.channel = 0;
	packet.data = data.data();
	packet.len = data.size();
	packet.maxlen = PACKET_SIZE;

	return SDLNet_UDP_Send(sock, packet.channel, &packet);
}

void client_socket::received() {
	packet_data_in in(recv_data);

	static const size_t HEAD_SIZE = 6;

	recv_packet p;
	p.pid = readLong(in);
	p.count = readByte(in);
	p.slices = readByte(in);
	
	p.data.assign(recv_data.begin() + HEAD_SIZE, recv_data.begin() + receiver.len);
	p.time_added = SDL_GetTicks();

	// Inserts a new vector in the joining map
	auto &packs = joining[p.pid];
	packs.push_back(p);

	if (packs.size() >= p.slices) {
		std::lock_guard lock(j_mutex);

		std::sort(packs.begin(), packs.end(), [](auto &a, auto &b) {
			return a.count < b.count;
		});

		packet_data joined_data;
		for (auto &x : packs) {
			joined_data.insert(joined_data.end(), x.data.begin(), x.data.end());
		}
		joined.push_back(joined_data);

		joining.erase(p.pid);
	}

	// Dirty way to check if the oldest element is timed out
	if (!joining.empty() && SDL_GetTicks() - joining.begin()->second.back().time_added > CLIENT_TIMEOUT) {
		joining.erase(joining.begin());
	}
}

}