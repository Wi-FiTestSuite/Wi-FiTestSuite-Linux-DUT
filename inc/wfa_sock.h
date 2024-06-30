/****************************************************************************
*
* Copyright (c) 2016 Wi-Fi Alliance
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
* SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
* RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
* USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/


#ifndef _WFA_SOCK_H
#define _WFA_SOCK_H

#include <stdio.h>		/* for printf() and fprintf() */

#ifdef _WINDOWS
#include <winsock.h>    /* for socket(), bind(), and connect() */
#else
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>		/* for atoi() and exit() */
#include <string.h>		/* for memset() */
#include <unistd.h>		/* for close() */
#include <sys/time.h>   /* for struct timeval {} */
#endif
#include <stdlib.h>		/* for atoi() and exit() */
#include <string.h>		/* for memset() */
#include <fcntl.h>		/* for fcntl() */
#include <errno.h>

#define MAX_UDP_LEN       1470
#define MAX_RCV_BUF_LEN   (32*1024)

struct sockfds
{
    int *agtfd;      /* dut agent main socket fd */
    int *cafd;       /* sock fd to control agent */
    int *tgfd;       /* traffic agent fd         */
    int *wmmfds;     /* wmm stream ids           */
    int *psfd;       /* wmm-ps socket id         */
};

extern int wfaCreateTCPServSock(unsigned short sport);
extern int wfaCreateUDPSock(char *sipaddr, unsigned short sport);
extern int wfaAcceptTCPConn(int servSock);
extern int wfaConnectUDPPeer(int sock, char *dipaddr, int dport);
extern void wfaSetSockFiDesc(fd_set *sockset, int *, struct sockfds *);
#ifdef _WINDOWS
extern int wfaCtrlSend(SOCKET sock, unsigned char *buf, int bufLen);
#else
extern int wfaCtrlSend(int sock, unsigned char *buf, int bufLen);
#endif
extern int wfaCtrlRecv(int sock, unsigned char *buf);
extern int wfaTrafficSendTo(int sock, char *buf, int bufLen, struct sockaddr *to);
extern int wfaTrafficRecv(int sock, char *buf, struct sockaddr *from);
extern int wfaGetifAddr(char *ifname, struct sockaddr_in *sa);
extern struct timeval *wfaSetTimer(int, int, struct timeval *);
extern int wfaSetSockMcastRecvOpt(int, char*);
extern int wfaSetSockMcastSendOpt(int);
extern int wfaSetProcPriority(int);

#endif /* _WFA_SOCK_H */
