#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <process.h>
#else
#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

static int VimComPort;

static void SendToVimCom(const char *msg)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    char portstr[16];
    int s, a;
    size_t len;

    /* Obtain address(es) matching host/port */

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    sprintf(portstr, "%d", VimComPort);
    a = getaddrinfo("localhost", portstr, &hints, &result);
    if (a != 0) {
        fprintf(stderr, "Error [nvimclient.c]: getaddrinfo: %s\n", gai_strerror(a));
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
        fprintf(stderr, "Error [nvimclient.c]: Could not connect\n");
        return;
    }

    freeaddrinfo(result);	   /* No longer needed */

    /* Prefix VIMRPLUGIN_SECRET to msg to increase security.
     * The vimclient does not need this because it is protect by the X server. */
    len = strlen(msg);
    if (write(s, msg, len) != len) {
        fprintf(stderr, "Error [nvimclient.c]: partial/failed write\n");
        return;
    }
}

int main(int argc, char **argv){
    char line[1024];

    VimComPort = atoi(argv[1]);

    while(fgets(line, 1023, stdin))
        SendToVimCom(line);
    return 0;
}

