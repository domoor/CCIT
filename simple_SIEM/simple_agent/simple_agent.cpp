#include <stdio.h>
#include <stdlib.h>
#include <cstring>  // strlen()
#include <cstdarg>  // va_list
#include <unistd.h> // 삭제 각
#include <arpa/inet.h>
#include <string>
//#include <iostream> // 삭제 각
#include <sstream>
//#include <malloc.h> // 삭제 각
using std::string;

#define BUF_SIZ		1024
#define SH_PATH		"/usr/share/zaproxy/"
#define PORT_PATH	"/root/nmap/"
#define WEB_PATH	"/root/zaproxy/"
#define FILE_NAME	"test_0000.xml"

string commands(int argc, ...) {
    string command;
    va_list ap;

    va_start(ap, argc);
    for(int i=0; i<argc; i++) command += va_arg(ap, char*);
    va_end(ap);
    
    return command;
}

int file_create_P(int sock) {
    int res;
    char target[BUF_SIZ];

    res = recv(sock, target, BUF_SIZ, 0);
    if(res < 0) {
	perror("target recv failed !");
	return -1;
    }
    target[res] = '\0';

    puts("\n포트 점검을 3초 뒤에 실행합니다.");
    sleep(3);

    string command = commands(5, "nmap -sS ", target, " -oX ",
			     PORT_PATH, FILE_NAME);
    puts(command.c_str());
    system(command.c_str());
  
    int len = 0;
    string file = commands(2, PORT_PATH, FILE_NAME);
    FILE *rfp = fopen(file.c_str(), "r");
    if(rfp == NULL) {
        perror("file read failed !");
	return -1;
    }
    fseek(rfp, 0,SEEK_END);
    len = ftell(rfp);
    fseek(rfp, 0, SEEK_SET);

    char *buf = (char*)malloc(len);
    fread(buf,1,len,rfp);
    fclose(rfp);
    
    char length[10];
    sprintf(length, "%d", len);

    res = send(sock, (char*)&len, strlen(length), 0);
    if(res < 0) {
	perror("send1 failed !");
	return -1;
    }
    puts("2초간 대기합니다.");
    sleep(2);

    res = send(sock, buf, len, 0);
    if(res < 0) {
	perror("send1 failed !");
	return -1;
    }

    puts("점검 내역이 서버로 전송되었습니다.");
    free(buf);
    return 1;
}

int file_create_W(int sock) {
    int res;
    char target[BUF_SIZ];

    res = recv(sock, target, BUF_SIZ, 0);
    if(res < 0) {
	perror("target recv failed !");
	return -1;
    }
    target[res] = '\0';

    puts("\n취약점 점검을 3초 뒤에 실행합니다.");
    sleep(3);

    string command = commands(6, SH_PATH, "zap.sh -cmd -quickurl \\ ",
		    	     target, " -quickprogress -quickout ",
			     WEB_PATH, FILE_NAME);
    puts(command.c_str());
    system(command.c_str());
  
    string file = WEB_PATH;
    file += FILE_NAME;
    FILE *rfp = fopen(file.c_str(), "r");
    if(rfp == NULL) {
        perror("file read failed !");
	return -1;
    }
    fseek(rfp, 0,SEEK_END);
    int len = ftell(rfp);
    fseek(rfp, 0, SEEK_SET);

    char *buf = (char*)malloc(len);
    fread(buf,1,len,rfp);
    fclose(rfp);
    
    char length[10];
    sprintf(length, "%d", len);

    res = send(sock, (char*)&len, strlen(length), 0);
    if(res < 0) {
	perror("send1 failed !");
	return -1;
    }
    puts("2초간 대기합니다.");
    sleep(2);

    res = send(sock, buf, len, 0);
    if(res < 0) {
	perror("send1 failed !");
	return -1;
    }

    puts("점검 내역이 서버로 전송되었습니다.");
    free(buf);
    return 1;
}

int main(int argc, char **argv)
{
    int sock;
    struct sockaddr_in servaddr;

    if(argc != 3)
    {
        printf("Usage: %s <ip_address> <port>\n", argv[0]);
        return -1;
    }

    if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror("socket failed !");
        return -1;
    }
    
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    servaddr.sin_port = htons(atoi(argv[2]));

    // 연결 요청
    if(connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Connection failed !");
        return -1;
    }
    puts("connect success !");

    while(1) {
        char buf[BUF_SIZ];
    	int res = recv(sock, buf, BUF_SIZ, 0);
	if(res <= 0) {
	    perror("size error !");
	    return -1;
	}
	else if(!strncmp(buf, "port\n", res)) {
	    if(file_create_P(sock) < 0) {
		perror("port scan error !");
		return -1;
	    }
	}
	else if(!strncmp(buf, "web\n", res)) {
	    if(file_create_W(sock) < 0) {
		perror("web scan error !");
		return -1;
	    }
	}
	else if(!strncmp(buf, "end\n", res)) {
	    puts("콘솔과의 연결이 끊어져 프로그램을 종료합니다.");
	    break;
	}
    }

    close(sock);
    return 0;
}
