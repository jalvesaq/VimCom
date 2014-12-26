/* The code in this file was adapted from version 1.1 of vim-remote, written
 * by Yukihiro Nakadaira based on Vim source code. The vim-remote library is
 * available at http://www.vim.org/scripts/script.php?script_id=3482 */

#ifndef NO_X_CLIENTSERVER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32
#include <windows.h>
#else
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

#include "vimthings.h"
#include "vimremote.h"

/* Use invalid encoding name to prevent conversion.
 * In Vim, conversion will fail and received data will be used as is. */
char_u p_enc[] = "RAWDATA";

#ifdef WIN32
static void serverSendEnc(HWND target);
static void CleanUpMessaging(void);
static LRESULT CALLBACK Messaging_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void serverInitMessaging(void);
static int getVimServerName(HWND hwnd, char *name, int namelen);
static BOOL CALLBACK enumWindowsGetServer(HWND hwnd, LPARAM lparam);
static HWND findServer(char_u *name);
int serverSendReply(char_u *name, char_u *reply);
int serverSendToVim(char_u *name, char_u *cmd, char_u **result);
static int save_reply(HWND server, char_u *reply, int expr);
char_u *serverGetReply(HWND server, int *expr_res, int remove, int wait);
void serverProcessPendingMessages(void);
#else
#define VIM_VERSION_SHORT "7.3"
#define MAX_PROP_WORDS 100000
typedef struct PendingCommand {
    int serial;
    int code;
    char_u *result;
    struct PendingCommand *nextPtr;
} PendingCommand;
static PendingCommand *pendingCommands = NULL;
typedef int (*EndCond) (void *);
static Window LookupName(Display *dpy, char_u *name, int delete);
static int SendInit(Display *dpy);
static int serverSendToVim(Display *dpy, char_u *name, char_u *cmd, char_u **result);
static void DeleteAnyLingerer(Display *dpy, Window w);
static int GetRegProp(Display *dpy, char_u **regPropp, long_u *numItemsp);
static void serverEventProc(Display *dpy, XEvent *eventPtr);
static int WaitForPend(void *p);
static int WindowValid(Display *dpy, Window w);
static void ServerWait(Display *dpy, Window w, EndCond endCond, void *endData, int seconds);
static int AppendPropCarefully(Display *display, Window window, Atom property, char_u *value, int length);
static int x_error_check(Display *dpy, XErrorEvent *error_event);
static Display *display = NULL;
static Window commWindow = None;
static Atom commProperty = None;
static Atom vimProperty = None;
static Atom registryProperty = None;
static int got_x_error = False;
static char_u *serverName = NULL;
#endif

typedef int (*vimremote_eval_f) (const char *expr, char **result);
static vimremote_eval_f usereval = NULL;


#ifdef WIN32
int
vimremote_init()
{
    serverInitMessaging();
    return 0;
}

int
vimremote_uninit()
{
    CleanUpMessaging();
    return 0;
}

int
vimremote_remoteexpr(const char *servername, const char *expr, char **result)
{
    return serverSendToVim((char_u *)servername, (char_u *)expr, (char_u **)result);
}


/* vim/src/os_mswin.c */
/* vi:set ts=8 sts=4 sw=4:
 *
 * VIM - Vi IMproved	by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */
/*
 * Client-server code for Vim
 *
 * Originally written by Paul Moore
 */

/* In order to handle inter-process messages, we need to have a window. But
 * the functions in this module can be called before the main GUI window is
 * created (and may also be called in the console version, where there is no
 * GUI window at all).
 *
 * So we create a hidden window, and arrange to destroy it on exit.
 */
HWND message_window = 0;	    /* window that's handling messsages */

#define VIM_CLASSNAME      "VIM_MESSAGES"
#define VIM_CLASSNAME_LEN  (sizeof(VIM_CLASSNAME) - 1)

/* Communication is via WM_COPYDATA messages. The message type is send in
 * the dwData parameter. Types are defined here. */
#define COPYDATA_KEYS		0
#define COPYDATA_REPLY		1
#define COPYDATA_EXPR		10
#define COPYDATA_RESULT		11
#define COPYDATA_ERROR_RESULT	12
#define COPYDATA_ENCODING	20

/* This is a structure containing a server HWND and its name. */
struct server_id
{
    HWND hwnd;
    char_u *name;
};

/* Last received 'encoding' that the client uses. */
static char_u	*client_enc = NULL;

