/****************************************************************************
*
* Copyright (c) 2015 Wi-Fi Alliance
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

/**
*    File: wfa_nw_al.c 
*    library functions for network layer abstraction,
*    
*    They are common library to open, close and send receive over any defined interface 
*    so above layer will be modular 
*
*    Revision History:
*
*
*/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>

#include "wfa_debug.h"
#include "wfa_nw_al.h"
#include "wfa_serial.h"
#include "wfa_sock.h"




/* Set the read time out for read function*/
#define READ_TIME_OUT_SEC   30
#define READ_TIME_OUT_U_SEC   0


/**
* @brief This function will create and oipen the interface, like tcp /udp or serial. 
*
* @param t_ifaceHandle *handle  pointer to handle.
* @param char * ifaceName  interface name, eth0, /dev/ttyS0, ...
* @param int typeOfConn    type of connection like TCP / serial
* @param int servFlag      true: it a server connection, false: client connection
*                          this relavent with ethernet connection.
* 
*
**/

int wfaOpenInterFace(t_ifaceHandle *handle, char * ifaceName, int typeOfConn, int servFlag )
{
#ifdef DEBUG_FUNC
    printf("%s \n",__func__);
#endif
    if( (handle == NULL) || (ifaceName == NULL))
    {
        return WFA_ERROR;
    }
    FD_ZERO(&handle->fds);
    handle->timeout.tv_sec = READ_TIME_OUT_SEC;
    handle->timeout.tv_usec = READ_TIME_OUT_U_SEC;
    switch(typeOfConn)
    {
        case CONN_TYPE_TCP:
        {   /* operation related to tcp*/
            handle->ifaceType = CONN_TYPE_TCP;
            if(strlen(ifaceName) > (sizeof(handle->if_attr.ipConn.device) - 1))
                return WFA_ERROR;
            /*expecting not more than 31 char + 1 null */
            strncpy(handle->if_attr.ipConn.device, ifaceName, 31); 
            if(servFlag)
            {
                handle->if_attr.ipConn.clientSockfd = -1;
                /*set up the server but need to call "wfaInterFacePeerConn"
                to listen for the client connection*/
                handle->if_attr.ipConn.sockfd = 
                    wfaCreateTCPServSock(handle->if_attr.ipConn.device,
                        handle->if_attr.ipConn.port);
                if(handle->if_attr.ipConn.sockfd == -1)
                {
                    return WFA_ERROR;
                }
                handle->if_attr.ipConn.srvFlag =  1;
            }
            else
            {   /*tcp client connect*/
                if(handle->if_attr.ipConn.sockfd == -1)
                {
                    if ((handle->if_attr.ipConn.sockfd = socket(PF_INET, 
                        SOCK_STREAM, IPPROTO_TCP)) < 0)
                    {
                        DPRINT_ERR(WFA_ERR, "socket() failed: %i", errno);
                        return WFA_ERROR;
                    }
                    handle->if_attr.ipConn.srvFlag =  0; /* client socket*/
                }
            }
            break;
        }
        case CONN_TYPE_UDP:
        {  /* operation related to udp*/
            handle->ifaceType = CONN_TYPE_UDP;
            if(servFlag)
            {
                if(strlen(ifaceName) > (sizeof(handle->if_attr.ipConn.device) - 1))
                    return WFA_ERROR;
                /*expecting not more than 31 char + 1 null */
                strncpy(handle->if_attr.ipConn.device, ifaceName, 31); 

                handle->if_attr.ipConn.sockfd =  
                    wfaCreateUDPSock(handle->if_attr.ipConn.srcIpaddr,
                        handle->if_attr.ipConn.port);
                if(handle->if_attr.ipConn.sockfd == -1)
                {
                    return WFA_ERROR;
                }
                handle->if_attr.ipConn.srvFlag =  1;
            }
            break;
        }
        case CONN_TYPE_SERIAL:
        {  /* operation related to serial*/
            printf("trying to open serial port %s \n",ifaceName);
            handle->ifaceType = CONN_TYPE_SERIAL;
             if(strlen(ifaceName) > (sizeof(handle->if_attr.serial.device) - 1))
                return WFA_ERROR;
            /*expecting not more than 31 char + 1 null */
            strncpy(handle->if_attr.ipConn.device, ifaceName, 31); 
            wfaOpenSerial(handle, handle->if_attr.serial.device, 
                handle->if_attr.serial.baudrate);
            break;
        }
    }
    return WFA_SUCCESS;
}


