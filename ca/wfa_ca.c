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


/*
 * File: wfa_ca.c
 *       This is the main program for Control Agent.
 *
 */
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <sys/select.h>
#include <getopt.h>     /* for get_option */


#include "wfa_debug.h"
#include "wfa_main.h"
#include "wfa_types.h"
#include "wfa_tlv.h"
#include "wfa_tg.h"
#include "wfa_cmds.h"
#include "wfa_miscs.h"
#include "wfa_sock.h"
#include "wfa_nw_al.h"
#include "wfa_ca.h"
#include "wfa_agtctrl.h"

#define WFA_ENV_AGENT_IPADDR "WFA_ENV_AGENT_IPADDR"

extern int xcCmdProcGetVersion(unsigned char *parms);
extern dutCommandRespFuncPtr wfaCmdRespProcFuncTbl[];
extern typeNameStr_t nameStr[];
extern char gRespStr[];

int gSock = -1;
int gCaSockfd = -1;
int xcSockfd = -1;
int btSockfd;


int gtgSend, gtgRecv, gtgTransac;


char gnetIf[32] = "any";
char gCaNetIf[32] = "any";

char logPath[100] = "";

tgStream_t    *theStreams;
long          itimeout = 0;

unsigned short wfa_defined_debug = WFA_DEBUG_ERR | WFA_DEBUG_WARNING | WFA_DEBUG_INFO;
unsigned short dfd_lvl = WFA_DEBUG_DEFAULT | WFA_DEBUG_ERR | WFA_DEBUG_INFO;

/*
 * the output format can be redefined for file output.
 */

void help_usage(char *str)
{
    printf( "Usage:  %s -i <eth iface> -I <interface> -P <local port> -T <type> [Options: ] \n", str);
    printf( "  Options:                                                                \n"
            "     -i   --iface       ethernet interface for control agent              \n"
            "     -I   --dutif       interface either ethernet or serial device name   \n"
            "     -T   --type        type interface like serial(1) TCP(2) UDP(3)       \n"
            "     -P   --port        local sever port address to start server          \n"
            "     -d   --dutip       dut ip address in case of TCP/UDP                 \n"
            "     -r   --dutport     dut port address in case of TCP                   \n"
            "     -b   --baud        baud rate in case of serial                       \n"
            "     -g   --log         log file path for debug                           \n"
            "     -h   --help        display usage                                     \n");
}


static const struct option longIPOptions[] = {
    { "iface",      required_argument, NULL, 'i' },
    { "dutif",      required_argument, NULL, 'I' },
    { "type",       required_argument, NULL, 'T' },
    { "dutip",      required_argument, NULL, 'd' },
    { "port",       required_argument, NULL, 'P' },
    { "dutport",    required_argument, NULL, 'r' },
    { "baud",       required_argument, NULL, 'b' },
    { "log",        required_argument, NULL, 'g' },
    { "help",       no_argument, NULL, 'h' },
    { NULL,         no_argument, NULL, 0 }
};
static const char *optionString = "i:I:T:d:P:r:b:g:h";