/*
 * Tell the other side what encoding we are using.
 * Errors are ignored.
 */
    static void
serverSendEnc(HWND target)
{
    COPYDATASTRUCT data;

    data.dwData = COPYDATA_ENCODING;
    data.cbData = (DWORD)STRLEN(p_enc) + 1;
    data.lpData = p_enc;
    (void)SendMessage(target, WM_COPYDATA, (WPARAM)message_window,
							     (LPARAM)(&data));
}

/*
 * Clean up on exit. This destroys the hidden message window.
 */
    static void
CleanUpMessaging(void)
{
    if (message_window != 0)
    {
	DestroyWindow(message_window);
	message_window = 0;
    }
}

static int save_reply(HWND server, char_u *reply, int expr);

/*s
 * The window procedure for the hidden message window.
 * It handles callback messages and notifications from servers.
 * In order to process these messages, it is necessary to run a
 * message loop. Code which may run before the main message loop
 * is started (in the GUI) is careful to pump messages when it needs
 * to. Features which require message delivery during normal use will
 * not work in the console version - this basically means those
 * features which allow Vim to act as a server, rather than a client.
 */
    static LRESULT CALLBACK
Messaging_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int err;

    if (msg == WM_COPYDATA)
    {
	/* This is a message from another Vim. The dwData member of the
	 * COPYDATASTRUCT determines the type of message:
	 *   COPYDATA_ENCODING:
	 *	The encoding that the client uses. Following messages will
	 *	use this encoding, convert if needed.
	 *   COPYDATA_KEYS:
	 *	A key sequence. We are a server, and a client wants these keys
	 *	adding to the input queue.
	 *   COPYDATA_REPLY:
	 *	A reply. We are a client, and a server has sent this message
	 *	in response to a request.  (server2client())
	 *   COPYDATA_EXPR:
	 *	An expression. We are a server, and a client wants us to
	 *	evaluate this expression.
	 *   COPYDATA_RESULT:
	 *	A reply. We are a client, and a server has sent this message
	 *	in response to a COPYDATA_EXPR.
	 *   COPYDATA_ERROR_RESULT:
	 *	A reply. We are a client, and a server has sent this message
	 *	in response to a COPYDATA_EXPR that failed to evaluate.
	 */
	COPYDATASTRUCT	*data = (COPYDATASTRUCT*)lParam;
	HWND		sender = (HWND)wParam;
	COPYDATASTRUCT	reply;
	char_u		*res;
	int		retval;
	char_u		*str;

	switch (data->dwData)
	{
	case COPYDATA_ENCODING:
	    /* Remember the encoding that the client uses. */
	    free(client_enc);
            // TODO:
	    //client_enc = enc_canonize((char_u *)data->lpData);
	    client_enc = vim_strsave(data->lpData);
	    return 1;

	case COPYDATA_KEYS:
            /* not supported */
	    return 1;

	case COPYDATA_EXPR:
	    str = (char_u *)data->lpData;
            if (usereval == NULL) {
                err = -1;
                res = NULL;
            } else {
                res = NULL;
                err = usereval((char *)str, (char **)&res);
            }

            if (!err)
		reply.dwData = COPYDATA_RESULT;
            else
		reply.dwData = COPYDATA_ERROR_RESULT;
	    reply.lpData = (res == NULL) ? (char_u *)"" : res;
	    reply.cbData = (res == NULL) ? 1 : (DWORD)STRLEN(res) + 1;

	    serverSendEnc(sender);
	    retval = (int)SendMessage(sender, WM_COPYDATA, (WPARAM)message_window,
							    (LPARAM)(&reply));
	    free(res);
	    return retval;

	case COPYDATA_REPLY:
	case COPYDATA_RESULT:
	case COPYDATA_ERROR_RESULT:
	    if (data->lpData != NULL)
	    {
		str = (char_u *)data->lpData;
                str = vim_strsave(str);
		if (save_reply(sender, str,
			   (data->dwData == COPYDATA_REPLY ?  0 :
			   (data->dwData == COPYDATA_RESULT ? 1 :
							      2))) == FAIL)
		    free(str);
	    }
	    return 1;
	}

	return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/*
 * Initialise the message handling process.  This involves creating a window
 * to handle messages - the window will not be visible.
 */
    void
serverInitMessaging(void)
{
    WNDCLASS wndclass;
    HINSTANCE s_hinst;

    /* Clean up on exit */
    atexit(CleanUpMessaging);

    /* Register a window class - we only really care
     * about the window procedure
     */
    s_hinst = (HINSTANCE)GetModuleHandle(0);
    wndclass.style = 0;
    wndclass.lpfnWndProc = Messaging_WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = s_hinst;
    wndclass.hIcon = NULL;
    wndclass.hCursor = NULL;
    wndclass.hbrBackground = NULL;
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = VIM_CLASSNAME;
    RegisterClass(&wndclass);

    /* Create the message window. It will be hidden, so the details don't
     * matter.  Don't use WS_OVERLAPPEDWINDOW, it will make a shortcut remove
     * focus from gvim. */
    message_window = CreateWindow(VIM_CLASSNAME, "",
			 WS_POPUPWINDOW | WS_CAPTION,
			 CW_USEDEFAULT, CW_USEDEFAULT,
			 100, 100, NULL, NULL,
			 s_hinst, NULL);
}

/*
 * Get the title of the window "hwnd", which is the Vim server name, in
 * "name[namelen]" and return the length.
 * Returns zero if window "hwnd" is not a Vim server.
 */
    static int
getVimServerName(HWND hwnd, char *name, int namelen)
{
    int		len;
    char	buffer[VIM_CLASSNAME_LEN + 1];

    /* Ignore windows which aren't Vim message windows */
    len = GetClassName(hwnd, buffer, sizeof(buffer));
    if (len != VIM_CLASSNAME_LEN || STRCMP(buffer, VIM_CLASSNAME) != 0)
	return 0;

    /* Get the title of the window */
    return GetWindowText(hwnd, name, namelen);
}

    static BOOL CALLBACK
enumWindowsGetServer(HWND hwnd, LPARAM lparam)
{
    struct	server_id *id = (struct server_id *)lparam;
    char	server[MAX_PATH];

    /* Get the title of the window */
    if (getVimServerName(hwnd, server, sizeof(server)) == 0)
	return TRUE;

    /* If this is the server we're looking for, return its HWND */
    if (STRICMP(server, id->name) == 0)
    {
	id->hwnd = hwnd;
	return FALSE;
    }

    /* Otherwise, keep looking */
    return TRUE;
}

    static HWND
findServer(char_u *name)
{
    struct server_id id;

    id.name = name;
    id.hwnd = 0;

    EnumWindows(enumWindowsGetServer, (LPARAM)(&id));

    return id.hwnd;
}


    int
serverSendReply(name, reply)
    char_u	*name;		/* Where to send. */
    char_u	*reply;		/* What to send. */
{
    HWND	target;
    COPYDATASTRUCT data;
    long_u	n = 0;

    /* The "name" argument is a magic cookie obtained from expand("<client>").
     * It should be of the form 0xXXXXX - i.e. a C hex literal, which is the
     * value of the client's message window HWND.
     */
    sscanf((char *)name, SCANF_HEX_LONG_U, &n);
    if (n == 0)
	return -1;

    target = (HWND)n;
    if (!IsWindow(target))
	return -1;

    data.dwData = COPYDATA_REPLY;
    data.cbData = (DWORD)STRLEN(reply) + 1;
    data.lpData = reply;

    serverSendEnc(target);
    if (SendMessage(target, WM_COPYDATA, (WPARAM)message_window,
							     (LPARAM)(&data)))
	return 0;

    return -1;
}

    int
serverSendToVim(name, cmd, result)
    char_u	 *name;			/* Where to send. */
    char_u	 *cmd;			/* What to send. */
    char_u	 **result;		/* Result of eval'ed expression */
{
    HWND	target;
    COPYDATASTRUCT data;
    char_u	*retval = NULL;
    int		retcode = 0;

    target = findServer(name);

    if (target == 0)
    {
	return -1;
    }

    data.dwData = COPYDATA_EXPR;
    data.cbData = (DWORD)STRLEN(cmd) + 1;
    data.lpData = cmd;

    serverSendEnc(target);
    if (SendMessage(target, WM_COPYDATA, (WPARAM)message_window,
							(LPARAM)(&data)) == 0)
	return -1;

    retval = serverGetReply(target, &retcode, TRUE, TRUE);

    if (result == NULL)
	free(retval);
    else
	*result = retval; /* Caller assumes responsibility for freeing */

    return retcode;
}

/* Replies from server need to be stored until the client picks them up via
 * remote_read(). So we maintain a list of server-id/reply pairs.
 * Note that there could be multiple replies from one server pending if the
 * client is slow picking them up.
 * We just store the replies in a simple list. When we remove an entry, we
 * move list entries down to fill the gap.
 * The server ID is simply the HWND.
 */
typedef struct
{
    HWND	server;		/* server window */
    char_u	*reply;		/* reply string */
    int		expr_result;	/* 0 for REPLY, 1 for RESULT 2 for error */
} reply_T;

static garray_T reply_list = {0, 0, sizeof(reply_T), 5, 0};

#define REPLY_ITEM(i) ((reply_T *)(reply_list.ga_data) + (i))
#define REPLY_COUNT (reply_list.ga_len)

/* Flag which is used to wait for a reply */
static int reply_received = 0;

/*
 * Store a reply.  "reply" must be allocated memory (or NULL).
 */
    static int
save_reply(HWND server, char_u *reply, int expr)
{
    reply_T *rep;

    if (ga_grow(&reply_list, 1) == FAIL)
	return FAIL;

    rep = REPLY_ITEM(REPLY_COUNT);
    rep->server = server;
    rep->reply = reply;
    rep->expr_result = expr;
    if (rep->reply == NULL)
	return FAIL;

    ++REPLY_COUNT;
    reply_received = 1;
    return OK;
}

/*
 * Get a reply from server "server".
 * When "expr_res" is non NULL, get the result of an expression, otherwise a
 * server2client() message.
 * When non NULL, point to return code. 0 => OK, -1 => ERROR
 * If "remove" is TRUE, consume the message, the caller must free it then.
 * if "wait" is TRUE block until a message arrives (or the server exits).
 */
    char_u *
serverGetReply(HWND server, int *expr_res, int remove, int wait)
{
    int		i;
    char_u	*reply;
    reply_T	*rep;

    /* When waiting, loop until the message waiting for is received. */
    for (;;)
    {
	/* Reset this here, in case a message arrives while we are going
	 * through the already received messages. */
	reply_received = 0;

	for (i = 0; i < REPLY_COUNT; ++i)
	{
	    rep = REPLY_ITEM(i);
	    if (rep->server == server
		    && ((rep->expr_result != 0) == (expr_res != NULL)))
	    {
		/* Save the values we've found for later */
		reply = rep->reply;
		if (expr_res != NULL)
		    *expr_res = rep->expr_result == 1 ? 0 : -1;

		if (remove)
		{
		    /* Move the rest of the list down to fill the gap */
		    mch_memmove(rep, rep + 1,
				     (REPLY_COUNT - i - 1) * sizeof(reply_T));
		    --REPLY_COUNT;
		}

		/* Return the reply to the caller, who takes on responsibility
		 * for freeing it if "remove" is TRUE. */
		return reply;
	    }
	}

	/* If we got here, we didn't find a reply. Return immediately if the
	 * "wait" parameter isn't set.  */
	if (!wait)
	    break;

	/* We need to wait for a reply. Enter a message loop until the
	 * "reply_received" flag gets set. */

	/* Loop until we receive a reply */
	while (reply_received == 0)
	{
	    /* Wait for a SendMessage() call to us.  This could be the reply
	     * we are waiting for.  Use a timeout of a second, to catch the
	     * situation that the server died unexpectedly. */
	    MsgWaitForMultipleObjects(0, NULL, TRUE, 1000, QS_ALLINPUT);

	    /* If the server has died, give up */
	    if (!IsWindow(server))
		return NULL;

	    serverProcessPendingMessages();
	}
    }

    return NULL;
}

/*
 * Process any messages in the Windows message queue.
 */
    void
serverProcessPendingMessages(void)
{
    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }
}

