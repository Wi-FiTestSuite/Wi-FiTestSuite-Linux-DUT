/****************************************************************************
*
* Copyright (c) 2014 Wi-Fi Alliance
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
 

#ifndef _WFA_NW_AL_H
#define _WFA_NW_AL_H

#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <termios.h>    /* for terminal */
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include "wfa_portall.h"
#include "wfa_types.h"
#include "wfa_sock.h"


#define CONN_TYPE_NONE                       0
#define CONN_TYPE_SERIAL                     1
#define CONN_TYPE_TCP                        2
#define CONN_TYPE_UDP                        3  

#define CONNECTION_SERVER   1
#define CONNECTION_CLIENT   0

#define IPV4_ADDR_STR_LEN 16


typedef struct  tcpUdp{
        char device[32];
        int sockfd;
        int clientSockfd;
        int srvFlag;   /* true (1): its server false(0): client*/
        int port;
        char srcIpaddr[IPV4_ADDR_STR_LEN];
        struct sockaddr_in    to;
        struct sockaddr_in    from;
    }t_tcpUdp;

typedef struct  serial{
        char device[32];
        int fd;
        int baudrate;

        struct termios oldtio;
        struct termios newtio;
    }t_serial;

typedef struct ifaceHandle{
    int ifaceType;      /** numeric id*/
    char ifName[16];    /** name of the interface*/
    fd_set fds;
    struct timeval timeout; /* timeout for the blocking read */
    union if_attr{
        t_serial serial;
        t_tcpUdp ipConn;
    }if_attr;
}t_ifaceHandle;





extern int wfaOpenInterFace(t_ifaceHandle *handle, char * ifaceName, int typeOfConn, int servFlag );
extern int wfaInterFacePeerInfoSet(t_ifaceHandle *handle, char *destIpAddr, int dstPort, int dstBuad, int typeOfConn );
extern int wfaInterFacePeerConn(t_ifaceHandle *handle );
extern int wfaInterFacePeerConnClose(t_ifaceHandle *handle );

extern int wfaInterFaceClose(t_ifaceHandle *handle );
extern int wfaInterFaceDataSend(t_ifaceHandle *handle, char *buffer, int bufferLen );
extern int wfaInterFaceDataRecv(t_ifaceHandle *handle, char *buffer, int bufferLen, int *recvLen );

#endif /* _WFA_NW_AL_H */
