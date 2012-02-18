
#define R_INTERFACE_PTRS 1

#include <R.h>  /* to include Rconfig.h */
#include <Rinternals.h>
#ifndef WIN32
#include <Rinterface.h>
#endif
#include <R_ext/Parse.h>
#include <R_ext/Callbacks.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#ifdef WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <process.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
static void (*save_pt_R_Busy)(int);
static int vimcom_relist = 0;
#endif

static int vimcom_initialized = 0;
static int verbose = 0;
static int opendf = 1;
static int openls = 0;
static int allnames = 0;
static int vimcom_is_utf8;
static int vimcom_failure = 0;
static int nlibs = 0;
static int nobjs = 0;
static int obport = 0;
static char strL[16];
static char strT[16];
static char tmpdir[512];
static int objbr_auto = 0;
static int has_new_lib = 0;
static int has_new_obj = 0;

typedef struct liststatus_ {
    char *key;
    int status;
    struct liststatus_ *next;
} ListStatus;

static ListStatus *firstList = NULL;

#ifdef WIN32
SOCKET sfd;
static int tid;
#else
static int sfd = -1;
static pthread_t tid;
#endif

static void vimcom_toggle_list_status(const char *x)
{
    ListStatus *tmp = firstList;
    while(tmp){
        if(strcmp(tmp->key, x) == 0){
            tmp->status = !tmp->status;
            break;
        }
        tmp = tmp->next;
    }
}

static void vimcom_add_list(const char *x, int s)
{
    ListStatus *tmp = firstList;
    while(tmp->next)
        tmp = tmp->next;
    tmp->next = (ListStatus*)calloc(1, sizeof(ListStatus));
    tmp->next->key = (char*)malloc((strlen(x) + 1) * sizeof(char));
    strcpy(tmp->next->key, x);
    tmp->next->status = s;
}

static int vimcom_get_list_status(const char *x, const char *xclass)
{
    ListStatus *tmp = firstList;
    while(tmp){
        if(strcmp(tmp->key, x) == 0)
            return(tmp->status);
        tmp = tmp->next;
    }
    if(strcmp(xclass, "data.frame") == 0){
        vimcom_add_list(x, opendf);
        return(opendf);
    } else if(strcmp(xclass, "list") == 0){
        vimcom_add_list(x, openls);
        return(openls);
    } else {
        vimcom_add_list(x, 0);
        return(0);
    }
}

static void vimcom_count_elements(SEXP *x)
{
    SEXP elmt;
    int len = length(*x);
    for(int i = 0; i < len; i++){
        nobjs++;
        elmt = VECTOR_ELT(*x, i);
        if(Rf_isNewList(elmt))
            vimcom_count_elements(&elmt);
    }
}

static int vimcom_count_objects()
{
    int np = 1;
    const char *varName;
    SEXP envVarsSEXP;

    int oldcount = nobjs;
    nobjs = 0;

    PROTECT(envVarsSEXP = R_lsInternal(R_GlobalEnv, 0));
    for(int i = 0; i < Rf_length(envVarsSEXP); i++){
        varName = CHAR(STRING_ELT(envVarsSEXP, i));
        SEXP varSEXP = Rf_findVar(Rf_install(varName), R_GlobalEnv);
        if (varSEXP != R_UnboundValue) // should never be unbound 
        {
            PROTECT(varSEXP);
            np++;
            nobjs++;
            if(Rf_isNewList(varSEXP))
                vimcom_count_elements(&varSEXP);
        } else {
            REprintf("Unexpected R_UnboundValue returned from R_lsInternal");
        }
    }
    UNPROTECT(np);

    return(oldcount != nobjs);
}

