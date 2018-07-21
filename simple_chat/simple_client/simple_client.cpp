#include <iostream>
#include <process.h>
#include <winsock2.h>
using namespace std;

#define STR_LEN 1024

void usage() {
	cout << "syntax: simple_client.exe <IP> <PORT>\n";
	cout << "sample: simple_client.exe 10.10.10.10 1234\n";
}

void recv_msg(void* sock) {
	while(1){
		char msg[STR_LEN];
		int res = recv(*(SOCKET*)sock, msg, STR_LEN, 0);
		if(res == SOCKET_ERROR)	break;
		res < STR_LEN ? res : res = STR_LEN-1;
		msg[res] = '\0';

		cout << msg << endl;
	}
	closesocket(*(SOCKET*)sock);
}

int main(int argc, char* argv[])
{
	if(argc != 3) {
		usage();
		return 1;
	}

	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp_sock == INVALID_SOCKET) {
		cout << "socket error!!\n";
		return 1;
	}

	SOCKADDR_IN srv;
	srv.sin_family = AF_INET;
	srv.sin_addr.S_un.S_addr = inet_addr(argv[1]);
	srv.sin_port = htons(atoi(argv[2]));

	if(connect(tcp_sock, (SOCKADDR *)&srv, sizeof(srv)) == SOCKET_ERROR) {
		cout << "connect error!!\n";
		return 1;
	}

	_beginthread(recv_msg, 0, &tcp_sock);
	char send_str[STR_LEN];
	int res = 1;
	do {
		cin.clear();
		cin.getline(send_str, STR_LEN);
		if(!strlen(send_str)) continue;
		res = send(tcp_sock, send_str, strlen(send_str), 0);
	}while(res != SOCKET_ERROR && strncmp(send_str, "exit", res));

	closesocket(tcp_sock);
	WSACleanup();
	cout << "프로그램을 종료합니다.\n";
	return 0;
}
