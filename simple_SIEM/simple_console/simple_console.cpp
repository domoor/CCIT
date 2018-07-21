#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <string>
using std::string;

#define BUF_SIZ		1024
#define PORT_PATH	"/opt/splunk/test/port/"
#define WEB_PATH	"/opt/splunk/test/web/"
#define FILE_NAME	"result_0000.xml"

int port_res(int sock){
    int res, len;
    char target[BUF_SIZ];

    puts("취약점 검사 대상을 입력해 주세요.");
    puts("ex) 127.0.0.1");
    printf(": ");
    scanf("%s", target);

    sleep(1);
    res = send(sock, target, strlen(target), 0);
    if(res < 0) {
	perror("target send failed !");
	return -1;
    }

    res = recv(sock, (char*)&len, 10, 0);
    if(res < 0) {
        perror("recv1 failed !");
	return -1;
    }
    char *buf = (char*)malloc(len+1);

    res = recv(sock, buf, len, 0);
    if(res < 0) {
        perror("recv2 failed !");
	return -1;
    }
    buf[len] = '\n';

    string file = PORT_PATH;
    file += FILE_NAME;
    FILE * wfp = fopen(file.c_str(), "w");
    fwrite(buf, 1, len, wfp);
    fclose(wfp);
    free(buf);

    puts("\n검사가 완료 되었습니다.");
    return 1;
}

int web_res(int sock){
    int res, len;
    char target[BUF_SIZ];

    puts("\n취약점 검사 대상을 입력해 주세요.");
    puts("ex) http://127.0.0.1:8000");
    printf(": ");
    scanf("%s", target);
    
    sleep(1);
    res = send(sock, target, strlen(target), 0);
    if(res < 0) {
	perror("target send failed !");
	return -1;
    }

    res = recv(sock, (char*)&len, 10, 0);
    if(res < 0) {
        perror("recv1 failed !");
	return -1;
    }
    char *buf = (char*)malloc(len+1);

    res = recv(sock, buf, len, 0);
    if(res < 0) {
        perror("recv2 failed !");
	return -1;
    }
    buf[len] = '\n';

    string file = WEB_PATH;
    file += FILE_NAME;
    FILE * wfp = fopen(file.c_str(), "w");
    fwrite(buf, 1, len, wfp);
    fclose(wfp);
    free(buf);

    puts("\n검사가 완료 되었습니다.");
    return 1;
}

int command_srv(int sock){
    int res;
    int start;

    while(1) {
	puts("\nport\t: 1");
	puts("web\t: 2");
	puts("exit\t: default");
	printf("Input : ");
        scanf("%d",&start);
        switch(start) {
	case 1:
	    puts("\n열려있는 포트에 대해 검사를 시작합니다.");
	    res = send(sock, "port\n",5,0);
	    if(res < 0) {
        	perror("send failed !");
		return -1;
	    }
	    if(res = port_res(sock) < 0) return res;
	    break;
	case 2:
	    puts("웹 취약점 검사를 시작합니다.");
	    res = send(sock,"web\n",4,0);
	    if(res < 0) {
        	perror("send failed !");
		return -1;
	    }
	    if(res = web_res(sock) < 0) return res;
	    break;
	default:
	    puts("프로그램을 종료합니다.");
	    res = send(sock,"end\n",4,0);
	    if(res < 0) {
        	perror("send failed !");
		return -1;
	    }
	    return 0;
 	}
    }
}

int main(int argc, char **argv)
{
    struct sockaddr_in servaddr, cliaddr;
    int sockfd; // socket descripter
    int acc_sock;   // accept_socket
    socklen_t addrlen = sizeof(cliaddr);

    if(argc != 2)
    {
        printf("Usage: %s <port> \n", argv[0]);
        return -1;
    }

    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket failed !");
        return -1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[1]));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed !");
        return -1;
    }

    if(listen(sockfd, 1) < 0)
    {
	    perror("listen failed !");
	    return -1;
    }
    puts("Listening.....");

    acc_sock = accept(sockfd, (struct sockaddr*)&cliaddr, &addrlen);
    if(acc_sock < 0)
    {
        perror("accept failed !");
        return -1;
    }
    puts("Connected client");

    int res;
    if(res = command_srv(acc_sock) < 0) return res;

    close(sockfd);
    return 0;
}