static void vimcom_browser_line(SEXP *x, const char *xname, const char *curenv, const char *prefix, FILE *f)
{
    char xclass[64];
    char newenv[512];
    char ebuf[64];
    char pre[128];
    char newpre[128];
    const char *ename;
    SEXP listNames, label, lablab, eexp, elmt = R_NilValue;

    if(Rf_isLogical(*x)){
        strcpy(xclass, "logical");
        fprintf(f, "%s%%#", prefix);
    } else if(Rf_isNumeric(*x)){
        strcpy(xclass, "numeric");
        fprintf(f, "%s{#", prefix);
    } else if(Rf_isFactor(*x)){
        strcpy(xclass, "factor");
        fprintf(f, "%s'#", prefix);
    } else if(Rf_isValidString(*x)){
        strcpy(xclass, "character");
        fprintf(f, "%s\"#", prefix);
    } else if(Rf_isFunction(*x)){
        strcpy(xclass, "function");
        fprintf(f, "%s(#", prefix);
    } else if(Rf_isFrame(*x)){
        strcpy(xclass, "data.frame");
        fprintf(f, "%s[#", prefix);
    } else if(Rf_isNewList(*x)){
        strcpy(xclass, "list");
        fprintf(f, "%s[#", prefix);
    } else {
        strcpy(xclass, "other");
        fprintf(f, "%s=#", prefix);
    }

    PROTECT(lablab = allocVector(STRSXP, 1));
    SET_STRING_ELT(lablab, 0, mkChar("label"));
    label = getAttrib(*x, lablab);
    if(length(label) > 0)
        fprintf(f, "%s\t%s\n", xname, CHAR(STRING_ELT(label, 0)));
    else
        fprintf(f, "%s\t\n", xname);
    UNPROTECT(1);

    if(strcmp(xclass, "list") == 0 || strcmp(xclass, "data.frame") == 0){
        snprintf(newenv, 500, "%s-%s", curenv, xname);
        if((vimcom_get_list_status(newenv, xclass) == 1)){
            int len = strlen(prefix);
            if(vimcom_is_utf8){
                int j = 0, i = 0;
                while(i < len){
                    if(prefix[i] == '\xe2'){
                        i += 3;
                        if(prefix[i-1] == '\x80' || prefix[i-1] == '\x94'){
                            pre[j] = ' '; j++;
                        } else {
                            pre[j] = '\xe2'; j++;
                            pre[j] = '\x94'; j++;
                            pre[j] = '\x82'; j++;
                        }
                    } else {
                        pre[j] = prefix[i];
                        i++, j++;
                    }
                }
                pre[j] = 0;
            } else {
                for(int i = 0; i < len; i++){
                    if(prefix[i] == '-' || prefix[i] == '`')
                        pre[i] = ' ';
                    else
                        pre[i] = prefix[i];
                }
                pre[len] = 0;
            }

            sprintf(newpre, "%s%s", pre, strT);
            listNames = getAttrib(*x, R_NamesSymbol);
            len = length(listNames);
            if(length(listNames) > 0){
                len -= 1;
                for(int i = 0; i < len; i++){
                    eexp = STRING_ELT(listNames, i);
                    if(Rf_isValidString(eexp)){
                        sprintf(ebuf, "[[%d]]", i);
                        ename = ebuf;
                        REprintf(">>>%s<<<\n", ename);
                    } else
                        ename = CHAR(eexp);
                    elmt = VECTOR_ELT(*x, i);
                    vimcom_browser_line(&elmt, ename, newenv, newpre, f);
                }
                sprintf(newpre, "%s%s", pre, strL);
                ename = CHAR(STRING_ELT(listNames, len));
                elmt = VECTOR_ELT(*x, len);
                vimcom_browser_line(&elmt, ename, newenv, newpre, f);
            }
        }
    }
}

