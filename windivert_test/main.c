#include <winsock2.h>
#include <Ws2tcpip.h>		// inet_ntop()
#include <windows.h>
#include <stdio.h>		// printf()
#include <stdint.h>		// uintN_t
#include <stdlib.h>
#include <string.h>
#include "windivert.h"

#define MAXBUF		0xFFFF
#define IPv4		4
#define PROTOCOL_TCP	6
#define PORT_HTTP	80

#define chk_flag	1

struct ipv4_hdr
{
	uint8_t HL  : 4,			/* header length */
		Ver : 4;			/* version */
	uint8_t tos;				/* type of service */
	uint16_t len;				/* total length */
	uint16_t id;				/* identification */
	uint16_t off;				/* offset */
	uint8_t ttl;				/* time to live */
	uint8_t protocol;			/* protocol */
	uint16_t chk;				/* checksum */
	struct in_addr src, dst;		/* source and dest address */
};

struct tcp_hdr
{
	uint16_t s_port;			/* source port */
	uint16_t d_port;			/* destination port */
	uint32_t seq;				/* sequence number */
	uint32_t ack;				/* acknowledgement number */
	uint8_t reservation : 4,		/* (unused) */
		off         : 4;		/* data offset */
	uint8_t  flag;				/* control flags */
	uint16_t windows;			/* window */
	uint16_t chk;				/* checksum */
	uint16_t urgent_P;			/* urgent pointer */
};

void check(struct ipv4_hdr *ip, struct tcp_hdr *tcp) {
	uint8_t ip_buf[INET_ADDRSTRLEN];
	printf("\nSource IP\t: %s\n", inet_ntop(AF_INET, &ip->src, ip_buf, INET_ADDRSTRLEN));
	printf("Destination IP\t: %s\n", inet_ntop(AF_INET, &ip->dst, ip_buf, INET_ADDRSTRLEN));
	printf("Source Port\t: %u\n", ntohs(tcp->s_port));
	printf("Destination Port: %u\n", ntohs(tcp->d_port));
}

int main() {
	uint16_t priority = 0;
	HANDLE handle = WinDivertOpen("true", WINDIVERT_LAYER_NETWORK, priority, 0);
	if (handle == INVALID_HANDLE_VALUE)	{
		fprintf(stderr, "error: failed to open the WinDivert device\n");
		return 1;
	}
	while(TRUE) {
		uint8_t packet[MAXBUF];
		uint32_t packet_len;
		WINDIVERT_ADDRESS addr;
		if(!WinDivertRecv(handle, packet, sizeof(packet), &addr, &packet_len)) {
			fprintf(stderr, "warning: failed to read packet\n");
			continue;
		}

		struct ipv4_hdr *ip = (struct ipv4_hdr*)packet;
		if(ip->Ver==IPv4 && ip->protocol==PROTOCOL_TCP) {
			struct tcp_hdr *tcp = (struct tcp_hdr*)(packet+(ip->HL<<2));
			if(ntohs(tcp->s_port)==PORT_HTTP || ntohs(tcp->d_port)==PORT_HTTP) {
				if(chk_flag) check(ip, tcp);
				continue;
			}
		}

		if(!WinDivertSend(handle, packet, packet_len, &addr, NULL)) {
			fprintf(stderr, "warning: failed to send\n");
		}
	}
}
