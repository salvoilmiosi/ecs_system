#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <SDL2/SDL_net.h>

#include <mutex>
#include <iostream>

#include "console.h"

namespace net {

static const uint16_t PORT = 2345;
static const int PACKET_SIZE = 1024;
static const int CHECK_TIMEOUT = 1000;
static const int CLIENT_TIMEOUT = 5000;

static const int TICKRATE = 60;

enum packet_type {
	PACKET_NONE,
	PACKET_SLICED,
	
	PACKET_EDITLOG,
	PACKET_SERVER_CHAT,
	PACKET_SERVER_MSG,
	PACKET_SERVER_QUIT,

	PACKET_USER_CONNECT,
	PACKET_USER_DISCONNECT,
	PACKET_USER_PING,
	PACKET_USER_STATE,
	PACKET_USER_MSG,
	PACKET_USER_INPUT
};

inline char *ipString(const IPaddress &ip) {
	static char addr_name[128];
	memset(addr_name, 0, 128);
	uint32_t full_ip = ip.host;
	uint8_t addr1 = (full_ip & 0x000000ff) >> (0 * 8);
	uint8_t addr2 = (full_ip & 0x0000ff00) >> (1 * 8);
	uint8_t addr3 = (full_ip & 0x00ff0000) >> (2 * 8);
	uint8_t addr4 = (full_ip & 0xff000000) >> (3 * 8);

	uint16_t port_host = ((ip.port & 0xff00) >> 8) | ((ip.port & 0x00ff) << 8);
	sprintf(addr_name, "%d.%d.%d.%d:%d", addr1, addr2, addr3, addr4, port_host);
	return addr_name;
}

constexpr bool operator == (const IPaddress &a, const IPaddress &b) {
	return a.host == b.host && a.port == b.port;
}

}

#endif // __SOCKET_H__