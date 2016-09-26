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

/*
 *    File: wfa_sock.c
 *    library functions for TCP and UDP socket creations and handling
 *    They are common library and shared by DUT, TC and CA.
 */

#if 0
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sched.h>
#endif

#include "wfa_portall.h"
#include "wfa_stdincs.h"
#include "wfa_debug.h"
#include "wfa_types.h"
#include "wfa_main.h"
#include "wfa_sock.h"

int wfaGetifAddr(char *ifname, struct sockaddr_in *sa);

extern unsigned short wfa_defined_debug;

extern BOOL gtgTransac;
extern char gnetIf[];

#define MAXPENDING 2    /* Maximum outstanding connection requests */

/*
 * wfaCreateTCPServSock(): initially create a TCP socket
 * intput:   port -- TCP socket port to listen
 * return:   socket id;
 */
int wfaCreateTCPServSock(unsigned short port)
{
    int sock;                        /* socket to create */
    struct sockaddr_in servAddr; /* Local address */
    const int on = 1;

    /* Create socket for incoming connections */
    if ((sock = wSOCKET(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        DPRINT_ERR(WFA_ERR, "createTCPServSock socket() failed");
        return WFA_FAILURE;
    }

    /* Construct local address structure */
    wMEMSET(&servAddr, 0, sizeof(servAddr));
    wfaGetifAddr(gnetIf, &servAddr);
    servAddr.sin_family = AF_INET;        /* Internet address family */
    servAddr.sin_port = htons(port);              /* Local port */

    wSETSOCKOPT(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    /* Bind to the local address */
    if (wBIND(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
    {
        DPRINT_ERR(WFA_ERR, "bind() failed");
        return WFA_FAILURE;
    }

    /* Mark the socket so it will listen for incoming connections */
    if (wLISTEN(sock, MAXPENDING) < 0)
    {
        DPRINT_ERR(WFA_ERR, "listen() failed");
        return WFA_FAILURE;
    }

    return sock;
}

/*
 * wfaCreateUDPSock(): create a UDP socket
 * input:
       ipaddr -- local ip address for test traffic
       port -- UDP port to receive and send
 * return:    socket id
 */
int wfaCreateUDPSock(char *ipaddr, unsigned short port)
{
    int udpsock;                        /* socket to create */
    struct sockaddr_in servAddr; /* Local address */

    if((udpsock = wSOCKET(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        DPRINT_ERR(WFA_ERR, "createUDPSock socket() failed");
        return WFA_FAILURE;
    }

    wBZERO(&servAddr, sizeof(servAddr));
    servAddr.sin_family      = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port        = htons(port);

    wBIND(udpsock, (struct sockaddr *) &servAddr, sizeof(servAddr));

    return udpsock;
}

int wfaSetSockMcastSendOpt(int sockfd)
{
    unsigned char ttlval = 1;

    return wSETSOCKOPT(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttlval, sizeof(ttlval));
}

int wfaSetSockMcastRecvOpt(int sockfd, char *mcastgroup)
{
    struct ip_mreq mcreq;
    int so;

    mcreq.imr_multiaddr.s_addr = inet_addr(mcastgroup);
    mcreq.imr_interface.s_addr = htonl(INADDR_ANY);
    so = wSETSOCKOPT(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                     (void *)&mcreq, sizeof(mcreq));

    return so;
}

int wfaConnectUDPPeer(int mysock, char *daddr, int dport)
{
    struct sockaddr_in peerAddr;

    wMEMSET(&peerAddr, 0, sizeof(peerAddr));
    peerAddr.sin_family = AF_INET;
    inet_aton(daddr, &peerAddr.sin_addr);
    peerAddr.sin_port   = htons(dport);

    wCONNECT(mysock, (struct sockaddr *)&peerAddr, sizeof(peerAddr));
    return mysock;
}

/*
 * acceptTCPConn(): handle and accept any incoming socket connection request.
 * input:        serSock -- the socket id to listen
 * return:       the connected socket id.
 */
int wfaAcceptTCPConn(int servSock)
{
    int clntSock;                /* Socket descriptor for client */
    struct sockaddr_in clntAddr; /* Client address */
    unsigned int clntLen;        /* Length of client address data structure */

    /* Set the size of the in-out parameter */
    clntLen = sizeof(clntAddr);

    /* Wait for a client to connect */
    if ((clntSock = wACCEPT(servSock, (struct sockaddr *) &clntAddr,
                            &clntLen)) < 0)
    {
        DPRINT_ERR(WFA_ERR, "accept() failed");
        exit(1);
    }

    /* clntSock is connected to a client! */
    return clntSock;
}

struct timeval *wfaSetTimer(int secs, int usecs, struct timeval *tv)
{
    struct timeval *mytv;

    if(gtgTransac != 0)
    {
        tv->tv_sec = secs ;             /* timeout (secs.) */
        tv->tv_usec = usecs;            /* 0 microseconds */
    }
    else
    {
        tv->tv_sec =  0;
        tv->tv_usec = 0;                /* 0 microseconds */
    }

    if(tv->tv_sec == 0 && tv->tv_usec == 0)
        mytv = NULL;
    else
        mytv = tv;

    return mytv;
}

/* this only set three file descriptors, the main agent fd, control agent
 * port fd and traffic generation fd.
 */
void wfaSetSockFiDesc(fd_set *fdset, int *maxfdn1, struct sockfds *fds)
{
    int i;

    FD_ZERO(fdset);
    if(fdset != NULL)
        FD_SET(*fds->agtfd, fdset);

    /* if the traffic generator socket port valid */
    if(*fds->tgfd != -1)
    {
        FD_SET(*fds->tgfd, fdset);
        *maxfdn1 = max(*maxfdn1-1, *fds->tgfd) + 1;
    }

    /* if the control agent socket fd valid */
    if(*fds->cafd != -1)
    {
        FD_SET(*fds->cafd, fdset);
        *maxfdn1 = max(*maxfdn1-1, *fds->cafd) + 1;
    }

#ifdef WFA_WMM_PS_EXT
    /* if the power save socket port valid */
    if(*fds->psfd != -1)
    {
        FD_SET(*fds->psfd, fdset);
        *maxfdn1 = max(*maxfdn1-1, *fds->psfd) + 1;
    }
#endif

    /* if any of wmm traffic stream socket fd valid */
    for(i = 0; i < WFA_MAX_TRAFFIC_STREAMS; i++)
    {
        if(fds->wmmfds[i] != -1)
        {
            FD_SET(fds->wmmfds[i], fdset);
            *maxfdn1 = max(*maxfdn1-1, fds->wmmfds[i]) +1;
        }
    }

    return;
}

/*
 * wfaCtrlSend(): Send control message/response through
 *                control link.
 *  Note: the function used to wfaTcpSend().
 */
int wfaCtrlSend(int sock, unsigned char *buf, int bufLen)
{
    int bytesSent = 0;

    if(bufLen == 0)
        return WFA_FAILURE;

    bytesSent = wSEND(sock, buf, bufLen, 0);

    if(bytesSent == -1)
    {
        DPRINT_WARNING(WFA_WNG, "Error sending tcp packet\n");
    }

    return bytesSent;
}

/*
 * wfaCtrlRecv(): Receive control message/response through
 *                control link.
 *  Note: the function used to wfaTcpRecv().
 */
int wfaCtrlRecv(int sock, unsigned char *buf)
{
    int bytesRecvd = 0;

    bytesRecvd = wRECV(sock, buf, WFA_BUFF_1K-1, 0);

    return bytesRecvd;
}

/*
 * wfaTrafficSendTo(): Send Traffic through through traffic interface.
 *  Note: the function used to wfaUdpSendTo().
 */
int wfaTrafficSendTo(int sock, char *buf, int bufLen, struct sockaddr *to)
{
    int bytesSent;

    bytesSent = wSENDTO(sock, buf, bufLen, 0, to, sizeof(struct sockaddr));

    return bytesSent;
}

/*
 * wfaTrafficRecv(): Receive Traffic through through traffic interface.
 *  Note: the function used to wfaRecvSendTo().
 */
int wfaTrafficRecv(int sock, char *buf, struct sockaddr *from)
{
    int bytesRecvd =0;

#if 0
    /* get current flags setting */
    int ioflags = wFCNTL(sock, F_GETFL, 0);

    /* set only BLOCKING flag to non-blocking */
    wFCNTL(sock, F_SETFL, ioflags | O_NONBLOCK);
#endif

    bytesRecvd = recv(sock, buf, MAX_RCV_BUF_LEN, 0);

    return bytesRecvd;
}

int wfaGetifAddr(char *ifname, struct sockaddr_in *sa)
{
    struct ifreq ifr;
    int fd = wSOCKET(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    if(fd < 0)
    {
        DPRINT_ERR(WFA_ERR, "socket open error\n");
        return WFA_FAILURE;
    }

    wSTRCPY(ifr.ifr_name, ifname);

    ifr.ifr_addr.sa_family = AF_INET;
    if(wIOCTL(fd, SIOCGIFADDR, &ifr) == 0)
    {
        wMEMCPY(sa, (struct sockaddr_in *)&ifr.ifr_addr, sizeof(struct sockaddr_in));
    }
    else
    {
        return WFA_FAILURE;
    }

    wCLOSE(fd);

    return WFA_FAILURE;
}

/*
 * wfaSetProcPriority():
 * With the linux 2.6 kernel, it allows an application process dynamically
 * adjust its running priority level. In order to achieve higher control of
 * packet sending/receiving and timer response, it is helpful to raise the
 * process priority level over others and lower it back once it finishes.
 *
 * This is purely used for performance tuning purpose and not required to
 * port if it is not needed.
 */
int wfaSetProcPriority(int set)
{
    int maxprio, currprio;
    struct sched_param schp;

    wMEMSET(&schp, 0, sizeof(schp));
    sched_getparam(0, &schp);

    currprio = schp.sched_priority;

    if(set != 0)
    {
        if(geteuid() == 0)
        {
            maxprio = sched_get_priority_max(SCHED_FIFO);
            if(maxprio == -1)
            {
                return WFA_FAILURE;
            }

            schp.sched_priority = maxprio;
            if(sched_setscheduler(0, SCHED_FIFO, &schp) != 0)
            {
            }
        }

        if(mlockall(MCL_CURRENT | MCL_FUTURE) != 0)
        {

        }
    }
    else
    {
        if(geteuid() == 0)
        {
            schp.sched_priority = 0;
            if(sched_setscheduler(0, SCHED_OTHER, &schp) != 0)
            {
            }
        }

        munlockall();
    }

    return currprio;
}
