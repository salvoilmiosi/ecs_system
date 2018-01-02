#include "client.h"

#include <algorithm>
#include <iostream>

namespace net {

bool client_socket::connect(IPaddress addr, const std::string &username) {
	disconnect(); // If connected already

	server_addr = addr;

	sock = SDLNet_UDP_Open(0);
	SDLNet_UDP_Bind(sock, 0, &addr);

	if (!sock) {
		std::cerr << "Could not open socket: " << SDLNet_GetError() << std::endl;
		return false;
	}

	packet_writer connect_packet;
	writeByte(connect_packet, PACKET_USER_CONNECT);
	writeString(connect_packet, username);
	send(connect_packet.data());

	SDLNet_UDP_AddSocket(sock_set, sock);

	client_thread = std::thread([&]() {
		while (sock) {
			sendPing();

			int numready = SDLNet_CheckSockets(sock_set, CHECK_TIMEOUT);

			if (numready > 0) {
				if (SDLNet_UDP_Recv(sock, &receiver)) {
					if (receiver.address == server_addr) {
						received();
					}
				} else if (is_open()) {
					// Server timed out -- localhost only
					SDLNet_UDP_DelSocket(sock_set, sock);
					SDLNet_UDP_Close(sock);
					sock = nullptr;
					break;
				}
			}
		}
	});
	return true;
}

void client_socket::disconnect() {
	if (is_open()) {
		sendDisconnect("Disconnect by user");

		SDLNet_UDP_DelSocket(sock_set, sock);
		SDLNet_UDP_Close(sock);
		sock = nullptr;
	}
	if (client_thread.joinable()) {
		client_thread.join();
	}
}

bool client_socket::sendStatePacket() {
	packet_writer out;
	writeByte(out, PACKET_USER_STATE);
	return send(out.data());
}

bool client_socket::sendPing() {
	packet_writer out;
	writeByte(out, PACKET_USER_PING);
	return send(out.data());
}

bool client_socket::sendDisconnect(const std::string &msg) {
	packet_writer out;
	writeByte(out, PACKET_USER_DISCONNECT);
	writeString(out, msg);
	return send(out.data());
}

bool client_socket::sendMessage(const std::string &msg) {
	packet_writer out;
	writeByte(out, PACKET_USER_MSG);
	writeString(out, msg);
	return send(out.data());
}

bool client_socket::sendInputCommand(const game::userinput::command &cmd) {
	if (cmd.cmd == game::userinput::CMD_NONE) return false;

	packet_writer out;
	writeByte(out, PACKET_USER_INPUT);
	writeByte(out, cmd.cmd);
	cmd.pos.write(out);
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
	packet_type type = static_cast<packet_type>(recv_data[0]);

	if (type != PACKET_SLICED) {
		std::lock_guard lock(j_mutex);

		joined.emplace_back(recv_data.begin(), recv_data.begin() + receiver.len);
		return;
	}
	
	constexpr size_t HEAD_SIZE = 7;

	packet_reader in(recv_data);

	recv_packet p;
	readByte(in); // should be PACKET_SLICED
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