#else

int
vimremote_init()
{
    if (display != NULL) {
        return 0;
    }

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        return -1;
    }

    if (SendInit(display) != 0) {
        return -1;
    }

    return 0;
}

int
vimremote_uninit()
{
    if (display == NULL) {
        return 0;
    }

    if (commWindow != None) {
        XDestroyWindow(display, commWindow);
        commWindow = None;
    }

    XCloseDisplay(display);
    display = NULL;

    return 0;
}

int
vimremote_remoteexpr(const char *servername, const char *expr, char **result)
{
    return serverSendToVim(display, (char_u *)servername, (char_u *)expr, (char_u **)result);
}


/* vim/src/if_xcmdsrv.c */
/* vi:set ts=8 sts=4 sw=4:
 *
 * VIM - Vi IMproved	by Bram Moolenaar
 * X command server by Flemming Madsen
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 *
 * if_xcmdsrv.c: Functions for passing commands through an X11 display.
 *
 */
/*
 * This file provides procedures that implement the command server
 * functionality of Vim when in contact with an X11 server.
 *
 * Adapted from TCL/TK's send command  in tkSend.c of the tk 3.6 distribution.
 * Adapted for use in Vim by Flemming Madsen. Protocol changed to that of tk 4
 */

/*
 * Copyright (c) 1989-1993 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

static Window
LookupName(Display *dpy, char_u *name, int delete)
{
    char_u	*regProp, *entry;
    char_u	*p;
    long_u	numItems;
    int_u	returnValue;

    /*
     * Read the registry property.
     */
    if (!GetRegProp(dpy, &regProp, &numItems))
	return 0;

    if (regProp == NULL) {
        return None;
    }

    /*
     * Scan the property for the desired name.
     */
    returnValue = (int_u)None;
    entry = NULL;	/* Not needed, but eliminates compiler warning. */
    for (p = regProp; (long_u)(p - regProp) < numItems; )
    {
	entry = p;
	while (*p != 0 && !isspace(*p))
	    p++;
	if (*p != 0 && STRICMP(name, p + 1) == 0)
	{
	    sscanf((char *)entry, "%x", &returnValue);
	    break;
	}
	while (*p != 0)
	    p++;
	p++;
    }

    /*
     * Delete the property, if that is desired (copy down the
     * remainder of the registry property to overlay the deleted
     * info, then rewrite the property).
     */
    if (delete && returnValue != (int_u)None)
    {
	int count;

	while (*p != 0)
	    p++;
	p++;
	count = numItems - (p - regProp);
	if (count > 0)
	    memmove(entry, p, count);
	XChangeProperty(dpy, RootWindow(dpy, 0), registryProperty, XA_STRING,
			8, PropModeReplace, regProp,
			(int)(numItems - (p - entry)));
	XSync(dpy, False);
    }

    XFree(regProp);

    return (Window)returnValue;
}

