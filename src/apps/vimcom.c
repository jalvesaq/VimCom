
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
    char fn[512];
    snprintf(fn, 510, "%s/rconsole_hwnd_%s", getenv("VIMRPLUGIN_TMPDIR"), getenv("VIMRPLUGIN_SECRET"));
    FILE *h = fopen(fn, "r");
    if(h){
        fread(&RConsole, sizeof(HWND), 1, h);
        fclose(h);
        char buf[256];
        if(GetWindowText(RConsole, buf, 255)){
            snprintf(Reply, 255, "let g:rplugin_R_window_ttl = '%s'", buf);
            return(Reply);
        }
    }
    RConsole = NULL;
    strcpy(Reply, "NotFound");
    return(Reply);
}

static void RaiseRConsole(){
    FindRConsole("R Console");
    if(RConsole){
        SetForegroundWindow(RConsole);
        Sleep(0.1);
    }
}

static void RightClick(){
    HWND myHandle = GetForegroundWindow();
    RaiseRConsole();
    Sleep(0.05);
    LPARAM lParam = (100 << 16) | 100;
    SendMessage(RConsole, WM_RBUTTONDOWN, 0, lParam);
    SendMessage(RConsole, WM_RBUTTONUP, 0, lParam);
    Sleep(0.05);
    SetForegroundWindow(myHandle);
}

static void CntrlV(){
    /* FIXME: This function isn't working!
       See: https://github.com/jcfaria/Vim-R-plugin/issues/144
       Below are some failed attempts of writing it. */


    /* Code that used to work in Python. Code was sent to R Console without
     * raising its window.
    keybd_event(0x11, 0, 0, 0);
    if(!PostMessage(RConsole, 0x100, 0x56, 0x002F0001))
        RConsole = NULL;
    if(RConsole){
        Sleep(0.05);
        PostMessage(RConsole, 0x101, 0x56, 0xC02F0001);
    }
    keybd_event(0x11, 0, 2, 0);
    */


    /* This is an attempt of using only PostMessage, but I don't know how to
     * write the last argument (lParam).
    // Send CTRL down
    PostMessage(RConsole, WM_SYSKEYDOWN, MapVirtualKey(VK_CONTROL, 0), 0);

    // Send V down and up
    PostMessage(RConsole, 0x100, 0x56, 0x002F0001);
    Sleep(50);
    PostMessage(RConsole, 0x101, 0x56, 0xC02F0001 );

    // Send CTRL up
    PostMessage(RConsole, WM_SYSKEYUP, MapVirtualKey(VK_CONTROL, 0), 0 );
    */

    // Copied from SendQuitMsg()
    strcpy(Reply, "OK");
    HWND myHandle = GetForegroundWindow();
    RaiseRConsole();
    if(RConsole && !Rterm){
        Sleep(0.1);
        keybd_event(VK_CONTROL, 0, 0, 0);
        keybd_event(VkKeyScan('V'), 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
        Sleep(0.05);
        keybd_event(VkKeyScan('V'), 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
        keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
        Sleep(0.05);
    }
    if(RConsole && Rterm){
        RightClick();
    }
    Sleep(0.05);
    SetForegroundWindow(myHandle);
}

const char *SendToRConsole(char *aString){
    SendToVimCom("\003Set R as busy [SendToRConsole()]");
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, aString);
    CloseClipboard();
    if(!RConsole)
        FindRConsole("R Console");
    if(RConsole){
        if(Rterm)
            RightClick();
        else
            CntrlV();
    }
    return NULL;
}

const char *RClearConsole(char *what){
    strcpy(Reply, "OK");
    if(strcmp(what, "Rterm"))
        return(Reply);
    if(!RConsole)
        FindRConsole("R Console");
    if(RConsole){
        keybd_event(VK_CONTROL, 0, 0, 0);
        if(!PostMessage(RConsole, 0x100, 0x4C, 0x002F0001)){
            strcpy(Reply, "R Console window not found [1].");
            RConsole = NULL;
        }
        if(RConsole){
            Sleep(0.05);
            if(!PostMessage(RConsole, 0x101, 0x4C, 0xC02F0001))
                strcpy(Reply, "R Console window not found [2].");
        }
        keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
    }
    return(Reply);
}

const char *SendQuitMsg(char *aString){
    strcpy(Reply, "OK");
    SendToVimCom("\003Set R as busy [SendQuitMsg()]");
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, aString);
    CloseClipboard();
    RaiseRConsole();
    if(RConsole && !Rterm){
        Sleep(0.1);
        keybd_event(VK_CONTROL, 0, 0, 0);
        keybd_event(VkKeyScan('V'), 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
        Sleep(0.05);
        keybd_event(VkKeyScan('V'), 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
        keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
        Sleep(0.05);
        RConsole = NULL;
    }
    if(RConsole && Rterm){
        RightClick();
        RConsole = NULL;
    }
    return(Reply);
}

const char *OpenPDF(char *fn){
    if(ShellExecute(NULL, "open", fn, NULL, NULL, SW_SHOW))
        strcpy(Reply, "OK");
    else
        strcpy(Reply, "Failed to open PDF.");
    return(Reply);
}

#endif
