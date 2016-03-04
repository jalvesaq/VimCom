#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#ifdef WIN32
#include <winsock2.h>
#include <process.h>
#include <windows.h>
HWND GVimHwnd = NULL;
HWND RConsole = NULL;
#else
#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#endif

static char VimcomPort[16];
static char VimSecret[128];
static int VimSecretLen;
FILE *df = NULL;

#ifdef WIN32
static int tid;
#else
static pthread_t tid;
#endif

static void HandleSigTerm(int s)
{
    if(df){
        fprintf(df, "HandleSigTerm called\n");
        fflush(df);
    }
    exit(0);
}

static void RegisterPort(int bindportn)
{
    if(df){
        fprintf(df, "Sending RSetMyPort message to: %d\n", bindportn);
        fflush(df);
    }

    // Register the port:
    printf("call RSetMyPort('%d')\n", bindportn);
    fflush(stdout);

    //char buf[128];
    //snprintf(buf, 127, "\001%d", bindportn);
    //SendToServer(VimcomPort, buf);
}

static void ParseMsg(char *buf)
{
    char *bbuf = buf;
    if(df){
        fprintf(df, "tcp: %s\n", bbuf);
        fflush(df);
    }

    if(strstr(bbuf, VimSecret) == bbuf){
        bbuf += VimSecretLen;
        printf("%s\n", bbuf);
        fflush(stdout);
    } else {
        fprintf(stderr, "Strange string received: \"%s\"\n", bbuf);
        fflush(stderr);
        if(df){
            fprintf(df, "Strange string received: \"%s\"\n", bbuf);
            fflush(df);
        }
    }
}

#ifndef WIN32
static void *VimServer(void *arg)
{
    unsigned short bindportn = 10100;
    ssize_t nread;
    int bsize = 5012;
    char buf[bsize];
    int result;

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
    while(rp == NULL && bindportn < 10149){
        bindportn++;
        sprintf(bindport, "%d", bindportn);
        result = getaddrinfo("127.0.0.1", bindport, &hints, &res);
        if(result != 0){
            fprintf(stderr, "Error at getaddrinfo (%s)\n", gai_strerror(result));
            fflush(stderr);
            return NULL;
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
        fprintf(stderr, "Could not bind\n");
        fflush(stderr);
        return NULL;
    }

    RegisterPort(bindportn);

    /* Read datagrams and reply to sender */
    for (;;) {
        memset(buf, 0, bsize);

        nread = recvfrom(sfd, buf, bsize, 0,
                (struct sockaddr *) &peer_addr, &peer_addr_len);
        if (nread == -1){
            fprintf(stderr, "recvfrom failed [port %d]\n", bindportn);
            fflush(stderr);
            continue;     /* Ignore failed request */
        }
        if(strstr(buf, "QUIT_VINSERVER_NOW"))
            break;

        ParseMsg(buf);
    }
    if(df){
        fclose(df);
        df = NULL;
    }
    return NULL;
}
#endif

#ifdef WIN32
static void VimServer(void *arg)
{
    unsigned short bindportn = 10100;
    ssize_t nread;
    int bsize = 5012;
    char buf[bsize];
    int result;

    WSADATA wsaData;
    SOCKADDR_IN RecvAddr;
    SOCKADDR_IN peer_addr;
    SOCKET sfd;
    int peer_addr_len = sizeof (peer_addr);
    int nattp = 0;
    int nfail = 0;

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != NO_ERROR) {
        fprintf(stderr, "WSAStartup failed with error %d.\n", result);
        fflush(stderr);
        return;
    }

    while(bindportn < 10149){
        bindportn++;
        sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sfd == INVALID_SOCKET) {
            fprintf(stderr, "socket failed with error %d\n", WSAGetLastError());
            fflush(stderr);
            return;
        }

        RecvAddr.sin_family = AF_INET;
        RecvAddr.sin_port = htons(bindportn);
        RecvAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        nattp++;
        if(bind(sfd, (SOCKADDR *) & RecvAddr, sizeof (RecvAddr)) == 0)
            break;
        nfail++;
    }
    if(nattp == nfail){
        fprintf(stderr, "Could not bind\n");
        fflush(stderr);
        return;
    }

    RegisterPort(bindportn);

    /* Read datagrams and reply to sender */
    for (;;) {
        memset(buf, 0, bsize);

        nread = recvfrom(sfd, buf, bsize, 0,
                (SOCKADDR *) &peer_addr, &peer_addr_len);
        if (nread == SOCKET_ERROR) {
            fprintf(stderr, "recvfrom failed with error %d [port %d]\n",
                    bindportn, WSAGetLastError());
            fflush(stderr);
            return;
        }
        if(strstr(buf, "QUIT_VINSERVER_NOW"))
            break;

        ParseMsg(buf);
    }
    if(df){
        fprintf(df, "Vim server: Finished receiving. Closing socket.\n");
        fflush(df);
    }
    result = closesocket(sfd);
    if (result == SOCKET_ERROR) {
        fprintf(stderr, "closesocket failed with error %d\n", WSAGetLastError());
        fflush(stderr);
        return;
    }
    WSACleanup();
    if(df){
        fclose(df);
        df = NULL;
    }
    return;
}
#endif