static int
SendInit(Display *dpy)
{
    XErrorHandler old_handler;

    /*
     * Create the window used for communication, and set up an
     * event handler for it.
     */
    old_handler = XSetErrorHandler(x_error_check);
    got_x_error = False;

    if (commProperty == None)
	commProperty = XInternAtom(dpy, "Comm", False);
    if (vimProperty == None)
	vimProperty = XInternAtom(dpy, "Vim", False);
    if (registryProperty == None)
	registryProperty = XInternAtom(dpy, "VimRegistry", False);

    if (commWindow == None)
    {
	commWindow = XCreateSimpleWindow(dpy, XDefaultRootWindow(dpy),
				getpid(), 0, 10, 10, 0,
				WhitePixel(dpy, DefaultScreen(dpy)),
				WhitePixel(dpy, DefaultScreen(dpy)));
	XSelectInput(dpy, commWindow, PropertyChangeMask);
	/* WARNING: Do not step through this while debugging, it will hangup
	 * the X server! */
	XGrabServer(dpy);
	DeleteAnyLingerer(dpy, commWindow);
	XUngrabServer(dpy);
    }

    /* Make window recognizable as a vim window */
    XChangeProperty(dpy, commWindow, vimProperty, XA_STRING,
		    8, PropModeReplace, (char_u *)VIM_VERSION_SHORT,
			(int)STRLEN(VIM_VERSION_SHORT) + 1);

    XSync(dpy, False);
    (void)XSetErrorHandler(old_handler);

    return got_x_error ? -1 : 0;
}

