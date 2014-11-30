#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#endif

static char VimSecret[128];
static int VimSecretLen;

static void NeovimServer()
{
    unsigned short bindportn = 1899;
    ssize_t nsent;
    ssize_t nread;
    int bsize = 5012;
    char buf[bsize];
    char rep[bsize];
    int result;

#ifdef WIN32
    WSADATA wsaData;
    SOCKADDR_IN RecvAddr;
    SOCKADDR_IN peer_addr;
    int peer_addr_len = sizeof (peer_addr);
    int nattp = 0;
    int nfail = 0;
    int lastfail = 0;
    SOCKET sfd;

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != NO_ERROR) {
        fprintf(stderr, "Neovim server: WSAStartup failed with error %d.\n", result);
        fflush(stderr);
        return;
    }

    while(bindportn < 1999){
        bindportn++;
        sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sfd == INVALID_SOCKET) {
            fprintf(stderr, "Neovim server: socket failed with error %d\n", WSAGetLastError());
            fflush(stderr);
            return;
        }

        RecvAddr.sin_family = AF_INET;
        RecvAddr.sin_port = htons(bindportn);
        RecvAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        nattp++;
        if(bind(sfd, (SOCKADDR *) & RecvAddr, sizeof (RecvAddr)) == 0)
            break;
        lastfail = WSAGetLastError();
        nfail++;
        fprintf(stderr, "Neovim server: Could not bind to port %d [error  %d].\n", bindportn, lastfail);
        fflush(stderr);
    }
    if(nattp == nfail){
        fprintf(stderr, "Neovim server: Could not bind.\n");
        fflush(stderr);
        return;
    }
#else
    struct addrinfo hints;
    struct addrinfo *rp;
    struct addrinfo *res;
    struct sockaddr_storage peer_addr;
    int sfd = -1;
    char bindport[16];
    socklen_t peer_addr_len = sizeof(struct sockaddr_storage);

    // block SIGINT
    {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGINT);
        sigprocmask(SIG_BLOCK, &set, NULL);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    rp = NULL;
    result = 1;
    while(rp == NULL && bindportn < 10049){
        bindportn++;
        sprintf(bindport, "%d", bindportn);
        result = getaddrinfo("127.0.0.1", bindport, &hints, &res);
        if(result != 0){
            fprintf(stderr, "Neovim server: Error at getaddrinfo (%s)\n", gai_strerror(result));
            fflush(stderr);
            return;
        }

        for (rp = res; rp != NULL; rp = rp->ai_next) {
            sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (sfd == -1)
                continue;
            if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
                break;       /* Success */
            close(sfd);
        }
        freeaddrinfo(res);   /* No longer needed */
    }

    if (rp == NULL) {        /* No address succeeded */
        fprintf(stderr, "Neovim server: Could not bind.\n");
        fflush(stderr);
        return;
    }
#endif

    // Register the port:
    printf("call RSetNeovimPort('%d')\n", bindportn);
    fflush(stdout);

    /* Read datagrams and reply to sender */
    for (;;) {
        memset(buf, 0, bsize);
        memset(rep, 0, bsize);
        strcpy(rep, "UNKNOWN");

#ifdef WIN32
        nread = recvfrom(sfd, buf, bsize, 0,
                (SOCKADDR *) &peer_addr, &peer_addr_len);
        if (nread == SOCKET_ERROR) {
            fprintf(stderr, "Neovim server: recvfrom failed with error %d\n", WSAGetLastError());
            fflush(stderr);
            return;
        }
#else
        nread = recvfrom(sfd, buf, bsize, 0,
                (struct sockaddr *) &peer_addr, &peer_addr_len);
        if (nread == -1){
            fprintf(stderr, "Neovim server: recvfrom failed\n");
            fflush(stderr);
            continue;     /* Ignore failed request */
        }
#endif

        char *bbuf = buf;

        if(strstr(bbuf, VimSecret)){
            bbuf += VimSecretLen;
            printf("%s\n", bbuf);
            fflush(stdout);
        } else {
            fprintf(stderr, "Strange string received: \"%s\"\n", bbuf);
            fflush(stderr);
        }

            nsent = strlen(rep);
        if (sendto(sfd, rep, nsent, 0, (struct sockaddr *) &peer_addr, peer_addr_len) != nsent)
            fprintf(stderr, "Neovim server: Error sending response. [vimcom]\n");
            fflush(stderr);

    }
#ifdef WIN32
    fprintf(stderr, "Neovim server: Finished receiving. Closing socket.\n");
    fflush(stderr);
    result = closesocket(sfd);
    if (result == SOCKET_ERROR) {
        fprintf(stderr, "Neovim server: closesocket failed with error %d\n", WSAGetLastError());
        fflush(stderr);
        return;
    }
    WSACleanup();
#endif
    return;
}

int main(int argc, char **argv){
    if(!getenv("VIMRPLUGIN_SECRET")){
        fprintf(stderr, "VIMRPLUGIN_SECRET not found\n");
        fflush(stderr);
        exit(1);
    }
    strncpy(VimSecret, getenv("VIMRPLUGIN_SECRET"), 127);
    VimSecretLen = strlen(VimSecret);
#ifdef WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
    NeovimServer();
    return 0;
}
