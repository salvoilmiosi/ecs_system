#include "server.h"

#include <iostream>
#include <algorithm>
#include <map>

namespace net {

bool server_socket::open(uint16_t port) {
	sock = SDLNet_UDP_Open(port);

	if (!sock) {
		std::cerr << "Could not open socket: " << SDLNet_GetError() << std::endl;
		return false;
	}

	console::addLine("Server open on port ", port);

	SDLNet_UDP_AddSocket(sock_set, sock);

	serv_thread = std::thread([&]() {
		UDPpacket receiver;

		uint8_t recv_data[PACKET_SIZE];

		receiver.channel = -1;
		receiver.data = recv_data;
		receiver.maxlen = PACKET_SIZE;

		while (sock) {
			testClients();

			int numready = SDLNet_CheckSockets(sock_set, CHECK_TIMEOUT);

			if (numready > 0) {
				if (SDLNet_UDP_Recv(sock, &receiver)) {
					received(receiver);
				}
			}
		}
	});
	return true;
}

void server_socket::close() {
	if (sock) {
		packet_writer out;
		writeByte(out, PACKET_SERVER_QUIT);
		sendAll(out.data());

		SDLNet_UDP_DelSocket(sock_set, sock);
		SDLNet_UDP_Close(sock);
		sock = nullptr;
	}
	if (serv_thread.joinable()) {
		serv_thread.join();
	}
}

void server_socket::sendServerMsg(const std::string &msg) {
	packet_writer out;
	writeByte(out, PACKET_SERVER_MSG);
	writeString(out, msg);
	sendAll(out.data());

	std::cout << msg << std::endl;
}

void server_socket::sendRaw(packet_data data, IPaddress addr) {
	UDPpacket packet;

	packet.channel = -1;
	packet.data = data.data();
	packet.len = data.size();
	packet.maxlen = PACKET_SIZE;
	packet.address = addr;
	
	SDLNet_UDP_Send(sock, packet.channel, &packet);
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
		
		packet_writer header;
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
	std::shared_lock lock(c_mutex);

	for (auto& c : clients_connected) {
		send(data, c.first);
	}
}

void server_socket::received(UDPpacket &packet) {
	packet_data recv_data(packet.data, packet.data + packet.len);
	packet_reader reader(recv_data);

	packet_type type = static_cast<packet_type>(readByte(reader));

	std::unique_lock lock(c_mutex);
	client_info &sender = clients_connected[packet.address];
	lock.unlock();

	switch (type) {
	case PACKET_USER_CONNECT:
		sender.address = packet.address;
		addClient(sender, reader);
		break;
	case PACKET_USER_DISCONNECT:
		clientDisconnect(sender, reader);
		break;
	case PACKET_USER_PING:
		clientPing(sender, reader);
		break;
	case PACKET_USER_STATE:
		clientState(sender, reader);
		break;
	case PACKET_USER_MSG:
		clientMessage(sender, reader);
		break;
	case PACKET_USER_INPUT:
		clientInput(sender, reader);
		break;
	default:
		lock.lock();
		clients_connected.erase(packet.address);
		break;
	}
}

void server_socket::addClient(client_info &sender, packet_reader &in) {
	std::unique_lock lock(c_mutex);
	
	sender.name = ipString(sender.address);
	sender.last_seen = SDL_GetTicks();

	if (!in.eof()) {
		//sender.name = readString(in);
	}

	lock.unlock();
	sendServerMsg(console::format(sender.name, " connected."));

	clientState(sender, in);
}

void server_socket::clientMessage(client_info &sender, packet_reader &in) {
	packet_writer pack;
	writeByte(pack, PACKET_SERVER_CHAT);
	writeString(pack, sender.name);
	writeString(pack, readString(in));
	sendAll(pack.data());
}

void server_socket::clientInput(client_info &sender, packet_reader &in) {
	game::userinput::command cmd;
	cmd.cmd = static_cast<game::userinput::command_type>(readByte(in));

	if (cmd.cmd == game::userinput::CMD_NONE) return;

	cmd.pos.read(in);

	sender.input.handleCommand(wld, cmd);
}

void server_socket::clientState(client_info &sender, packet_reader &reader) {
	packet_writer packet;

	writeByte(packet, PACKET_EDITLOG);
	wld.createStateLogger().write(packet);
	
	send(packet.data(), sender.address);
}

void server_socket::clientPing(client_info &sender, packet_reader &reader) {
	sender.last_seen = SDL_GetTicks();
}

void server_socket::clientDisconnect(client_info &sender, packet_reader &reader) {
	sendServerMsg(console::format(sender.name, " disconnected (", readString(reader), ")"));

	std::unique_lock lock(c_mutex);
	clients_connected.erase(sender.address);
}

void server_socket::testClients() {
	std::shared_lock lock(c_mutex);

	Uint32 now = SDL_GetTicks();

	for (auto it = clients_connected.begin(); it != clients_connected.end(); ) {
		if (now - it->second.last_seen > CLIENT_TIMEOUT) {
			sendServerMsg(console::format(it->second.name, " timed out."));
			clients_connected.erase(it++);
		} else {
			++it;
		}
	}
}

}