/**
* @brief  
*
* @param t_ifaceHandle *handle  pointer to handle.
* @param char *destIpAddr
* @param int dstPort
* @param int dstBuad      
* @param int typeOfConn
* 
*
**/
int wfaInterFacePeerInfoSet(t_ifaceHandle *handle, char *destIpAddr, int dstPort, int dstBuad, int typeOfConn )
{
#ifdef DEBUG_FUNC
        printf("%s \n",__func__);
#endif

    if(handle == NULL) {
        return WFA_ERROR;
    }
    if(typeOfConn !=  handle->ifaceType) {
        DPRINT_ERR(WFA_ERR, "type of connection difference \n");
        return WFA_ERROR;
    }
    switch(typeOfConn) {
        case CONN_TYPE_TCP:
        {   /* operation related to tcp*/
            if(destIpAddr == NULL)
                return WFA_ERROR;
            memset(&handle->if_attr.ipConn.to, 0, sizeof(handle->if_attr.ipConn.to)); 
            handle->if_attr.ipConn.to.sin_family      = AF_INET;
            handle->if_attr.ipConn.to.sin_addr.s_addr = inet_addr(destIpAddr);
            handle->if_attr.ipConn.to.sin_port        = htons(dstPort);
            break;
        }
        case CONN_TYPE_UDP:
        {  /* operation related to udp*/
            if(destIpAddr == NULL)
                return WFA_ERROR;
            memset(&handle->if_attr.ipConn.to, 0, sizeof(handle->if_attr.ipConn.to)); 
            handle->if_attr.ipConn.to.sin_family      = AF_INET;
            handle->if_attr.ipConn.to.sin_addr.s_addr = inet_addr(destIpAddr);
            handle->if_attr.ipConn.to.sin_port        = htons(dstPort);
            break;
        }
        case CONN_TYPE_SERIAL:
        {  /* operation related to serial*/
            /* for this connection there is nothing like peer connection*/
            if(handle->if_attr.serial.baudrate != dstBuad)
            {
                printf("there is mismatch in dst buad\n");
     //           DPRINT_ERR(WFA_ERR, "diffrebce in baud rate: local :%d remote: %d\n",
     //                   handle->if_attr.serial.baudrate, dstBuad);
     //           return WFA_ERROR;
            }
            break;
        }
    }
    return WFA_SUCCESS;
}


/**
* @brief  
*
* @param t_ifaceHandle *handle  pointer to handle.
* @param char *destIpAddr
* @param int dstPort
* @param int dstBuad      
* @param int typeOfConn
* 
*
**/
int wfaInterFacePeerConn(t_ifaceHandle *handle )
{
    int typeOfConn;
    int retStatus =0;
#ifdef DEBUG_FUNC
    printf("%s \n",__func__);
#endif

    if (handle == NULL) {
        return WFA_ERROR;
    }
    typeOfConn =  handle->ifaceType;
    switch(typeOfConn) {
        case CONN_TYPE_TCP:
        {   /* operation related to tcp*/
            if(handle->if_attr.ipConn.srvFlag) {
                /* wait for client connection, just start listen */
                /* accept the connection*/
                if (handle->if_attr.ipConn.clientSockfd == -1) {
                    handle->if_attr.ipConn.clientSockfd = 
                            wfaAcceptTCPConn( handle->if_attr.ipConn.sockfd);
                    printf("clent sock %d\n",handle->if_attr.ipConn.clientSockfd);
                }
                else {
                    printf("clent sock %d\n",handle->if_attr.ipConn.clientSockfd);
                }
            }
            else {
                if(handle->if_attr.ipConn.sockfd != -1) {
                    retStatus = wCONNECT(handle->if_attr.ipConn.sockfd,
                        (struct sockaddr *)&handle->if_attr.ipConn.to, 
                        sizeof(handle->if_attr.ipConn.to));
                    if(retStatus < 0)
                        printf("Connection error %d\n", errno);
                }
            }
            break;
        }
        case CONN_TYPE_UDP:
        {  /* operation related to udp*/
            break;
        }
        case CONN_TYPE_SERIAL:
        {  /* operation related to serial*/
            /* for this connection there is nothing like peer connection*/
            break;
        }
    }
    return retStatus;
}