static int
serverSendToVim(Display *dpy, char_u *name, char_u *cmd, char_u **result)
{
    Window	    w;
    char_u	    *property;
    int		    length;
    int		    res;
    static int	    serial = 0;	/* Running count of sent commands.
				 * Used to give each command a
				 * different serial number. */
    PendingCommand  pending;

    w = LookupName(dpy, name, False);
    if (w == None) {
        return -1;
    }

    /*
     * Send the command to target interpreter by appending it to the
     * comm window in the communication window.
     * Length must be computed exactly!
     */
    length = STRLEN(name) + STRLEN(p_enc) + STRLEN(cmd) + 14;
    property = (char_u *)malloc((unsigned)length + 30);

    sprintf((char *)property, "%c%c%c-n %s%c-E %s%c-s %s",
		      0, 'c', 0, name, 0, p_enc, 0, cmd);
    /* Add a back reference to our comm window */
    serial++;
    sprintf((char *)property + length, "%c-r %x %d",
						0, (int_u)commWindow, serial);
    /* Add length of what "-r %x %d" resulted in, skipping the NUL. */
    length += STRLEN(property + length + 1) + 1;

    res = AppendPropCarefully(dpy, w, commProperty, property, length + 1);
    free(property);
    if (res < 0)
    {
	return -1;
    }

    /*
     * Register the fact that we're waiting for a command to
     * complete (this is needed by SendEventProc and by
     * AppendErrorProc to pass back the command's results).
     */
    pending.serial = serial;
    pending.code = 0;
    pending.result = NULL;
    pending.nextPtr = pendingCommands;
    pendingCommands = &pending;

    ServerWait(dpy, w, WaitForPend, &pending, 600);

    /*
     * Unregister the information about the pending command
     * and return the result.
     */
    if (pendingCommands == &pending)
	pendingCommands = pending.nextPtr;
    else
    {
	PendingCommand *pcPtr;

	for (pcPtr = pendingCommands; pcPtr != NULL; pcPtr = pcPtr->nextPtr)
	    if (pcPtr->nextPtr == &pending)
	    {
		pcPtr->nextPtr = pending.nextPtr;
		break;
	    }
    }
    if (result != NULL)
	*result = pending.result;
    else
	free(pending.result);

    return pending.code == 0 ? 0 : -1;
}