static void vimcom_list_env(int checkcount)
{
    int np = 1;
    const char *varName;
    SEXP envVarsSEXP;

    if(checkcount && vimcom_count_objects() == 0)
        return;
    if(verbose > 1 && objbr_auto)
        Rprintf("New number of Objects: %d\n", nobjs);

    char fn[512];

    if(tmpdir[0] == 0)
        return;

    snprintf(fn, 510, "%s/object_browser", tmpdir);
    FILE *f = fopen(fn, "w");
    if(f == NULL){
        REprintf("Error: could not write to '%s'\n", fn);
        return;
    }

    fprintf(f, "\n");

    PROTECT(envVarsSEXP = R_lsInternal(R_GlobalEnv, allnames));
    for(int i = 0; i < Rf_length(envVarsSEXP); i++){
        varName = CHAR(STRING_ELT(envVarsSEXP, i));
        SEXP varSEXP = Rf_findVar(Rf_install(varName), R_GlobalEnv);
        if (varSEXP != R_UnboundValue) // should never be unbound 
        {
            PROTECT(varSEXP);
            np++;
            vimcom_browser_line(&varSEXP, varName, "", "   ", f);
        } else {
            REprintf("Unexpected R_UnboundValue returned from R_lsInternal");
        }
    }
    UNPROTECT(np);
    fclose(f);
    has_new_obj = 1;
}

static void vimcom_list_libs(int checkcount)
{
    int er = 0;
    int newnlibs;
    int idx, len, len1;
    const char *libname;
    char *libn;
    char prefixT[64];
    char prefixL[64];
    char fn[512];
    SEXP s, t, a, l;

    PROTECT(t = s = allocList(1));
    SET_TYPEOF(s, LANGSXP);
    SETCAR(t, install("search"));
    PROTECT(a = R_tryEval(s, R_GlobalEnv, &er));
    int np = 2;
    
    if(tmpdir[0] == 0)
        return;

    newnlibs = Rf_length(a);
    if(checkcount && nlibs == newnlibs)
        return;

    nlibs = newnlibs;

    snprintf(fn, 510, "%s/liblist", tmpdir);
    FILE *f = fopen(fn, "w");
    if(f == NULL){
        REprintf("Error: could not write to '%s'\n", fn);
        return;
    }
    fprintf(f, "\n");

    strcpy(prefixT, "   ");
    strcpy(prefixL, "   ");
    strcat(prefixT, strT);
    strcat(prefixL, strL);

    for(int i = 0; i < Rf_length(a); i++){
        l = STRING_ELT(a, i);
        PROTECT(l);
        np++;
        libname = CHAR(l);
        libn = strstr(libname, "package:");
        if(libn != NULL){
            libn += 8;
            fprintf(f, "   ##%s\t\n", libn);
            if(vimcom_get_list_status(libname, "library") == 1){
                idx = i + 1;

                SEXP x, y, oblist, obj;

                PROTECT(y = x = allocList(2));
                np++;
                SET_TYPEOF(x, LANGSXP);
                SETCAR(y, install("objects")); y = CDR(y);
                SETCAR(y, ScalarInteger(idx));
                PROTECT(oblist = R_tryEval(x, R_GlobalEnv, &er));
                np++;
                len = Rf_length(oblist);
                len1 = len - 1;
                for(int j = 0; j < len; j++){
                    PROTECT(y = x = allocList(3));
                    np++;
                    SET_TYPEOF(x, LANGSXP);
                    SETCAR(y, install("get")); y = CDR(y);
                    SETCAR(y, ScalarString(STRING_ELT(oblist, j))); y = CDR(y);
                    SETCAR(y, ScalarInteger(idx)); y = CDR(y);
                    PROTECT(obj = R_tryEval(x, R_GlobalEnv, &er));
                    np++;
                    if(j == len1)
                        vimcom_browser_line(&obj, CHAR(STRING_ELT(oblist, j)), libname, prefixL, f);
                    else
                        vimcom_browser_line(&obj, CHAR(STRING_ELT(oblist, j)), libname, prefixT, f);
                }
            }
        }
    }
    UNPROTECT(np);
    fclose(f);
    has_new_lib = 2;
}