/**
* @brief  
*
* @param t_ifaceHandle *handle  pointer to handle.
* @param char *destIpAddr
* @param int dstPort
* @param int dstBuad      
* @param int typeOfConn
* 
*
**/

int wfaInterFacePeerConnClose(t_ifaceHandle *handle )
{
    int typeOfConn;

#ifdef DEBUG_FUNC
    printf("%s \n",__func__);
#endif

    if (handle == NULL) {
        return WFA_ERROR;
    }
    typeOfConn =  handle->ifaceType;
    switch(typeOfConn) {
        case CONN_TYPE_TCP:
        {   /* operation related to tcp*/
            if(handle->if_attr.ipConn.srvFlag) {
                /* close the connection*/
                if(handle->if_attr.ipConn.clientSockfd != -1)
                {
                    shutdown(handle->if_attr.ipConn.clientSockfd, SHUT_WR);
                    close(handle->if_attr.ipConn.clientSockfd);
                    handle->if_attr.ipConn.clientSockfd = -1;
                }
            }
            else {
                if(handle->if_attr.ipConn.sockfd != -1)
                {
                    shutdown(handle->if_attr.ipConn.sockfd, SHUT_WR);
                    close(handle->if_attr.ipConn.sockfd);
                    handle->if_attr.ipConn.sockfd = -1;
                }
            }
            break;
        }
        case CONN_TYPE_UDP:
        {  /* operation related to udp*/
            break;
        }
        case CONN_TYPE_SERIAL:
        {  /* operation related to serial*/
            /* disconnect the uart*/
            wfaInterFaceClose(handle);
            break;
        }
    }
    return WFA_SUCCESS;
}



/**
* @brief  
*
* @param t_ifaceHandle *handle  pointer to handle.
* @param char *destIpAddr
* @param int dstPort
* @param int dstBuad      
* @param int typeOfConn
* 
*
**/
int wfaInterFaceClose(t_ifaceHandle *handle )
{
#ifdef DEBUG_FUNC
    printf("%s \n",__func__);
#endif

    if (handle == NULL) {
        return WFA_ERROR;
    }

    switch(handle->ifaceType) {
        case CONN_TYPE_TCP:
        {   /* operation related to tcp*/
            if(handle->if_attr.ipConn.srvFlag)
            {
                /* check for the client socket */
                if(handle->if_attr.ipConn.clientSockfd != -1)
                {
                    shutdown(handle->if_attr.ipConn.clientSockfd, SHUT_WR);
                    close(handle->if_attr.ipConn.clientSockfd);
                    handle->if_attr.ipConn.clientSockfd = -1;
                }
            }
            shutdown(handle->if_attr.ipConn.clientSockfd, SHUT_RDWR);
            close(handle->if_attr.ipConn.sockfd);
            handle->if_attr.ipConn.sockfd = -1;
            break;
        }
        case CONN_TYPE_UDP:
        {  /* operation related to udp*/
            wCLOSE(handle->if_attr.ipConn.sockfd);
            break;
        }
        case CONN_TYPE_SERIAL:
        {  /* operation related to serial*/
            /* for this connection there is nothing like peer connection*/
            wfaCloseSerial(handle);
            break;
        }
    }
    return WFA_SUCCESS;
}

