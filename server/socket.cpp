#include "socket.h"

#include <iostream>
#include <algorithm>

#include "main.h"
#include "userinput.h"
#include "components_serial.h"

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

bool server_socket::open(uint16_t port) {
	sock = SDLNet_UDP_Open(port);

	if (!sock) {
		std::cerr << "Could not open socket: " << SDLNet_GetError() << std::endl;
		return false;
	}

	std::cout << "Server open on port " << port << std::endl;

	SDLNet_UDP_AddSocket(sock_set, sock);

	serv_thread = std::thread([&]() {
		while (sock) {
			testClients();

			int numready = SDLNet_CheckSockets(sock_set, CHECK_TIMEOUT);

			if (numready > 0) {
				if (SDLNet_UDP_Recv(sock, &receiver)) {
					received();
				}
			}
		}
	});
	return true;
}

void server_socket::close() {
	if (sock) {
		SDLNet_UDP_DelSocket(sock_set, sock);
		SDLNet_UDP_Close(sock);
		sock = NULL;
	}
	if (serv_thread.joinable()) {
		serv_thread.join();
	}
}

void server_socket::sendServerMsg(const std::string &msg) {
	packet_data_out out;
	writeByte(out, PACKET_SERVERMSG);
	writeString(out, msg);
	sendAll(out.data());
}

void server_socket::sendRaw(packet_data data, IPaddress addr) {
	UDPpacket packet;

	packet.channel = -1;
	packet.data = data.data();
	packet.len = data.size();
	packet.maxlen = PACKET_SIZE;
	packet.address = addr;
	
	if (!SDLNet_UDP_Send(sock, packet.channel, &packet)) {
		std::cout << "Lost packet" << std::endl;
	}
}

void server_socket::sendSliced(const packet_data &data, IPaddress addr) {
	// Divide the packet in PACKET_SIZE sized slices.
	// Add a header to all packets so they can be reconstructed in order when received

	constexpr size_t HEAD_SIZE = 7;

	uint32_t pid = maxPid++;
	uint8_t count = 0;
	uint8_t slices = data.size() / (PACKET_SIZE - HEAD_SIZE) + 1;

	size_t index = 0;

	for (int remaining = data.size(); remaining > 0;) {
		size_t packet_len = PACKET_SIZE > (remaining + HEAD_SIZE) ? (remaining + HEAD_SIZE) : PACKET_SIZE;
		
		packet_data_out header;
		writeByte(header, PACKET_SLICED);
		writeLong(header, pid);
		writeByte(header, count);
		writeByte(header, slices);
		packet_data writer(header.data());
		writer.insert(writer.end(), data.begin() + index, data.begin() + index + packet_len - HEAD_SIZE);

		sendRaw(writer, addr);

		index += PACKET_SIZE - HEAD_SIZE;
		remaining -= PACKET_SIZE - HEAD_SIZE;

		++count;
	}
}

void server_socket::send(const packet_data &data, IPaddress addr) {
	if (data.size() > PACKET_SIZE) {
		sendSliced(data, addr);
	} else {
		sendRaw(data, addr);
	}
}

void server_socket::sendAll(const packet_data &data) {
	std::lock_guard lock(c_mutex);

	for (auto &c : clients_connected) {
		send(data, c.address);
	}
}

void server_socket::received() {
	c_mutex.lock();

	last_sender = std::find_if(clients_connected.begin(), clients_connected.end(), [&](auto &c) {
		return c.address == receiver.address;
	});

	c_mutex.unlock();

	packet_data_in reader(recv_data);

	packet_type type = static_cast<packet_type>(readByte(reader));

	switch (type) {
	case PACKET_USER_COMMAND:
		parseCommand(reader);
		break;
	case PACKET_USER_INPUT:
		parseInput(reader);
		break;
	case PACKET_NONE:
	default:
		break;
	}
}

void server_socket::parseCommand(packet_data_in &in) {
	std::string cmd = readString(in);
	if (cmd == "connect") {
		addClient();
	} else if (cmd == "state") {
		stateClient();
	} else if (cmd == "ping") {
		pingClient();
	} else if (cmd == "disconnect") {
		delClient();
	}
}

void server_socket::parseInput(packet_data_in &in) {
	input_command cmd;
	cmd.cmd = static_cast<command_type>(readByte(in));

	if (cmd.cmd == CMD_NONE) return;

	cmd.pos = readBinary<position>(in);

	last_sender->input.handleCommand(cmd);
}

void server_socket::addClient() {
	if (last_sender == clients_connected.end()) {
		// If none are found create one
		client_info c;
		c.address = receiver.address;
		c.last_seen = SDL_GetTicks();

		clients_connected.push_back(c);
		std::cout << ipString(receiver.address) << " connected" << std::endl;

		stateClient();
	}
}

void server_socket::stateClient() {
	packet_data_out packet;

	writeByte(packet, PACKET_EDITLOG);
	server::wld.logState().write(packet);
	
	send(packet.data(), receiver.address);
}

void server_socket::pingClient() {
	last_sender->last_seen = SDL_GetTicks();
}

void server_socket::delClient() {
	std::lock_guard lock(c_mutex);

	clients_connected.erase(std::remove_if(clients_connected.begin(), clients_connected.end(), [&](auto &c) {
		if (c.address == receiver.address) {
			std::cout << ipString(c.address) << " disconnected" << std::endl;
			return true;
		} else {
			return false;
		}
	}), clients_connected.end());
}

void server_socket::testClients() {
	std::lock_guard lock(c_mutex);

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

}