#ifndef WIN32
static void SendToServer(const char *port, const char *msg)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, a;
    size_t len;

    /* Obtain address(es) matching host/port */
    if(strncmp(port, "0", 15) == 0){
        fprintf(stderr, "Port is 0\n");
        fflush(stderr);
        return;
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    a = getaddrinfo("127.0.0.1", port, &hints, &result);
    if (a != 0) {
        fprintf(stderr, "Error in getaddrinfo [port = '%s'] [msg = '%s']: %s\n", port, msg, gai_strerror(a));
        fflush(stderr);
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
        fprintf(stderr, "Could not connect.\n");
        fflush(stderr);
        return;
    }

    freeaddrinfo(result);	   /* No longer needed */

    len = strlen(msg);
    if (write(s, msg, len) != (ssize_t)len) {
        fprintf(stderr, "Partial/failed write.\n");
        fflush(stderr);
        return;
    }
}
#endif

#ifdef WIN32
static void SendToServer(const char *port, const char *msg)
{
    WSADATA wsaData;
    struct sockaddr_in peer_addr;
    SOCKET sfd;

    if(strncmp(port, "0", 15) == 0){
        fprintf(stderr, "Port is 0\n");
        fflush(stderr);
        return;
    }

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(sfd < 0){
        fprintf(stderr, "Socket failed\n");
        fflush(stderr);
        return;
    }

    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(atoi(port));
    peer_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(connect(sfd, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0){
        fprintf(stderr, "Could not connect\n");
        fflush(stderr);
        return;
    }

    int len = strlen(msg);
    if (send(sfd, msg, len+1, 0) < 0) {
        fprintf(stderr, "Failed sending message\n");
        fflush(stderr);
        return;
    }

    if(closesocket(sfd) < 0){
        fprintf(stderr, "Error closing socket\n");
        fflush(stderr);
    }
}

static void RClearConsole(){
    if(!RConsole){
        fprintf(stderr, "R Console window ID not defined [RClearConsole]\n");
        fflush(stderr);
        return;
    }

    SetForegroundWindow(RConsole);
    keybd_event(VK_CONTROL, 0, 0, 0);
    keybd_event(VkKeyScan('L'), 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
    Sleep(0.05);
    keybd_event(VkKeyScan('L'), 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
}

static void SaveWinPos(char *cachedir){
    if(!RConsole){
        fprintf(stderr, "R Console window ID not defined [SaveWinPos]\n");
        fflush(stderr);
        return;
    }

    RECT rcR, rcV;
    if(!GetWindowRect(RConsole, &rcR)){
        fprintf(stderr, "Could not get R Console position\n");
        fflush(stderr);
        return;
    }

    if(!GetWindowRect(GVimHwnd, &rcV)){
        fprintf(stderr, "Could not get GVim position\n");
        fflush(stderr);
        return;
    }

    rcR.right = rcR.right - rcR.left;
    rcR.bottom = rcR.bottom - rcR.top;
    rcV.right = rcV.right - rcV.left;
    rcV.bottom = rcV.bottom - rcV.top;

    char fname[512];
    snprintf(fname, 511, "%s/win_pos", cachedir);
    FILE *f = fopen(fname, "w");
    if(f == NULL){
        fprintf(stderr, "Could not write to '%s'\n", fname);
        fflush(stderr);
        return;
    }
    fprintf(f, "%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n",
            rcR.left, rcR.top, rcR.right, rcR.bottom,
            rcV.left, rcV.top, rcV.right, rcV.bottom);
    fclose(f);
}

static void ArrangeWindows(char *cachedir){
    if(!RConsole){
        fprintf(stderr, "R Console window ID not defined [ArrangeWindows]\n");
        fflush(stderr);
        return;
    }

    char fname[512];
    snprintf(fname, 511, "%s/win_pos", cachedir);
    FILE *f = fopen(fname, "r");
    if(f == NULL){
        fprintf(stderr, "Could not read '%s'\n", fname);
        fflush(stderr);
        return;
    }

    RECT rcR, rcV;
    char b[32];
    if((fgets(b, 31, f))){
        rcR.left = atol(b);
    } else {
        fprintf(stderr, "Error reading R left position\n");
        fflush(stderr);
        fclose(f);
        return;
    }
    if((fgets(b, 31, f))){
        rcR.top = atol(b);
    } else {
        fprintf(stderr, "Error reading R top position\n");
        fflush(stderr);
        fclose(f);
        return;
    }
    if((fgets(b, 31, f))){
        rcR.right = atol(b);
    } else {
        fprintf(stderr, "Error reading R right position\n");
        fflush(stderr);
        fclose(f);
        return;
    }
    if((fgets(b, 31, f))){
        rcR.bottom = atol(b);
    } else {
        fprintf(stderr, "Error reading R bottom position\n");
        fflush(stderr);
        fclose(f);
        return;
    }
    if((fgets(b, 31, f))){
        rcV.left = atol(b);
    } else {
        fprintf(stderr, "Error reading GVim left position\n");
        fflush(stderr);
        fclose(f);
        return;
    }
    if((fgets(b, 31, f))){
        rcV.top = atol(b);
    } else {
        fprintf(stderr, "Error reading GVim top position\n");
        fflush(stderr);
        fclose(f);
        return;
    }
    if((fgets(b, 31, f))){
        rcV.right = atol(b);
    } else {
        fprintf(stderr, "Error reading GVim right position\n");
        fflush(stderr);
        fclose(f);
        return;
    }
    if((fgets(b, 31, f))){
        rcV.bottom = atol(b);
    } else {
        fprintf(stderr, "Error reading GVim bottom position\n");
        fflush(stderr);
        fclose(f);
        return;
    }

    if(rcR.left > 0 && rcR.top > 0 && rcR.right > 0 && rcR.bottom > 0 &&
            rcR.right > rcR.left && rcR.bottom > rcR.top){
        if(!SetWindowPos(RConsole, HWND_TOP,
                    rcR.left, rcR.top, rcR.right, rcR.bottom, 0)){
            fprintf(stderr, "Error positioning RConsole window\n");
            fflush(stderr);
            fclose(f);
            return;
        }
    }

    if(rcV.left > 0 && rcV.top > 0 && rcV.right > 0 && rcV.bottom > 0 &&
            rcV.right > rcV.left && rcV.bottom > rcV.top){
        if(!SetWindowPos(GVimHwnd, HWND_TOP,
                    rcV.left, rcV.top, rcV.right, rcV.bottom, 0)){
            fprintf(stderr, "Error positioning GVim window\n");
            fflush(stderr);
        }
    }

    SetForegroundWindow(GVimHwnd);
    fclose(f);
}
#endif

int main(int argc, char **argv){
    char line[1024];
    char *msg;
    memset(line, 0, 1024);
    strcpy(VimcomPort, "0");

    if(getenv("DEBUG_VIMR")){
        df = fopen("/tmp/vclientserver_debug", "a");
        if(df){
            fprintf(df, "VIMR_SECRET=%s VIMCOMPORT=%s\n",
                    getenv("VIMR_SECRET"), getenv("VIMCOMPORT"));
            fflush(df);
        } else {
            fprintf(stderr, "Error opening \"nclientserver_debug\" for writing\n");
            fflush(stderr);
        }
    }

    if(argc == 3 && getenv("VIMR_PORT") && getenv("VIMR_SECRET")){
        snprintf(line, 1023, "%scall SyncTeX_backward('%s', %s)", getenv("VIMR_SECRET"), argv[1], argv[2]);
        SendToServer(getenv("VIMR_PORT"), line);
        if(df){
            fprintf(df, "%s %s %s %s\n", getenv("VIMR_PORT"), getenv("VIMR_SECRET"), argv[1], argv[2]);
            fclose(df);
        }
        return 0;
    }

    // Set vimcom port
    if(getenv("VIMCOMPORT"))
        strcpy(VimcomPort, getenv("VIMCOMPORT"));

#ifdef WIN32
    // Set the value of RConsole
    if(getenv("RCONSOLE")){
#ifdef _WIN64
        RConsole = (HWND)atoll(getenv("RCONSOLE"));
#else
        RConsole = (HWND)atol(getenv("RCONSOLE"));
#endif
    } else {
        fprintf(stderr, "$RCONSOLE not defined\n");
        fflush(stderr);

        RConsole = FindWindow(NULL, "R Console (64-bit)");
        if(!RConsole){
            RConsole = FindWindow(NULL, "R Console (32-bit)");
            if(!RConsole)
                RConsole = FindWindow(NULL, "R Console");
        }
        if(!RConsole){
            fprintf(stderr, "\"R Console\" window not found\n");
            fflush(stderr);
        }
    }

    // Set the value of GVimHwnd
    if(getenv("WINDOWID")){
#ifdef _WIN64
        GVimHwnd = (HWND)atoll(getenv("WINDOWID"));
#else
        GVimHwnd = (HWND)atol(getenv("WINDOWID"));
#endif
    } else {
        fprintf(stderr, "$WINDOWID not defined\n");
        fflush(stderr);
        GVimHwnd = FindWindow(NULL, "GVim");
        if(!GVimHwnd){
            fprintf(stderr, "\"GVim\" window not found\n");
            fflush(stderr);
        }
    }
#endif

    // Start the server
    if(!getenv("VIMR_SECRET")){
        fprintf(stderr, "VIMR_SECRET not found\n");
        fflush(stderr);
        exit(1);
    }
    strncpy(VimSecret, getenv("VIMR_SECRET"), 127);
    VimSecretLen = strlen(VimSecret);

    // Finish immediately with SIGTERM
    signal(SIGTERM, HandleSigTerm);

#ifdef WIN32
    Sleep(1000);
#else
    sleep(1);
#endif

#ifdef WIN32
    tid = _beginthread(VimServer, 0, NULL);
#else
    pthread_create(&tid, NULL, VimServer, NULL);
#endif

    while(fgets(line, 1023, stdin)){
        if(df){
            msg = line;
            msg++;
            fprintf(df, "stdin: [%d] %s", (unsigned int)*line, msg);
            fflush(df);
        }

        for(unsigned int i = 0; i < strlen(line); i++)
            if(line[i] == '\n' || line[i] == '\r')
                line[i] = 0;
        msg = line;
        switch(*msg){
            case 1: // SetPort
                msg++;
                strncpy(VimcomPort, msg, 15);
#ifdef WIN32
                if(msg[0] == '0')
                    RConsole = NULL;
#endif
                break;
            case 2: // Send message
                msg++;
                SendToServer(VimcomPort, msg);
                break;
#ifdef WIN32
            case 4: // SaveWinPos
                msg++;
                SaveWinPos(msg);
                break;
            case 5: // ArrangeWindows
                msg++;
                ArrangeWindows(msg);
                break;
            case 6:
                RClearConsole();
                break;
            case 7: // RaiseGVimWindow
                if(GVimHwnd)
                    SetForegroundWindow(GVimHwnd);
                break;
#endif
            default:
                fprintf(stderr, "Unknown command received: [%d] %s\n", line[0], msg);
                fflush(stderr);
                break;
        }
        memset(line, 0, 1024);
    }
    if(df)
        fclose(df);
    return 0;
}
