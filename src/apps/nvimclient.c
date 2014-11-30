#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef WIN32
#include <winsock2.h>
#else
#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

static char VimComPort[32];

#ifndef WIN32
static void SendToVimCom(const char *msg)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, a;
    size_t len;

    /* Obtain address(es) matching host/port */

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    a = getaddrinfo("127.0.0.1", VimComPort, &hints, &result);
    if (a != 0) {
        fprintf(stderr, "Neovim client error [getaddrinfo]: %s.\n", gai_strerror(a));
        return;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        s = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
        if (s == -1)
            continue;

        if (connect(s, rp->ai_addr, rp->ai_addrlen) != -1)
            break;		   /* Success */

        close(s);
    }

    if (rp == NULL) {		   /* No address succeeded */
        fprintf(stderr, "Neovim client could not connect.\n");
        return;
    }

    freeaddrinfo(result);	   /* No longer needed */

    len = strlen(msg);
    if (write(s, msg, len) != len) {
        fprintf(stderr, "Neovim client partial/failed write.\n");
        return;
    }
}
#endif

#ifdef WIN32
static void SendToVimCom(const char *msg)
{

    WSADATA wsaData;
    struct sockaddr_in peer_addr;
    SOCKET sfd;

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(sfd < 0){
        fprintf(stderr, "Neovim client socket failed.\n");
        return;
    }

    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(atoi(VimComPort));
    peer_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(connect(sfd, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0){
        fprintf(stderr, "Neovim client could not connect.\n");
        return;
    }

    int len = strlen(msg);
    if (send(sfd, msg, len+1, 0) < 0) {
        fprintf(stderr, "Neovim client failed sending message.\n");
        return;
    }

    if(closesocket(sfd) < 0)
        fprintf(stderr, "Neovim client error closing socket.\n");
}
#endif

int main(int argc, char **argv){
    char line[1024];

    strncpy(VimComPort, argv[1], 31);

    while(fgets(line, 1023, stdin)){
        for(int i = 0; i < strlen(line); i++)
            if(line[i] == '\n')
                line[i] = 0;
        SendToVimCom(line);
    }
    return 0;
}