int main(int argc, char *argv[])
{
    t_ifaceHandle  ctrlRecvHandle;
    t_ifaceHandle  dutHandle;
    int typeOfConn=0;
    int opt;
    int optionIndex = 0;
#if 0 
    // these variable used for or TG 
    int nfds; 
    struct sockaddr_in servAddr; 
    char *servIP=NULL;
    fd_set sockSet;
    int slen;
#endif
    /* remote port for sending command over tcp */
    unsigned short remoteServPort = 0; 
    unsigned short locPortNo = 0;
    int baudrate = 0;
    int retStatus = 0;
    
    /*char *tstr=NULL;*/
    char dutSrvIP[WFA_IP_ADDR_STR_LEN];
#if DEBUG 
    int bytesRcvd;                   
#endif

    char cmdName[WFA_BUFF_32];
    int i, isFound = 0, nbytes, ret_status;
    WORD tag;
    int cmdLen = WFA_BUFF_1K;
    /* int maxfdn1; */
    BYTE xcCmdBuf[WFA_BUFF_4K];
    BYTE caCmdBuf[WFA_BUFF_4K];
    BYTE pcmdBuf[WFA_BUFF_1K];
    char *pcmdStr = NULL;
    char respStr[WFA_BUFF_512];

    /*  temporarily disable 
    //start of CLI handling variables
    char wfaCliBuff[128];
    FILE *wfaCliFd;
    char * cliCmd;
    */
    char *tempCmdBuff;

    while ((opt = getopt_long(argc, argv, optionString, longIPOptions, &optionIndex)) != -1) 
    {
        switch (opt) {
        case 'i':
            printf ("option -i with value %s\n", optarg);
            strncpy(gCaNetIf, optarg, 31);
            break;
        case 'I':
            printf ("option -I with value %s\n", optarg);
            strncpy(gnetIf, optarg, 31);
            break;
        case 'T':
            /* interface type */
            /* none: 0, serial: 1, tcp: 2, udp: 3 */
            typeOfConn = atoi(optarg);
            break;
        case 'P':
            /* local server port*/
            locPortNo = atoi(optarg);
            break;
        case 'b':
            /* baudrate for serial operation*/
            baudrate = atoi(optarg);
            break;
        case 'd':
            if(isIpV4Addr(optarg)== WFA_ERROR) {
                printf("invalid ip address %s\n",optarg);
                help_usage(argv[0]);
                exit(1);
            }
            strncpy(dutSrvIP, optarg, WFA_IP_ADDR_STR_LEN);
            break;
        case 'r':
            /* dut server port  use in case dut use TCP/UDP*/
            remoteServPort = atoi(optarg);
            break;
        case 'g':
            /* log path for debug */
            printf ("option -I with value %s\n", optarg);
            strncpy(logPath, optarg, sizeof(logPath)-1);
            break;
        case 'h':
            help_usage(argv[0]);
            exit(1);
            break;
        case ':':
            printf ("option  --%s must have value \n", longIPOptions[optionIndex].name);
            break;
        case '?':
        /* getopt_long() set a variable, just keep going */
        break;
        }
    }


    if (locPortNo == 0) {
        printf ("wrong local port for server to start %d\n",locPortNo);
        help_usage(argv[0]);
        exit(1);
    }

    if ((CONN_TYPE_SERIAL == typeOfConn)) {
        if ( baudrate == 0) {
            printf ("wrong baud rate \n");
            exit(1);
        }
    }
    else if (CONN_TYPE_TCP == typeOfConn || CONN_TYPE_UDP == typeOfConn) {
        if ( remoteServPort == 0) {
            printf ("wrong remote port for IP connection\n");
            exit(1);
        }
    }
    else {
        printf ("type (-T) should be with correct value %d\n", typeOfConn);
        help_usage(argv[0]);
        exit(1);
    }

    /* check need in case logpath allocated as require*/
    if(logPath != NULL && strlen(logPath)) {
        FILE *logfile;
        int fd;
        logfile = fopen(logPath,"a");
        if(logfile != NULL) {
            fd = fileno(logfile);
            DPRINT_INFO(WFA_OUT,"redirecting the output to %s\n", logPath);
            dup2(fd,1);
            dup2(fd,2);
        }
        else {
            DPRINT_ERR(WFA_ERR, "Cant open the log file continuing without redirecting\n");
        }
    }

    ctrlRecvHandle.if_attr.ipConn.port = locPortNo;
    retStatus = wfaOpenInterFace(&ctrlRecvHandle, gCaNetIf, CONN_TYPE_TCP, CONNECTION_SERVER);
    if(retStatus) {
        printf("CA server faild to start \n");
        exit(1);
    }
    /* must be removed*/
    dutHandle.if_attr.ipConn.sockfd = -1;

//    maxfdn1 = tmsockfd + 1;
//    FD_ZERO(&sockSet);
#if 0
    if(gSock == -1)
    {
        if ((gSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        {
            DPRINT_ERR(WFA_ERR, "socket() failed: %i", errno);
            exit(1);
        }

        memset(&servAddr, 0, sizeof(servAddr)); 
        servAddr.sin_family      = AF_INET;
        servAddr.sin_addr.s_addr = inet_addr(servIP);
        servAddr.sin_port        = htons(servPort);

        if (connect(gSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        {
            DPRINT_ERR(WFA_ERR, "connect() failed: %i", errno);
            exit(1);
        }
    }
#endif


    /*infinite loop */
    for(;;)
    {
#if 0        
        FD_ZERO(&sockSet);
        FD_SET(tmsockfd, &sockSet);
        maxfdn1 = tmsockfd + 1;

        if(gCaSockfd != -1)
        {
            FD_SET(gCaSockfd, &sockSet);
            if(maxfdn1 < gCaSockfd)
                maxfdn1 = gCaSockfd +1; 
        }

        if(gSock != -1)
        {
            FD_SET(gSock, &sockSet);
            if(maxfdn1 < gSock)
                maxfdn1 = gSock +1; 
        }

        if((nfds = select(maxfdn1, &sockSet, NULL, NULL, NULL)) < 0)
        {
            if(errno == EINTR)
                continue;
            else
                DPRINT_WARNING(WFA_WNG, "select error %i", errno);
        }
 #endif
        DPRINT_INFO(WFA_OUT, "new event \n");

        wfaInterFacePeerConn( &ctrlRecvHandle);
        printf("client got connected\n");
/*
        if(FD_ISSET(tmsockfd, &sockSet))
        {
            gCaSockfd = wfaAcceptTCPConn(tmsockfd);
            DPRINT_INFO(WFA_OUT, "accept new connection\n");
            continue;
        }
*/

        memset(xcCmdBuf, 0, WFA_BUFF_4K);
        memset(gRespStr, 0, WFA_BUFF_512);
#if 0

        nbytes = wfaCtrlRecv(gCaSockfd, xcCmdBuf); 
        if(nbytes <=0)
        {
            shutdown(gCaSockfd, SHUT_WR);
            close(gCaSockfd);
            gCaSockfd = -1;
            continue;
        }
#endif

        retStatus = wfaInterFaceDataRecv(&ctrlRecvHandle, (char *)xcCmdBuf, WFA_BUFF_1K, &nbytes);
        printf("retStatus %d nbytes %d {%s}",retStatus, nbytes, xcCmdBuf);
        if(nbytes <=0)
        {
            printf("Ctrl data receive error nbytes = %d \n",nbytes);
            /* may not be correct idea unless recev wait till it gets some data*/
            wfaInterFacePeerConnClose(&ctrlRecvHandle);
            continue;
        }

        /*
         * send back to command line or TM.
         */
        //sleep(1); /* having this is for slowing down unexpected output result on CLI command sometimes */
        memset(respStr, 0, WFA_BUFF_128);
        sprintf(respStr, "status,RUNNING\r\n");
        retStatus = wfaInterFaceDataSend(&ctrlRecvHandle ,respStr, strlen(respStr));
        if(retStatus == -1)
            continue;

#if 0
/*      wfaCtrlSend(gCaSockfd, (BYTE *)respStr, strlen(respStr));  */
        DPRINT_INFO(WFA_OUT, "%s\n", respStr);
        DPRINT_INFO(WFA_OUT, "message %s %i\n", xcCmdBuf, nbytes);
        slen = (int )strlen((char *)xcCmdBuf);

        DPRINT_INFO(WFA_OUT, "last %x last-1  %x last-2 %x last-3 %x\n", 
                        cmdName[slen], cmdName[slen-1], cmdName[slen-2], cmdName[slen-3]);
        xcCmdBuf[slen-3] = '\0';
#endif

        if(CONN_TYPE_TCP ==typeOfConn )
        {
            wfaOpenInterFace(&dutHandle, gnetIf, CONN_TYPE_TCP, CONNECTION_CLIENT);
            wfaInterFacePeerInfoSet( &dutHandle, dutSrvIP, remoteServPort, 0, typeOfConn);
            retStatus = wfaInterFacePeerConn(&dutHandle);
            if( WFA_ERROR == retStatus ) {
                printf("Dut Connection failed\n");
                retStatus = wfaInterFaceDataSend(&ctrlRecvHandle ,"status,ERROR\r\n",
                    strlen("status,ERROR\r\n"));
                wfaInterFaceClose(&dutHandle);
                continue;
            }
        }
        else if(CONN_TYPE_SERIAL==typeOfConn )
        {
            dutHandle.if_attr.serial.baudrate = baudrate;
            wfaOpenInterFace(&dutHandle, gnetIf, CONN_TYPE_SERIAL, CONNECTION_CLIENT);
            wfaInterFacePeerInfoSet( &dutHandle, dutSrvIP, remoteServPort, 115200, typeOfConn);
        }
        isFound = 0;
        tempCmdBuff = (char* )malloc(sizeof(xcCmdBuf));
        memcpy(tempCmdBuff, xcCmdBuf, sizeof(xcCmdBuf));

        memcpy(cmdName, strtok_r((char *)tempCmdBuff, ",", (char **)&pcmdStr), 32);
        printf("\nInside the CLI huck block \n");
#if 0
        wfaCliFd=fopen("/etc/WfaEndpoint/wfa_cli.txt","r");
        printf("\nAfter File open \n");
        if(wfaCliFd!= NULL)
        {
            //printf("\nInside File open \n");
            while(fgets(wfaCliBuff, 128, wfaCliFd) != NULL)
            {
                //printf("Line read from CLI file : %s",wfaCliBuff);
                if(ferror(wfaCliFd))
                    break;
                cliCmd=strtok(wfaCliBuff,"-");
                if(strcmp(cliCmd,cmdName) == 0)
                {
                    strcpy(cmdName,"wfa_cli_cmd");
                    pcmdStr = (char *)&xcCmdBuf[0];
                    break;
                }
            }
            fclose(wfaCliFd);
        }
#endif
        printf("\nOutside the new block \n");
        free(tempCmdBuff);
        if(strcmp(cmdName,"wfa_cli_cmd") != 0)
        memcpy(cmdName, strtok_r((char *)xcCmdBuf, ",", (char **)&pcmdStr), 32);
        
        i = 0;
        while(nameStr[i].type != -1)
        {
            if(strcmp(nameStr[i].name, cmdName) == 0)
            {
                isFound = 1;
                break;
            }
            i++;
        }

        DPRINT_INFO(WFA_OUT, "%s\n", cmdName);

        if(isFound == 0)
        {
            sleep(1);
            sprintf(respStr, "status,INVALID\r\n");
            retStatus = wfaInterFaceDataSend(&ctrlRecvHandle ,(char *)respStr, strlen(respStr));
            DPRINT_WARNING(WFA_WNG, "Command not valid, check the name\n");
            continue;
        }

        memset(pcmdBuf, 0, WFA_BUFF_1K); 
        if(nameStr[i].cmdProcFunc(pcmdStr, pcmdBuf, &cmdLen)==WFA_FAILURE)
        {
            sleep(1);
            sprintf(respStr, "status,INVALID\r\n");
            retStatus = wfaInterFaceDataSend(&ctrlRecvHandle ,(char *)respStr, strlen(respStr));
            DPRINT_WARNING(WFA_WNG, "Incorrect command syntax\n");
            continue;
        }
        /*
         * send to DUT.
         */
        DPRINT_INFO(WFA_OUT, "sent to DUT\n");
        retStatus = wfaInterFaceDataSend(&dutHandle, (char *)pcmdBuf, cmdLen);
        if(retStatus == -1)
        {
            DPRINT_WARNING(WFA_WNG, "Incorrect sending ...\n");
            wfaInterFaceClose(&dutHandle);
            continue;
        }
        //sleep(1);

        DPRINT_INFO(WFA_OUT, "received from DUT\n");
        memset(respStr, 0, WFA_BUFF_128);
        memset(caCmdBuf, 0, WFA_BUFF_4K);
        retStatus = wfaInterFaceDataRecv(&dutHandle,(char *) caCmdBuf, WFA_BUFF_4K, &nbytes);
        if (retStatus < 0)
        {
            DPRINT_WARNING(WFA_WNG, "recv() failed or connection closed prematurely");
            wfaInterFaceClose(&dutHandle);
            continue;
        }
/*      if ((bytesRcvd = recv(gSock, caCmdBuf, WFA_BUFF_4K, 0)) <= 0)
        {
            DPRINT_WARNING(WFA_WNG, "recv() failed or connection closed prematurely");
            continue;
        }
*/
#if DEBUG 
        for(i = 0; i< bytesRcvd; i++)
            printf("%x ", caCmdBuf[i]);
            printf("\n");
#endif
        tag = ((wfaTLV *)caCmdBuf)->tag;
        memcpy(&ret_status, caCmdBuf+4, 4);

        DPRINT_INFO(WFA_OUT, "tag %i \n", tag);
        if(tag != 0 && wfaCmdRespProcFuncTbl[tag] != NULL)
        {
            wfaCmdRespProcFuncTbl[tag](caCmdBuf);
            /*send back to ctrl agent*/
            retStatus = wfaInterFaceDataSend(&ctrlRecvHandle ,
                    (char *)gRespStr, strlen(gRespStr));
        }
        else
        {
        DPRINT_WARNING(WFA_WNG, "function not defined\n");
        }
        /* close the dut connection */
        wfaInterFaceClose(&dutHandle);
        wfaInterFacePeerConnClose(&ctrlRecvHandle);
    } /* for */
    wfaInterFaceClose(&ctrlRecvHandle);
    return 0;
}