static void
DeleteAnyLingerer(Display *dpy, Window win)
{
    char_u	*regProp, *entry = NULL;
    char_u	*p;
    long_u	numItems;
    int_u	wwin;

    /*
     * Read the registry property.
     */
    if (!GetRegProp(dpy, &regProp, &numItems))
	return;

    if (regProp == NULL)
        return;

    /* Scan the property for the window id.  */
    for (p = regProp; (long_u)(p - regProp) < numItems; )
    {
	if (*p != 0)
	{
	    sscanf((char *)p, "%x", &wwin);
	    if ((Window)wwin == win)
	    {
		int lastHalf;

		/* Copy down the remainder to delete entry */
		entry = p;
		while (*p != 0)
		    p++;
		p++;
		lastHalf = numItems - (p - regProp);
		if (lastHalf > 0)
		    memmove(entry, p, lastHalf);
		numItems = (entry - regProp) + lastHalf;
		p = entry;
		continue;
	    }
	}
	while (*p != 0)
	    p++;
	p++;
    }

    if (entry != NULL)
    {
	XChangeProperty(dpy, RootWindow(dpy, 0), registryProperty,
			XA_STRING, 8, PropModeReplace, regProp,
			(int)(p - regProp));
	XSync(dpy, False);
    }

    XFree(regProp);
}

static int
GetRegProp(Display *dpy, char_u **regPropp, long_u *numItemsp)
{
    int result;
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes_after;
    unsigned char *prop;
    XErrorHandler old_handler;

    got_x_error = False;
    old_handler = XSetErrorHandler(x_error_check);
    result = XGetWindowProperty(dpy, RootWindow(dpy, 0), registryProperty,
            0L, (long)MAX_PROP_WORDS, False,
            XA_STRING, &actual_type, &actual_format,
            &nitems, &bytes_after, &prop);
    XSync(dpy, False);
    XSetErrorHandler(old_handler);

    if (got_x_error) {
        return False;
    }

    if (actual_type == None) {
        /* No prop yet. Logically equal to the empty list */
        *numItemsp = 0;
        *regPropp = NULL;
        return True;
    }

    /* If the property is improperly formed, then delete it. */
    if (result != Success || actual_format != 8 || actual_type != XA_STRING) {
        if (prop != NULL) {
            XFree(prop);
        }
        XDeleteProperty(dpy, RootWindow(dpy, 0), registryProperty);
        return False;
    }

    *numItemsp = nitems;
    *regPropp = prop;

    return True;
}

