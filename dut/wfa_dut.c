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
 * File: wfa_dut.c - The main program for DUT agent.
 *       This is the top level of traffic control. It initializes a local TCP
 *       socket for command and control link and waits for a connect request
 *       from a Control Agent. Once the the connection is established, it 
 *       will process the commands from the Control Agent. For details, please
 *       reference the architecture documents.   
 *
 */

#include <pthread.h>
#include <signal.h>
#include <time.h>

#include "wfa_portall.h"
#include "wfa_debug.h"
#include "wfa_main.h"
#include "wfa_types.h"
#include "wfa_dut.h"
#include "wfa_sock.h"
#include "wfa_tlv.h"
#include "wfa_tg.h"
#include "wfa_miscs.h"
#include "wfa_agt.h"
#include "wfa_rsp.h"
#include "wfa_wmmps.h"

/* Global flags for synchronizing the TG functions */
int        gtimeOut = 0;        /* timeout value for select call in usec */

#ifdef WFA_WMM_PS_EXT
extern BOOL gtgWmmPS;
extern unsigned long psTxMsg[512];
extern unsigned long psRxMsg[512];
extern wfaWmmPS_t wmmps_info;
extern tgWMM_t    wmmps_mutex_info;
extern int  psSockfd;
extern struct apts_msg *apts_msgs;

extern void BUILD_APTS_MSG(int msg, unsigned long *txbuf);
extern int wfaWmmPowerSaveProcess(int sockfd);
extern void wfaSetDUTPwrMgmt(int);
extern void wfaTGSetPrio(int, int);
#endif /* WFA_WMM_PS_EXT */

extern     int adj_latency;           /* adjust sleep time due to latency */
char       gnetIf[WFA_BUFF_32];        /* specify the interface to use */

extern BYTE   *trafficBuf, *respBuf;

/* stream table */
extern tgStream_t gStreams[];         /* streams' buffers             */ 

/* the agent local Socket, Agent Control socket and baseline test socket*/
int   gagtSockfd = -1;
extern int btSockfd;


/* the WMM traffic streams socket fds - Socket Handler table */
extern int tgSockfds[];

extern     xcCommandFuncPtr gWfaCmdFuncTbl[]; /* command process functions */
extern     char gCmdStr[];
extern     tgStream_t *findStreamProfile(int);
extern     int clock_drift_ps;

dutCmdResponse_t gGenericResp;

/* Debug message flags */
unsigned short wfa_defined_debug = WFA_DEBUG_ERR | WFA_DEBUG_WARNING | WFA_DEBUG_INFO;
unsigned short dfd_lvl = WFA_DEBUG_DEFAULT | WFA_DEBUG_ERR | WFA_DEBUG_INFO;

/*
 * Thread Synchronize flags
 */
tgWMM_t wmm_thr[WFA_THREADS_NUM];

extern void *wfa_wmm_thread(void *thr_param);
extern void *wfa_wmmps_thread();

extern double gtgPktRTDelay;

int gxcSockfd = -1;

#define DEBUG 0

extern int wfa_estimate_timer_latency();
extern void wfa_dut_init(BYTE **tBuf, BYTE **rBuf, BYTE **paBuf, BYTE **cBuf, struct timeval **timerp);

