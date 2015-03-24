
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef WIN32
#include <windows.h>
#include <process.h>
#ifdef _WIN64
#include <inttypes.h>
#endif
#else
#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#endif


/* Below are functions called by Vim */

static char Reply[256];

#ifndef WIN32
const char *SendToVimCom(char *instr)
{
    strcpy(Reply, "OK");
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, a;
    size_t len;

    char buf[1024];
    char *portnum = buf;
    char *msg = buf;
    strncpy(buf, instr, 1023);
    while(*msg != ' ')
        msg++;
    *msg = 0;
    msg++;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    a = getaddrinfo("127.0.0.1", portnum, &hints, &result);
    if (a != 0) {
        snprintf(Reply, 254, "Sending message to vimcom [getaddrinfo]: %s.\n", gai_strerror(a));
        return(Reply);
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
        sprintf(Reply, "Sending message to vimcom: could not connect.\n");
        return(Reply);
    }

    freeaddrinfo(result);	   /* No longer needed */

    len = strlen(msg);
    if (write(s, msg, len) == len) {
        sprintf(Reply, "OK");
    } else {
        sprintf(Reply, "Sending message to vimcom: partial/failed write.\n");
    }
    return(Reply);
}
#endif

#ifdef WIN32

const char *SendToVimCom(const char *instr)
{
    strcpy(Reply, "OK");
    WSADATA wsaData;
    struct sockaddr_in peer_addr;
    SOCKET sfd;

    char buf[1024];
    char *portnum = buf;
    char *msg = buf;
    strncpy(buf, instr, 1023);
    while(*msg != ' ')
        msg++;
    *msg = 0;
    msg++;

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(sfd < 0){
        sprintf(Reply, "Sending message to vimcom: socket failed.\n");
        return(Reply);
    }

    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(atoi(portnum));
    peer_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(connect(sfd, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0){
        sprintf(Reply, "Sending message to vimcom: could not connect.\n");
        return(Reply);
    }

    int len = strlen(msg);
    if (send(sfd, msg, len+1, 0) < 0) {
        sprintf(Reply, "Sending message to vimcom: failed sending message.\n");
        return(Reply);
    }

    if(closesocket(sfd) < 0)
        sprintf(Reply, "Sending message to vimcom: error closing socket.\n");
    return(Reply);
}

HWND RConsole = NULL;

const char *FindRConsole(char *Rttl){
    if(*Rttl == 'T'){
        RConsole = FindWindow(NULL, "RTerm (64-bit)");
        if(!RConsole){
            RConsole = FindWindow(NULL, "RTerm (32-bit)");
            if(!RConsole)
                RConsole = FindWindow(NULL, "RTerm");
        }
    } else {
        RConsole = FindWindow(NULL, "R Console (64-bit)");
        if(!RConsole){
            RConsole = FindWindow(NULL, "R Console (32-bit)");
            if(!RConsole)
                RConsole = FindWindow(NULL, "R Console");
        }
    }
    if(RConsole)
        strcpy(Reply, "OK");
    else
        strcpy(Reply, "NotFound");
    return(Reply);
}

static void RaiseRConsole(){
    SetForegroundWindow(RConsole);
    Sleep(0.05);
}

const char *SendToRTerm(char *aString){
    if(!RConsole)
        FindRConsole("Term");
    if(!RConsole){
        strcpy(Reply, "R Console not found");
        return(Reply);
    }

    SendToVimCom("\003Set R as busy [SendToRConsole()]");

    RaiseRConsole();
    LPARAM lParam = (100 << 16) | 100;
    SendMessage(RConsole, WM_RBUTTONDOWN, 0, lParam);
    SendMessage(RConsole, WM_RBUTTONUP, 0, lParam);
    Sleep(0.05);

    strcpy(Reply, "OK");
    return(Reply);
}

const char *SendToRConsole(char *aString){
    if(!RConsole)
        FindRConsole("Rgui");
    if(!RConsole){
        strcpy(Reply, "R Console not found");
        return(Reply);
    }

    SendToVimCom("\003Set R as busy [SendToRConsole()]");

    // This is the most inefficient way of sending Ctrl+V. See:
    // http://stackoverflow.com/questions/27976500/postmessage-ctrlv-without-raising-the-window
    RaiseRConsole();
    keybd_event(VK_CONTROL, 0, 0, 0);
    keybd_event(VkKeyScan('V'), 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
    Sleep(0.05);
    keybd_event(VkKeyScan('V'), 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);

    strcpy(Reply, "OK");
    return(Reply);
}

const char *RClearConsole(char *what){
    if(!RConsole)
        FindRConsole(what);
    if(!RConsole){
        strcpy(Reply, "R Console not found");
        return(Reply);
    }

    RaiseRConsole();
    keybd_event(VK_CONTROL, 0, 0, 0);
    keybd_event(VkKeyScan('L'), 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
    Sleep(0.05);
    keybd_event(VkKeyScan('L'), 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);

    strcpy(Reply, "OK");
    return(Reply);
}

const char *SaveWinPos(char *cachedir){
    if(!RConsole)
        FindRConsole("RGui");
    if(!RConsole){
        strcpy(Reply, "R Console not found");
        return(Reply);
    }

    HWND GVimHwnd = GetActiveWindow();
    if(!GVimHwnd){
        snprintf(Reply, 254, "Could not get active window");
        return(Reply);
    }

    RECT rcR, rcV;
    if(!GetWindowRect(RConsole, &rcR)){
        strcpy(Reply, "Could not get R Console position");
        return(Reply);
    }

    if(!GetWindowRect(GVimHwnd, &rcV)){
        strcpy(Reply, "Could not get GVim position");
        return(Reply);
    }

    rcR.right = rcR.right - rcR.left;
    rcR.bottom = rcR.bottom - rcR.top;
    rcV.right = rcV.right - rcV.left;
    rcV.bottom = rcV.bottom - rcV.top;

    char fname[512];
    snprintf(fname, 511, "%s/win_pos", cachedir);
    FILE *f = fopen(fname, "w");
    if(f == NULL){
        snprintf(Reply, 254, "Could not write to '%s'", fname);
        return(Reply);
    }
    fprintf(f, "%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n",
            rcR.left, rcR.top, rcR.right, rcR.bottom,
            rcV.left, rcV.top, rcV.right, rcV.bottom);
    fclose(f);

    strcpy(Reply, "OK");
    return(Reply);
}

const char *ArrangeWindows(char *cachedir){
    char fname[512];
    snprintf(fname, 511, "%s/win_pos", cachedir);
    FILE *f = fopen(fname, "r");
    if(f == NULL){
        snprintf(Reply, 254, "Could not read '%s'", fname);
        return(Reply);
    }

    if(!RConsole)
        FindRConsole("RGui");
    if(!RConsole){
        strcpy(Reply, "R Console not found");
        return(Reply);
    }

    HWND GVimHwnd = GetActiveWindow();
    if(!GVimHwnd){
        snprintf(Reply, 254, "Could not get active window");
        return(Reply);
    }

    RECT rcR, rcV;
    char b[32];
    if((fgets(b, 31, f))){
        rcR.left = atol(b);
    } else {
        strcpy(Reply, "Error reading R left position");
        return(Reply);
    }
    if((fgets(b, 31, f))){
        rcR.top = atol(b);
    } else {
        strcpy(Reply, "Error reading R top position");
        return(Reply);
    }
    if((fgets(b, 31, f))){
        rcR.right = atol(b);
    } else {
        strcpy(Reply, "Error reading R right position");
        return(Reply);
    }
    if((fgets(b, 31, f))){
        rcR.bottom = atol(b);
    } else {
        strcpy(Reply, "Error reading R bottom position");
        return(Reply);
    }
    if((fgets(b, 31, f))){
        rcV.left = atol(b);
    } else {
        strcpy(Reply, "Error reading GVim left position");
        return(Reply);
    }
    if((fgets(b, 31, f))){
        rcV.top = atol(b);
    } else {
        strcpy(Reply, "Error reading GVim top position");
        return(Reply);
    }
    if((fgets(b, 31, f))){
        rcV.right = atol(b);
    } else {
        strcpy(Reply, "Error reading GVim right position");
        return(Reply);
    }
    if((fgets(b, 31, f))){
        rcV.bottom = atol(b);
    } else {
        strcpy(Reply, "Error reading GVim bottom position");
        return(Reply);
    }

    if(!SetWindowPos(RConsole, HWND_NOTOPMOST,
                rcR.left, rcR.top, rcR.right, rcR.bottom, 0)){
        strcpy(Reply, "Error positioning GVim window");
        return(Reply);
    }
    if(!SetWindowPos(GVimHwnd, HWND_NOTOPMOST,
                rcV.left, rcV.top, rcV.right, rcV.bottom, 0)){
        strcpy(Reply, "Error positioning GVim window");
        return(Reply);
    }

    strcpy(Reply, "OK");
    return(Reply);
}

#endif