static void vimcom_eval_expr(const char *buf, char *rep)
{
    SEXP cmdSexp, cmdexpr, ans = R_NilValue;
    ParseStatus status;
    int er = 0;

    PROTECT(cmdSexp = allocVector(STRSXP, 1));
    SET_STRING_ELT(cmdSexp, 0, mkChar(buf));
    cmdexpr = PROTECT(R_ParseVector(cmdSexp, -1, &status, R_NilValue));

    if (status != PARSE_OK) {
        sprintf(rep, "INVALID");
    } else {
        /* Loop is needed here as EXPSEXP will be of length > 1 */
        for(R_len_t i = 0; i < length(cmdexpr); i++){
            ans = R_tryEval(VECTOR_ELT(cmdexpr, i), R_GlobalEnv, &er);
            if(er){
                strcpy(rep, "ERROR");
                break;
            }
            switch(TYPEOF(ans)) {
                case REALSXP:
                    sprintf(rep, "%f", REAL(ans)[i]);
                    break;
                case LGLSXP:
                case INTSXP:
                    sprintf(rep, "%d", INTEGER(ans)[i]);
                    break;
                case STRSXP:
                    if(length(ans) > 0)
                        sprintf(rep, "%s", CHAR(STRING_ELT(ans, i)));
                    else
                        sprintf(rep, "EMPTY");
                    break;
                default:
                    sprintf(rep, "RTYPE");
            }
        }
    }
    UNPROTECT(2);
}

#ifndef WIN32
Rboolean vimcom_task(SEXP expr, SEXP value, Rboolean succeeded,
        Rboolean visible, void *userData)
{
    if(objbr_auto)
        vimcom_relist = 1;

    return(TRUE);
}

static void vimcom_ob_client(const char *msg)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    char portstr[16];
    int s, a;
    size_t len;

    /* Obtain address(es) matching host/port */

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    sprintf(portstr, "%d", obport);
    a = getaddrinfo("localhost", portstr, &hints, &result);
    if (a != 0) {
        REprintf("Error: getaddrinfo: %s\n", gai_strerror(a));
        objbr_auto = 0;
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
        REprintf("Error: Could not connect\n");
        objbr_auto = 0;
        return;
    }

    freeaddrinfo(result);	   /* No longer needed */

    len = strlen(msg);
    if (write(s, msg, len) != len) {
        REprintf("Error: partial/failed write\n");
        objbr_auto = 0;
        return;
    }
}
#endif

#ifdef WIN32
static void vimcom_server_thread(void *arg)
#else
static void *vimcom_server_thread(void *arg)
#endif
{
    unsigned short bindportn = 9998;
    ssize_t nsent;
    ssize_t nread;
    int bsize = 1024;
    char buf[bsize];
    char rep[bsize];
    int result;

#ifdef WIN32
    WSADATA wsaData;
    SOCKADDR_IN RecvAddr;
    SOCKADDR_IN peer_addr;
    int peer_addr_len = sizeof (peer_addr);

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != NO_ERROR) {
        REprintf("WSAStartup failed with error %d\n", result);
        return;
    }

    while(bindportn < 10100){
        bindportn++;
        sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sfd == INVALID_SOCKET) {
            REprintf("socket failed with error %d\n", WSAGetLastError());
            return;
        }

        RecvAddr.sin_family = AF_INET;
        RecvAddr.sin_port = htons(bindportn);
        RecvAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        if(bind(sfd, (SOCKADDR *) & RecvAddr, sizeof (RecvAddr)) == 0)
            break;
        REprintf("bind failed with error %d\n", WSAGetLastError());
    }
