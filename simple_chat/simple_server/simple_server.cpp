#include <iostream>
#include <list>
#include <mutex>
#include <process.h>
#include <WS2tcpip.h> // inet_ntop
#include <winsock2.h>

using std::list;
using std::mutex;
using std::cout;

#define STR_LEN		1024

list<SOCKET> user;
mutex mtx, mtxc;

void usage() {
	cout << "syntax: simple_server.exe <PORT>\n";
	cout << "sample: simple_server.exe 1234\n";
}

void recv_clt(void* clt) {
	SOCKET sock = *(SOCKET*)clt;
	mtxc.lock();
	cout << "식별 번호 : " << sock << "\n\n";
	mtxc.unlock();

	char msg[STR_LEN]="채팅방에 입장하였습니다.\n";
	int res = send(sock, msg, strlen(msg), 0);
	while(res != SOCKET_ERROR) {
		res = recv(sock, msg, sizeof(msg), 0);
		if(res == SOCKET_ERROR) continue;

		char edit_msg[STR_LEN];
		sprintf(edit_msg, "\n[ %d ] ", sock);
		int size = strlen(edit_msg) + 1;
		res < STR_LEN-size ? res : res = STR_LEN-size;
		strncat(edit_msg, msg, res);

		mtx.lock();
		for(list<SOCKET>::iterator it=user.begin(); it != user.end(); it++) {
			if(*it != sock) send(*it, edit_msg, strlen(edit_msg), 0);
		}
		mtx.unlock();
		if(!strncmp(msg, "exit", res)) res = SOCKET_ERROR;
	}
	user.remove(sock);
	closesocket(sock);
	mtxc.lock();
	cout << "익명의 이용자 < 식별 번호 : " << sock << " > 퇴장\n\n";
	mtxc.unlock();
}

int main(int argc, char* argv[])
{
	if(argc != 2) {
		usage();
		return 1;
	}

	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_sock == INVALID_SOCKET) {
		cout << "socket error!!\n";
		return 1;
	}

	SOCKADDR_IN srv;
	srv.sin_family = AF_INET;
	srv.sin_port = htons(atoi(argv[1]));
	srv.sin_addr.s_addr = htonl(INADDR_ANY);

	int error = bind(tcp_sock, (SOCKADDR *)&srv, sizeof(srv));
	if (error == SOCKET_ERROR) {
		cout << "socket error!!\n";
		return 1;
	}

	error = listen(tcp_sock, SOMAXCONN);	// 서비스 과부하시 에러
	if (error == SOCKET_ERROR) {
		cout << "listen error!!\n";
		return 1;
	}

	SOCKADDR_IN clt_addr;	// client address
	int clt_len = sizeof(clt_addr);
	cout << "채팅 서버 프로그램이 실행되었습니다.\n";
	while(1) {
		SOCKET clt = accept(tcp_sock, (SOCKADDR *)&clt_addr, &clt_len);
		if(clt != INVALID_SOCKET) {
			mtx.lock();
			user.push_back(clt);
			mtx.unlock();
			_beginthread(recv_clt, 0, &user.back());
			char ip_buf[16];
			mtxc.lock();
			cout << user.size() << "번째 익명의 이용자 < " << inet_ntop(AF_INET, &clt_addr.sin_addr, ip_buf, 16) << " > 입장\n";
			mtxc.unlock();
		}
	}
	closesocket(tcp_sock);
	WSACleanup();
	return 0;
}