int
main(int argc, char **argv)
{
    int	      nfds, maxfdn1 = -1, nbytes = 0, cmdLen = 0, isExit = 1;
    int       respLen;
    WORD      locPortNo = 0;   /* local control port number                  */
    fd_set    sockSet;         /* Set of socket descriptors for select()     */
    BYTE      *xcCmdBuf=NULL, *parmsVal=NULL;
    struct timeval *toutvalp=NULL, *tovalp; /* Timeout for select()           */
    WORD      xcCmdTag;
    struct sockfds fds;

    tgThrData_t tdata[WFA_THREADS_NUM];
    int i = 0;
    pthread_attr_t ptAttr;
    int ptPolicy;
    
    struct sched_param ptSchedParam;
   
    if (argc < 3)              /* Test for correct number of arguments */
    {
        DPRINT_ERR(WFA_ERR, "Usage:  %s <command interface> <Local Control Port> \n", argv[0]);
        exit(1);
    }
#ifdef WFA_PC_CONSOLE
    else if(argc > 3)
    {
        FILE *logfile;
        int fd;
        logfile = fopen(argv[3],"a");
        if(logfile != NULL)
        {
            fd = fileno(logfile);
            DPRINT_INFO(WFA_OUT,"redirecting the output to %s\n",argv[3]);
            dup2(fd,1);
            dup2(fd,2);
        }
        else
        {
            DPRINT_ERR(WFA_ERR, "Cant open the log file continuing without redirecting\n");
        }
        printf("Output starts\n");
    }
#endif

    if(isString(argv[1]) == WFA_FAILURE)
    {
        DPRINT_ERR(WFA_ERR, "incorrect network interface\n");
        exit(1);
    }

    strncpy(gnetIf, argv[1], 31);

    if(isNumber(argv[2]) == WFA_FAILURE)
    {
        DPRINT_ERR(WFA_ERR, "incorrect port number\n");
        exit(1);
    }

    locPortNo = atoi(argv[2]);

    adj_latency = wfa_estimate_timer_latency() + 4000; /* four more mini */

    if(adj_latency > 500000)
    {
        printf("****************** WARNING  **********************\n");
        printf("!!!THE SLEEP TIMER LATENCY IS TOO HIGH!!!!!!!!!!!!\n");
        printf("**************************************************\n");

        /* Just set it to  500 mini seconds */
        adj_latency = 500000;
    }

    /* allocate the traffic stream table */
    wfa_dut_init(&trafficBuf, &respBuf, &parmsVal, &xcCmdBuf, &toutvalp);

    /* 4create listening TCP socket */
    gagtSockfd = wfaCreateTCPServSock(locPortNo);
    if(gagtSockfd == -1)
    {
       DPRINT_ERR(WFA_ERR, "Failed to open socket\n");
       exit(1);
    }

    pthread_attr_init(&ptAttr);

    ptSchedParam.sched_priority = 10;
    pthread_attr_setschedparam(&ptAttr, &ptSchedParam);
    pthread_attr_getschedpolicy(&ptAttr, &ptPolicy);
    pthread_attr_setschedpolicy(&ptAttr, SCHED_RR);
    pthread_attr_getschedpolicy(&ptAttr, &ptPolicy);

    /*
     * Create multiple threads for WMM Stream processing.
     */
    for(i = 0; i< WFA_THREADS_NUM; i++)
    {
        tdata[i].tid = i;
        pthread_mutex_init(&wmm_thr[i].thr_flag_mutex, NULL);
        pthread_cond_init(&wmm_thr[i].thr_flag_cond, NULL);
        wmm_thr[i].thr_id = pthread_create(&wmm_thr[i].thr, 
                       &ptAttr, wfa_wmm_thread, &tdata[i]);
    }

    for(i = 0; i < WFA_MAX_TRAFFIC_STREAMS; i++)
       tgSockfds[i] = -1;

#ifdef WFA_WMM_PS_EXT
    /* WMMPS thread   */  
	int ret = 0;
    ret = pthread_mutex_init(&wmmps_mutex_info.thr_flag_mutex,NULL);
	if ( ret !=0)
	{
		DPRINT_INFO(WFA_OUT, "WMMPS pthread_mutex_init faile\n");
	}
    ret = pthread_cond_init(&wmmps_mutex_info.thr_flag_cond,NULL);
	if (ret != 0)
	{
		DPRINT_INFO(WFA_OUT, "WMMPS pthread_cond_init faile\n");
	}
    wmmps_mutex_info.thr_id=pthread_create(&wmmps_mutex_info.thr,NULL /*&ptAttr*/,wfa_wmmps_thread,(void*)&wmmps_mutex_info.thr_id);// calls up the wmmps-thread here
#endif

    maxfdn1 = gagtSockfd + 1;
    while (isExit) 
    {
        fds.agtfd = &gagtSockfd;
        fds.cafd = &gxcSockfd;
        fds.tgfd = &btSockfd; 
        fds.wmmfds = tgSockfds; 
#ifdef WFA_WMM_PS_EXT
        fds.psfd = &psSockfd;
#endif

        wfaSetSockFiDesc(&sockSet, &maxfdn1, &fds);

        /* 
         * The timer will be set for transaction traffic if no echo is back
         * The timeout from the select call force to send a new packet
         */
        tovalp = NULL;
        if(gtimeOut != 0)
        {
          /* timeout is set to usec */
          tovalp = wfaSetTimer(0, gtimeOut*1000, toutvalp);
        }

        nfds = 0;
        if ( (nfds = select(maxfdn1, &sockSet, NULL, NULL, tovalp)) < 0) 
        {
            if (errno == EINTR)
                continue;           /* back to for() */
	    else
                DPRINT_WARNING(WFA_WNG, "Warning: select()-%i", errno);
        }

        if(nfds == 0)
        {
#if 0 //def WFA_WMM_PS_EXT
            /*
             * For WMM-Power Save
             * periodically send HELLO to Console for initial setup.
             */
            if(gtgWmmPS != 0 && psSockfd != -1)
            {
                wfaSetDUTPwrMgmt(0);
                wfaTGSetPrio(psSockfd, 0);
                BUILD_APTS_MSG(APTS_HELLO, psTxMsg);
                wfaTrafficSendTo(psSockfd, (char *)psTxMsg, sizeof(psTxMsg), (struct sockaddr *) &wmmps_info.psToAddr);

                wmmps_info.sta_state = 0;
                wmmps_info.wait_state = WFA_WAIT_STAUT_00;
                continue;
            }
#endif /* WFA_WMM_PS_EXT */
        }

        if (FD_ISSET(gagtSockfd, &sockSet)) 
        {
            /* Incoming connection request */
            gxcSockfd = wfaAcceptTCPConn(gagtSockfd);
            if(gxcSockfd == -1)
            {
               DPRINT_ERR(WFA_ERR, "Failed to open control link socket\n");
               exit(1);
            }
        }

        /* Control Link port event*/
        if(gxcSockfd >= 0 && FD_ISSET(gxcSockfd, &sockSet)) 
        {
            memset(xcCmdBuf, 0, WFA_BUFF_1K);  /* reset the buffer */
            nbytes = wfaCtrlRecv(gxcSockfd, xcCmdBuf);

            if(nbytes <=0)
            {
                /* errors at the port, close it */
                shutdown(gxcSockfd, SHUT_WR);
                close(gxcSockfd);
                gxcSockfd = -1;
            }
            else
            {
               /* command received */
               wfaDecodeTLV(xcCmdBuf, nbytes, &xcCmdTag, &cmdLen, parmsVal);    
               memset(respBuf, 0, WFA_RESP_BUF_SZ); 
               respLen = 0;

               /* reset two commond storages used by control functions */
               memset(gCmdStr, 0, WFA_CMD_STR_SZ);
               memset(&gGenericResp, 0, sizeof(dutCmdResponse_t));

               /* command process function defined in wfa_ca.c and wfa_tg.c */
               if(xcCmdTag != 0 && gWfaCmdFuncTbl[xcCmdTag] != NULL)
               {
	           /* since the new commands are expanded to new block */
		           gWfaCmdFuncTbl[xcCmdTag](cmdLen, parmsVal, &respLen, (BYTE *)respBuf);
               }
               else
               {       // no command defined
		          gWfaCmdFuncTbl[0](cmdLen, parmsVal, &respLen, (BYTE *)respBuf);
               }

               // gWfaCmdFuncTbl[xcCmdTag](cmdLen, parmsVal, &respLen, (BYTE *)respBuf);
               if(gxcSockfd != -1)
               if(wfaCtrlSend(gxcSockfd, (BYTE *)respBuf, respLen) != respLen)
               {
                   DPRINT_WARNING(WFA_WNG, "wfa-wfaCtrlSend Error\n");
               }
            }

        }

#if 0 // def WFA_WMM_PS_EXT
        /*
         * Check if there is from Console
         */
        if(psSockfd != -1 && FD_ISSET(psSockfd, &sockSet))
        {
            wfaWmmPowerSaveProcess(psSockfd);
            continue;
        }
#endif /* WFA_WMM_PS_EXT */

    }

    /*
     * necessarily free all mallocs for flat memory real-time systems
     */
    wFREE(trafficBuf);
    wFREE(toutvalp);
    wFREE(respBuf);
    wFREE(xcCmdBuf);
    wFREE(parmsVal);

     /* Close sockets */
    wCLOSE(gagtSockfd);
    wCLOSE(gxcSockfd);
    wCLOSE(btSockfd);

    for(i= 0; i< WFA_MAX_TRAFFIC_STREAMS; i++)
    {
       if(tgSockfds[i] != -1)
       {
         wCLOSE(tgSockfds[i]); 
         tgSockfds[i] = -1;
       }
    }

    return 0;
}