/**
* @brief  
*
* @param t_ifaceHandle *handle  pointer to handle.
* @param char *destIpAddr
* @param int dstPort
* @param int dstBuad      
* @param int typeOfConn
* 
*
**/
int wfaInterFaceDataSend(t_ifaceHandle *handle, char *buffer, int bufferLen )
{
    int bytesSent = 0;
//#ifdef DEBUG_FUNC
    printf("%s -> to send %d \n",__func__, bufferLen);
//#endif

    if( (handle == NULL) || (buffer == NULL) || (bufferLen <= 0)) {
        return WFA_ERROR;
    }
    switch(handle->ifaceType) {
        case CONN_TYPE_TCP:
        {   /* tcp send */
            if(handle->if_attr.ipConn.srvFlag){
                bytesSent = wSEND(handle->if_attr.ipConn.clientSockfd, buffer, bufferLen, 0); 
                if(bytesSent == -1) {
                    DPRINT_ERR(WFA_ERR, "Error sending tcp packet\n");
                }
            }
            else {
                bytesSent = wSEND(handle->if_attr.ipConn.sockfd, buffer, bufferLen, 0); 
                if(bytesSent == -1) {
                    DPRINT_ERR(WFA_ERR, "Error sending tcp packet\n");
                }
            }
            printf("able to sent %d \n", bytesSent );
            return bytesSent;
            break;
        }
        case CONN_TYPE_UDP:
        {  /*  udp sendto*/
            bytesSent = wSENDTO(handle->if_attr.ipConn.sockfd, buffer, bufferLen, 0,
                        (struct sockaddr *)&handle->if_attr.ipConn.to, sizeof(handle->if_attr.ipConn.to));
            return bytesSent;
            break;
        }
        case CONN_TYPE_SERIAL:
        {  /* serial wrtie to the fd*/
            wfaSerialSend(handle, buffer, bufferLen);
            break;
        }
    }
    return WFA_SUCCESS;
}


/**
* @brief  
*
* @param t_ifaceHandle *handle  pointer to handle.
* @param char *destIpAddr
* @param int dstPort
* @param int dstBuad      
* @param int typeOfConn
* 
*
**/
int wfaInterFaceDataRecv(t_ifaceHandle *handle, char *buffer, int bufferLen, int *recvLen )
{
    int bytesRecvd = 0;
    int retStatus = WFA_SUCCESS;
    int ret = 0;
    
#ifdef DEBUG_FUNC
    printf("%s \n",__func__);
#endif
    if( (handle == NULL) || (buffer == NULL) || (bufferLen <= 0))
    {
        printf("handle or buffer error \n");
        return WFA_ERROR;
    }
    switch(handle->ifaceType)
    {
        case CONN_TYPE_TCP:
        {   /* tcp send */
            FD_ZERO(&handle->fds);
            if(handle->if_attr.ipConn.srvFlag)
            {
                FD_SET(handle->if_attr.ipConn.clientSockfd, &handle->fds);
                ret = select(handle->if_attr.ipConn.clientSockfd+1, &handle->fds, NULL, NULL, &handle->timeout);
                printf("after select ret %d\n",ret);
                if ( ret > 0)
                {
                    bytesRecvd = wRECV(handle->if_attr.ipConn.clientSockfd, 
                        buffer, bufferLen, 0);
                }
                else
                {
                   bytesRecvd = -1;
                   printf("timeout on receive\n");
                }
                if(bytesRecvd == -1)
                {
                    DPRINT_ERR(WFA_ERR, "Error in receiving tcp packet\n");
                    *recvLen = 0;
                    retStatus = WFA_ERROR;
                }
            }
            else {
                FD_SET(handle->if_attr.ipConn.sockfd, &handle->fds);
                if (select(handle->if_attr.ipConn.sockfd+1, &handle->fds, NULL, NULL, &handle->timeout) > 0)
                {
                    bytesRecvd = wRECV(handle->if_attr.ipConn.sockfd, buffer, bufferLen, 0);
                }
                else
                {
                   bytesRecvd = -1;
                   printf("timeout on receive\n");
                }
                if(bytesRecvd == -1)
                {
                    DPRINT_ERR(WFA_ERR, "Error in receiving tcp packet\n");
                    *recvLen = 0;
                    retStatus = WFA_ERROR;
                }
             }
            printf("byte recv %d \n",bytesRecvd);
            *recvLen = bytesRecvd;
            break;
        }
        case CONN_TYPE_UDP:
        {  /*  udp recvfrom*/
            *recvLen = wRECVFROM(handle->if_attr.ipConn.sockfd,buffer, 
                bufferLen, 0, NULL, NULL);
            return WFA_SUCCESS;
            break;
        }
        case CONN_TYPE_SERIAL:
        {  /* serial wrtie to the fd*/
            /*need to do little more*/
            retStatus = wfaSerialRecv(handle, buffer, bufferLen, recvLen);
            if(WFA_SUCCESS != retStatus)
                printf("serial recev error \n");
            break;
        }
    }
    return retStatus;
}