#else
    struct addrinfo hints;
    struct addrinfo *rp;
    struct addrinfo *res;
    struct sockaddr_storage peer_addr;
    char bindport[16];
    socklen_t peer_addr_len = sizeof(struct sockaddr_storage);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;	    /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    rp = NULL;
    result = 1;
    while(rp == NULL && bindportn < 10100){
        bindportn++;
        sprintf(bindport, "%d", bindportn);
        result = getaddrinfo(NULL, bindport, &hints, &res);
        if(result != 0){
            REprintf("getaddrinfo: %s [vimcom]\n", gai_strerror(result));
            vimcom_failure = 1;
            return(NULL);
        }

        for (rp = res; rp != NULL; rp = rp->ai_next) {
            sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (sfd == -1)
                continue;
            if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
                break;		   /* Success */
            close(sfd);
        }
        freeaddrinfo(res);	   /* No longer needed */
    }

    if (rp == NULL) {		   /* No address succeeded */
        REprintf("Could not bind. [vimcom]\n");
        vimcom_failure = 1;
        return(NULL);
    }
#endif

    if(verbose > 1)
        REprintf("vimcom port: %d\n", bindportn);

    /* Read datagrams and echo them back to sender */
    for (;;) {
        memset(buf, 0, bsize);
        memset(rep, 0, bsize);
        strcpy(rep, "UNKNOWN");

#ifdef WIN32
        nread = recvfrom(sfd, buf, bsize, 0,
                (SOCKADDR *) &peer_addr, &peer_addr_len);
        if (nread == SOCKET_ERROR) {
            REprintf("recvfrom failed with error %d\n", WSAGetLastError());
            continue;
        }
#else
        nread = recvfrom(sfd, buf, bsize, 0,
                (struct sockaddr *) &peer_addr, &peer_addr_len);
        if (nread == -1)
            continue;		   /* Ignore failed request */
#endif

        int status;
        char *bbuf;

        if(verbose > 1){
            bbuf = buf;
            if(buf[0] < 30)
                bbuf++;
            REprintf("VimCom Received: %s\n", bbuf);
        }

        switch(buf[0]){
            case 1: // Flag not used yet
                break;
            case 2: // Confirm port number
                sprintf(rep, "%s", getenv("VIMINSTANCEID"));
                if(strcmp(rep, "(null)") == 0)
                    REprintf("vimcom: the environment variable VIMINSTANCEID is not set.\n");
                strcat(rep, "&");
                strcat(rep, getenv("TMUX_PANE"));
                break;
            case 3: // Update Object Browser (.GlobalEnv)
                vimcom_list_env(0);
                break;
            case 4: // Update Object Browser (libraries)
                vimcom_list_libs(0);
                break;
            case 5: // Toggle list status
                bbuf = buf;
                bbuf++;
                vimcom_toggle_list_status(bbuf);
                if(strstr(bbuf, "package:") == bbuf){
                    vimcom_list_libs(0);
                } else {
                    vimcom_list_env(0);
                }
                strcpy(rep, "OK");
                break;
            case 6: // Close/open all lists
                bbuf = buf;
                bbuf++;
                status = atoi(bbuf);
                ListStatus *tmp = firstList;
                while(tmp){
                    if(strstr(tmp->key, "package:") != tmp->key)
                        tmp->status = status;
                    tmp = tmp->next;
                }
                vimcom_list_env(0);
                break;
            case 7: // Set obport
                bbuf = buf;
                bbuf++;
                obport = atoi(bbuf);
                objbr_auto = 1;
                sprintf(rep, "Object Browser: %d\n", obport);
                break;
            case 8: // Stop automatic update of Object Browser info
                objbr_auto = 0;
                break;
            default: // eval expression
                vimcom_eval_expr(buf, rep);
                break;
        }

        nsent = strlen(rep);
        if (sendto(sfd, rep, nsent, 0, (struct sockaddr *) &peer_addr, peer_addr_len) != nsent)
            REprintf("Error sending response. [vimcom]\n");

        if(verbose > 1)
            REprintf("VimCom Sent: %s\n", rep);
    }
#ifdef WIN32
    REprintf("vimcom: Finished receiving. Closing socket.\n");
    result = closesocket(sfd);
    if (result == SOCKET_ERROR) {
        REprintf("closesocket failed with error %d\n", WSAGetLastError());
        return;
    }
    WSACleanup();
    return;