void
serverEventProc(Display *dpy, XEvent *eventPtr)
{
    char_u	*propInfo;
    char_u	*p;
    int		result, actualFormat, code;
    long_u	numItems, bytesAfter;
    Atom	actualType;

    if (eventPtr != NULL)
    {
	if (eventPtr->xproperty.atom != commProperty
		|| eventPtr->xproperty.state != PropertyNewValue)
	    return;
    }

    /*
     * Read the comm property and delete it.
     */
    propInfo = NULL;
    result = XGetWindowProperty(dpy, commWindow, commProperty, 0L,
				(long)MAX_PROP_WORDS, True,
				XA_STRING, &actualType,
				&actualFormat, &numItems, &bytesAfter,
				&propInfo);

    /* If the property doesn't exist or is improperly formed then ignore it. */
    if (result != Success || actualType != XA_STRING || actualFormat != 8)
    {
	if (propInfo != NULL)
	    XFree(propInfo);
	return;
    }

    /*
     * Several commands and results could arrive in the property at
     * one time;  each iteration through the outer loop handles a
     * single command or result.
     */
    for (p = propInfo; (long_u)(p - propInfo) < numItems; )
    {
	/*
	 * Ignore leading NULs; each command or result starts with a
	 * NUL so that no matter how badly formed a preceding command
	 * is, we'll be able to tell that a new command/result is
	 * starting.
	 */
	if (*p == 0)
	{
	    p++;
	    continue;
	}

	if ((*p == 'c' || *p == 'k') && (p[1] == 0))
	{
	    Window	resWindow;
	    char_u	*name, *script, *serial, *end, *res;
	    Bool	asKeys = *p == 'k';
	    garray_T	reply;
            int err;

	    /*
	     * This is an incoming command from some other application.
	     * Iterate over all of its options.  Stop when we reach
	     * the end of the property or something that doesn't look
	     * like an option.
	     */
	    p += 2;
	    name = NULL;
	    resWindow = None;
	    serial = (char_u *)"";
	    script = NULL;
	    while ((long_u)(p - propInfo) < numItems && *p == '-')
	    {
		switch (p[1])
		{
		    case 'r':
			end = skipwhite(p + 2);
			resWindow = 0;
			while (vim_isxdigit(*end))
			{
			    resWindow = 16 * resWindow + (long_u)hex2nr(*end);
			    ++end;
			}
			if (end == p + 2 || *end != ' ')
			    resWindow = None;
			else
			{
			    p = serial = end + 1;
			}
			break;
		    case 'n':
			if (p[2] == ' ')
			    name = p + 3;
			break;
		    case 's':
			if (p[2] == ' ')
			    script = p + 3;
			break;
		}
		while (*p != 0)
		    p++;
		p++;
	    }

	    if (script == NULL || name == NULL)
		continue;

            /* remote_send() is not supported */
            if (asKeys)
                continue;

	    /*
	     * Initialize the result property, so that we're ready at any
	     * time if we need to return an error.
	     */
	    if (resWindow != None)
	    {
		ga_init2(&reply, 1, 100);
		ga_grow(&reply, 50 + STRLEN(p_enc));
		sprintf(reply.ga_data, "%cr%c-E %s%c-s %s%c-r ",
						   0, 0, p_enc, 0, serial, 0);
		reply.ga_len = 14 + STRLEN(p_enc) + STRLEN(serial);
	    }
            err = 0;
	    res = NULL;
	    if (serverName != NULL && STRICMP(name, serverName) == 0) {
                if (usereval == NULL) {
                    err = -1;
                    res = NULL;
                } else {
                    err = usereval((char *)script, (char **)&res);
                }
	    }
	    if (resWindow != None) {
                if (!err) {
		    ga_concat(&reply, (res == NULL ? (char_u *)"" : res));
                } else {
		    ga_concat(&reply, (res == NULL ? (char_u *)"" : res));
		    ga_append(&reply, 0);
		    ga_concat(&reply, (char_u *)"-c 1");
		}
		ga_append(&reply, NUL);
		(void)AppendPropCarefully(dpy, resWindow, commProperty,
					   reply.ga_data, reply.ga_len);
		ga_clear(&reply);
	    }
	    free(res);
	}
	else if (*p == 'r' && p[1] == 0)
	{
	    int		    serial, gotSerial;
	    char_u	    *res;
	    PendingCommand  *pcPtr;

	    /*
	     * This is a reply to some command that we sent out.  Iterate
	     * over all of its options.  Stop when we reach the end of the
	     * property or something that doesn't look like an option.
	     */
	    p += 2;
	    gotSerial = 0;
	    res = (char_u *)"";
	    code = 0;
	    while ((long_u)(p - propInfo) < numItems && *p == '-')
	    {
		switch (p[1])
		{
		    case 'r':
			if (p[2] == ' ')
			    res = p + 3;
			break;
		    case 's':
			if (sscanf((char *)p + 2, " %d", &serial) == 1)
			    gotSerial = 1;
			break;
		    case 'c':
			if (sscanf((char *)p + 2, " %d", &code) != 1)
			    code = 0;
			break;
		}
		while (*p != 0)
		    p++;
		p++;
	    }

	    if (!gotSerial)
		continue;

	    /*
	     * Give the result information to anyone who's
	     * waiting for it.
	     */
	    for (pcPtr = pendingCommands; pcPtr != NULL; pcPtr = pcPtr->nextPtr)
	    {
		if (serial != pcPtr->serial || pcPtr->result != NULL)
		    continue;

		pcPtr->code = code;
		if (res != NULL) {
		    pcPtr->result = vim_strsave(res);
		} else
		    pcPtr->result = vim_strsave((char_u *)"");
		break;
	    }
	}
	else if (*p == 'n' && p[1] == 0)
	{
	    unsigned int u;
	    int		gotWindow;

	    /*
	     * This is a (n)otification.  Sent with serverreply_send in VimL.
	     * Execute any autocommand and save it for later retrieval
	     */
	    p += 2;
	    gotWindow = 0;
	    while ((long_u)(p - propInfo) < numItems && *p == '-')
	    {
		switch (p[1])
		{
		    case 'w':
			if (sscanf((char *)p + 2, " %x", &u) == 1)
			{
			    gotWindow = 1;
			}
			break;
		}
		while (*p != 0)
		    p++;
		p++;
	    }

	    if (!gotWindow)
		continue;

            /* not supported */
	}
	else
	{
	    /*
	     * Didn't recognize this thing.  Just skip through the next
	     * null character and try again.
	     * Even if we get an 'r'(eply) we will throw it away as we
	     * never specify (and thus expect) one
	     */
	    while (*p != 0)
		p++;
	    p++;
	}
    }
    XFree(propInfo);
}

