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

#include "wfa_debug.h"
#include "wfa_main.h"
#include "wfa_types.h"
#include "wfa_tlv.h"
#include "wfa_tg.h"
#include "wfa_cmds.h"
#include "wfa_miscs.h"
#include "wfa_sock.h"
#include "wfa_ca.h"
#include "wfa_agtctrl.h"

#define WFA_ENV_AGENT_IPADDR "WFA_ENV_AGENT_IPADDR"

extern int xcCmdProcGetVersion(unsigned char *parms);
extern dutCommandRespFuncPtr wfaCmdRespProcFuncTbl[];
extern typeNameStr_t nameStr[];
extern char gRespStr[];

int gSock = -1, tmsockfd, gCaSockfd = -1, xcSockfd, btSockfd;
int gtgSend, gtgRecv, gtgTransac;
char gnetIf[32] = "any";
tgStream_t    *theStreams;
long          itimeout = 0;

unsigned short wfa_defined_debug = WFA_DEBUG_ERR | WFA_DEBUG_WARNING | WFA_DEBUG_INFO;
unsigned short dfd_lvl = WFA_DEBUG_DEFAULT | WFA_DEBUG_ERR | WFA_DEBUG_INFO;

/*
 * the output format can be redefined for file output.
 */

int main(int argc, char *argv[])
{
    int nfds;
    struct sockaddr_in servAddr;
    unsigned short servPort, myport;
    char *servIP=NULL, *tstr=NULL;
    int bytesRcvd;
    fd_set sockSet;
    char cmdName[WFA_BUFF_32];
    int i, isFound = 0, nbytes, ret_status, slen;
    WORD tag;
    int tmsockfd, cmdLen = WFA_BUFF_1K;
    int maxfdn1;
    BYTE xcCmdBuf[WFA_BUFF_4K];
    BYTE caCmdBuf[WFA_BUFF_4K];
    BYTE pcmdBuf[WFA_BUFF_1K];
    char *pcmdStr = NULL;
    char respStr[WFA_BUFF_512];

    //start of CLI handling variables
    char wfaCliBuff[128];
    FILE *wfaCliFd;
    char * cliCmd,*tempCmdBuff;
    if(argc < 3)
        {
            DPRINT_ERR(WFA_ERR, "Usage: %s <control interface> <local control agent port>\n", argv[0]);
            exit(1);
        }

    myport = atoi(argv[2]);

    if(argc > 3)
        {
            if(argc < 5)
                {
                    DPRINT_ERR(WFA_ERR, "Usage: %s <control interface> <local control agent port> <DUT IP ADDRESS> <DUT PORT>\n", argv[0]);
                    exit(1);
                }
            servIP = argv[3];
            if(isIpV4Addr(argv[3])== WFA_FAILURE)
                return WFA_FAILURE;
            if(isNumber(argv[4])== WFA_FAILURE)
                return WFA_FAILURE;
            servPort = atoi(argv[4]);
            if(argc > 5)
                {
                    FILE *logfile;
                    int fd;
                    logfile = fopen(argv[5],"a");
                    if(logfile != NULL)
                        {
                            fd = fileno(logfile);
                            DPRINT_INFO(WFA_OUT,"redirecting the output to %s\n",argv[5]);
                            dup2(fd,1);
                            dup2(fd,2);
                        }
                    else
                        {
                            DPRINT_ERR(WFA_ERR, "Cant open the log file continuing without redirecting\n");
                        }
                }
        }
    else
        {
            if((tstr = getenv("WFA_ENV_AGENT_IPADDR")) == NULL)
                {
                    DPRINT_ERR(WFA_ERR, "Environment variable WFA_ENV_AGENT_IPADDR not set or specify DUT IP/PORT\n");
                    exit(1);
                }
            if(isIpV4Addr(tstr)== WFA_FAILURE)
                return WFA_FAILURE;
            servIP= tstr;
            if((tstr = getenv("WFA_ENV_AGENT_PORT")) == NULL)
                {
                    DPRINT_ERR(WFA_ERR, "Environment variable WFA_ENV_AGENT_PORT not set or specify DUT IP/PORT\n");
                    exit(1);
                }
            if(isNumber(tstr)== WFA_FAILURE)
                return WFA_FAILURE;
            servPort = atoi(tstr);
        }

    tmsockfd = wfaCreateTCPServSock(myport);

    maxfdn1 = tmsockfd + 1;

    FD_ZERO(&sockSet);
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

    for(;;)
        {
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

            DPRINT_INFO(WFA_OUT, "new event \n");
            if(FD_ISSET(tmsockfd, &sockSet))
                {
                    gCaSockfd = wfaAcceptTCPConn(tmsockfd);
                    DPRINT_INFO(WFA_OUT, "accept new connection\n");
                    continue;
                }

            if(gCaSockfd > 0 && FD_ISSET(gCaSockfd, &sockSet))
                {
                    memset(xcCmdBuf, 0, WFA_BUFF_4K);
                    memset(gRespStr, 0, WFA_BUFF_512);

                    nbytes = wfaCtrlRecv(gCaSockfd, xcCmdBuf);
                    if(nbytes <=0)
                        {
                            shutdown(gCaSockfd, SHUT_WR);
                            close(gCaSockfd);
                            gCaSockfd = -1;
                            continue;
                        }

                    /*
                     * send back to command line or TM.
                     */
                    memset(respStr, 0, WFA_BUFF_128);
                    sprintf(respStr, "status,RUNNING\r\n");
                    wfaCtrlSend(gCaSockfd, (BYTE *)respStr, strlen(respStr));

                    DPRINT_INFO(WFA_OUT, "%s\n", respStr);
                    DPRINT_INFO(WFA_OUT, "message %s %i\n", xcCmdBuf, nbytes);
                    slen = (int )strlen((char *)xcCmdBuf);

                    DPRINT_INFO(WFA_OUT, "last %x last-1  %x last-2 %x last-3 %x\n", cmdName[slen], cmdName[slen-1], cmdName[slen-2], cmdName[slen-3]);

                    xcCmdBuf[slen-3] = '\0';

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

                    isFound = 0;

                    tempCmdBuff=(char* )malloc(sizeof(xcCmdBuf));
                    memcpy(tempCmdBuff,xcCmdBuf,sizeof(xcCmdBuf));

                    memcpy(cmdName, strtok_r((char *)tempCmdBuff, ",", (char **)&pcmdStr), 32);
                    printf("\nInside the CLI huck block \n");

                    wfaCliFd=fopen("/etc/WfaEndpoint/wfa_cli.txt","r");
                    printf("\nAfter File open \n");
                    if(wfaCliFd!= NULL)
                        {
                            while(fgets(wfaCliBuff, 128, wfaCliFd) != NULL)
                                {
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
                            wfaCtrlSend(gCaSockfd, (BYTE *)respStr, strlen(respStr));
                            DPRINT_WARNING(WFA_WNG, "Command not valid, check the name\n");
                            continue;
                        }

                    memset(pcmdBuf, 0, WFA_BUFF_1K);
                    if(nameStr[i].cmdProcFunc(pcmdStr, pcmdBuf, &cmdLen)==WFA_FAILURE)
                        {
                            sleep(1);
                            sprintf(respStr, "status,INVALID\r\n");
                            wfaCtrlSend(gCaSockfd, (BYTE *)respStr, strlen(respStr));
                            DPRINT_WARNING(WFA_WNG, "Incorrect command syntax\n");
                            continue;
                        }

                    /*
                     * send to DUT.
                     */
                    if(send(gSock, pcmdBuf, cmdLen, 0) != cmdLen)
                        {
                            DPRINT_WARNING(WFA_WNG, "Incorrect sending ...\n");
                            continue;
                        }

                    DPRINT_INFO(WFA_OUT, "sent to DUT\n");
                } /* done with gCaSockfd */

            if(gSock > 0 && FD_ISSET(gSock, &sockSet))
                {
                    DPRINT_INFO(WFA_OUT, "received from DUT\n");
                    sleep(1);
                    memset(respStr, 0, WFA_BUFF_128);
                    memset(caCmdBuf, 0, WFA_BUFF_4K);
                    if ((bytesRcvd = recv(gSock, caCmdBuf, WFA_BUFF_4K, 0)) <= 0)
                        {
                            DPRINT_WARNING(WFA_WNG, "recv() failed or connection closed prematurely");
                            continue;
                        }

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
                        }
                    else
                        DPRINT_WARNING(WFA_WNG, "function not defined\n");
                } /* if(gCaSock */

        } /* for */

    close(gSock);
    exit(0);
}
