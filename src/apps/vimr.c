
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
int Rterm = 0;

const char *FindRConsole(char *Rttl){
    RConsole = FindWindow(NULL, "R Console (64-bit)");
    if(!RConsole){
        RConsole = FindWindow(NULL, "R Console (32-bit)");
        if(!RConsole)
            RConsole = FindWindow(NULL, "R Console");
    }
    if(RConsole)
        strcpy(Reply, "OK");
    else
        strcpy(Reply, "NotFound");
    return(Reply);
}

static void RaiseRConsole(){
    SetForegroundWindow(RConsole);
    Sleep(atof(getenv("VIM_SLEEPTIME")) / 1000);
}

static void RightClick(){
    HWND myHandle = GetForegroundWindow();
    RaiseRConsole();
    LPARAM lParam = (100 << 16) | 100;
    SendMessage(RConsole, WM_RBUTTONDOWN, 0, lParam);
    SendMessage(RConsole, WM_RBUTTONUP, 0, lParam);
    Sleep(0.05);
    SetForegroundWindow(myHandle);
}

static void CntrlV(){
    // This is the most inefficient way of sending Ctrl+V. See:
    // http://stackoverflow.com/questions/27976500/postmessage-ctrlv-without-raising-the-window
    RaiseRConsole();
    keybd_event(VK_CONTROL, 0, 0, 0);
    keybd_event(VkKeyScan('V'), 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
    Sleep(0.05);
    keybd_event(VkKeyScan('V'), 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
}

static void CopyTxtToCB(char *str)
{
    const size_t len = strlen(str) + 1;
    HGLOBAL m = GlobalAlloc(GMEM_MOVEABLE, len);
    memcpy(GlobalLock(m), str, len);
    GlobalUnlock(m);
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, m);
    CloseClipboard();
}

const char *SendToRConsole(char *aString){
    if(!RConsole)
        FindRConsole(NULL);
    if(!RConsole){
        strcpy(Reply, "R Console not found");
        return(Reply);
    }

    SendToVimCom("\003Set R as busy [SendToRConsole()]");

    CopyTxtToCB(aString);
    if(Rterm)
        RightClick();
    else
        CntrlV();

    strcpy(Reply, "OK");
    return(Reply);
}

const char *RClearConsole(char *what){
    if(!RConsole)
        FindRConsole(NULL);
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

const char *SendQuitMsg(char *aString){
    SendToRConsole(aString);
    RConsole = NULL;
    return(Reply);
}

#endif