static int
WaitForPend(void *p)
{
    PendingCommand *pending = (PendingCommand *) p;
    return pending->result != NULL;
}

static int
WindowValid(Display *dpy, Window w)
{
    Atom *plist;
    int num_prop;
    int i;
    XErrorHandler old_handler;

    got_x_error = False;
    old_handler = XSetErrorHandler(x_error_check);
    plist = XListProperties(dpy, w, &num_prop);
    XSync(dpy, False);
    XSetErrorHandler(old_handler);

    if (plist == NULL || got_x_error) {
        return False;
    }

    for (i = 0; i < num_prop; ++i) {
        if (plist[i] == vimProperty) {
            XFree(plist);
            return True;
        }
    }
    XFree(plist);
    return False;
}

static void
ServerWait(Display *dpy, Window w, EndCond endCond, void *endData, int seconds)
{
    time_t	    start;
    time_t	    now;
    XEvent	    event;
    XPropertyEvent *e = (XPropertyEvent *)&event;
#   define SEND_MSEC_POLL 50

    time(&start);
    while (endCond(endData) == 0)
    {
	time(&now);
	if (seconds >= 0 && (now - start) >= seconds)
	    break;
        if (!WindowValid(dpy, w))
            break;
        while (XEventsQueued(dpy, QueuedAfterReading) > 0)
        {
            XNextEvent(dpy, &event);
            if (event.type == PropertyNotify && e->window == commWindow)
                serverEventProc(dpy, &event);
        }
        usleep(SEND_MSEC_POLL);
    }
}

static int
AppendPropCarefully(Display *dpy, Window window, Atom property, char_u *value, int length)
{
    XErrorHandler old_handler;

    old_handler = XSetErrorHandler(x_error_check);
    got_x_error = False;
    XChangeProperty(dpy, window, property, XA_STRING, 8,
            PropModeAppend, value, length);
    XSync(dpy, False);
    XSetErrorHandler(old_handler);
    return got_x_error ? -1 : 0;
}

static int
x_error_check(Display * UNUSED(dpy), XErrorEvent * UNUSED(error_event))
{
    got_x_error = True;
    return 0;
}

#endif
#endif