#else
    return(NULL);
#endif
}

#ifndef WIN32
static void vimcom_R_Busy(int which)
{
    if(verbose > 3)
        REprintf("vimcom_busy: %d\n", which);
    if(which == 0 && vimcom_relist){
        vimcom_relist = 0;
        vimcom_list_env(1);
        vimcom_list_libs(1);
        switch(has_new_lib + has_new_obj){
            case 1:
                vimcom_ob_client("G");
                if(verbose > 3)
                    Rprintf("G\n");
                break;
            case 2:
                vimcom_ob_client("L");
                if(verbose > 3)
                    Rprintf("L\n");
                break;
            case 3:
                vimcom_ob_client("B");
                if(verbose > 3)
                    Rprintf("B\n");
                break;
        }
        has_new_lib = 0;
        has_new_obj = 0;
    }
}
#endif

void vimcom_Start(int *vrb, int *odf, int *ols, int *anm)
{
    verbose = *vrb;
    opendf = *odf;
    openls = *ols;
    allnames = *anm;

    if(getenv("VIMRPLUGIN_TMPDIR")){
        strncpy(tmpdir, getenv("VIMRPLUGIN_TMPDIR"), 500);
        if(verbose > 1)
            REprintf("vimcom tmpdir = %s\n", tmpdir);
    } else {
        if(verbose)
            REprintf("vimcom: environment variable VIMRPLUGIN_TMPDIR not found.\n");
        tmpdir[0] = 0;
        return;
    }

    char envstr[1024];
    envstr[0] = 0;
    if(getenv("LC_MESSAGES"))
        strcat(envstr, getenv("LC_MESSAGES"));
    if(getenv("LC_ALL"))
        strcat(envstr, getenv("LC_ALL"));
    if(getenv("LANG"))
        strcat(envstr, getenv("LANG"));
    int len = strlen(envstr);
    for(int i = 0; i < len; i++)
        envstr[i] = toupper(envstr[i]);
    if(strstr(envstr, "UTF-8") != NULL || strstr(envstr, "UTF8") != NULL){
        vimcom_is_utf8 = 1;
        strcpy(strL, "\xe2\x94\x94\xe2\x94\x80 ");
        strcpy(strT, "\xe2\x94\x9c\xe2\x94\x80 ");
    } else {
        vimcom_is_utf8 = 0;
        strcpy(strL, "`- ");
        strcpy(strT, "|- ");
    }

#ifdef WIN32
    tid = _beginthread(vimcom_server_thread, 0, NULL);
#else
    pthread_create(&tid, NULL, vimcom_server_thread, NULL);
#endif

    if(vimcom_failure == 0){
        // Linked list sentinel
        firstList = calloc(1, sizeof(ListStatus));
        firstList->key = (char*)malloc(13 * sizeof(char));
        strcpy(firstList->key, "package:base");

#ifndef WIN32
        Rf_addTaskCallback(vimcom_task, NULL, free, "VimComHandler", NULL);
        save_pt_R_Busy = ptr_R_Busy;
        ptr_R_Busy = vimcom_R_Busy;
#endif
        vimcom_initialized = 1;
        if(verbose > 0)
            REprintf("vimcom 0.9-1 loaded\n");
    }
}

void vimcom_Stop()
{
    if(vimcom_initialized){
#ifdef WIN32
        closesocket(sfd);
//	_endthread(tid);
#else
        Rf_removeTaskCallbackByName("VimComHandler");
        ptr_R_Busy = save_pt_R_Busy;
        close(sfd);
        pthread_cancel(tid);
#endif
        ListStatus *tmp = firstList;
        while(tmp){
            firstList = tmp->next;
            free(tmp->key);
            free(tmp);
            tmp = firstList;
        }
        if(verbose)
            REprintf("vimcom stopped\n");
    }
    vimcom_initialized = 0;
}

