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
 * For MADWIFI driver, the TOS to 11E queue mapping as:
 *    0x08, 0x20            ----> WME_AC_BK;
 *    0x28, 0xa0            ----> WMC_AC_VI;
 *    0x30, 0xe0 0x88, 0xb8 ----> WME_AC_VO
 *      here 0x88 for UPSD, will be implemented later
 *    all other/default     ----> WME_AC_BE;
 */

#include "wfa_portall.h"
#include "wfa_stdincs.h"
#include "wfa_types.h"
#include "wfa_main.h"
#include "wfa_tg.h"
#include "wfa_debug.h"
#include "wfa_tlv.h"
#include "wfa_sock.h"
#include "wfa_rsp.h"
#include "wfa_wmmps.h"
#include "wfa_miscs.h"

/*
 * external global thread sync variables
 */
tgWMM_t wmm_thr[WFA_THREADS_NUM];
extern int resetsnd;
extern int resetrcv;
extern int newCmdOn;

extern tgStream_t *findStreamProfile(int id);
extern int gxcSockfd;
int vend;
extern int wfaSetProcPriority(int);
tgStream_t gStreams[WFA_MAX_TRAFFIC_STREAMS];
int tgSockfds[WFA_MAX_TRAFFIC_STREAMS] = {-1, -1, -1, -1, -1, -1, -1, -1};

extern unsigned short wfa_defined_debug;
extern unsigned int recvThr;
extern int tgWMMTestEnable;
int num_stops=0;
int num_hello=0;

BOOL gtgCaliRTD;

#ifdef WFA_WMM_PS_EXT
BOOL gtgWmmPS;
int psSockfd = -1;
extern int **ac_seq;
wfaWmmPS_t wmmps_info;
int msgsize=256;

extern void wfaSetDUTPwrMgmt(int mode);
extern void BUILD_APTS_MSG(int msg, unsigned long *txbuf);
extern void wmmps_wait_state_proc();

extern void mpx(char *m, void *buf_v, int len);

unsigned int psTxMsg[512];
unsigned int psRxMsg[512];
#endif /* WFA_WMM_PS_EXT */

extern void tmout_stop_send(int);
extern StationProcStatetbl_t stationProcStatetbl[LAST_TEST+1][11];

int nsent;

int runLoop = 0;
int usedThread=0;
BOOL gtgTransac = 0;
BOOL gtgSend = 0;
BOOL gtgRecv = 0;

extern int slotCnt;
extern int btSockfd;
int totalTranPkts=0, sentTranPkts=0;
BYTE *trafficBuf=NULL, *respBuf=NULL;

#ifdef WFA_VOICE_EXT
double gtgPktRTDelay = 0xFFFFFFFF;
double min_rttime = 0xFFFFFFFF;
static double rttime = 0;
#endif
int sendThrId = 0;

/* this is to stop sending packets by timer       */
void tmout_stop_send(int num)
{
    struct timeval af;
    int i =0;

    gettimeofday(&af,0);
    DPRINT_INFO(WFA_OUT, "timer fired, stop sending traffic, Exiting at sec %d usec %d\n", (int )af.tv_sec, (int)af.tv_usec);
    //DPRINT_INFO(WFA_OUT, "timer fired, stop sending traffic\n");

    /*
     *  After runLoop reset, all sendLong will stop
     */
    runLoop = 0;

    /*
     * once usedThread is reset, WMM tests using multithread is ended
     * the threads will be reused for the next test.
     */
    usedThread = 0;

    /*
     * once the stream table slot count is reset, it implies that the test
     * is done. When the next set profile command comes in, it will reset/clean
     * the stream table.
     */
    slotCnt = 0;

    /*
     * The test is for DT3 transaction test.
     * Timeout to stop it.
     */
    if(gtgTransac != 0)
    {
        gtgSend = 0;
        gtgRecv = 0;
        gtgTransac = 0;
    }

    /*
     * all WMM streams also stop
     */
    for(i=0; i<WFA_THREADS_NUM; i++)
    {
        wmm_thr[i].thr_flag = 0;
    }

    /* all alarms need to reset */
    wALARM(0);
}



/*
 * wfaTGSetPrio(): This depends on the network interface card.
 *               So you might want to remap according to the driver
 *               provided.
 *               The current implementation is to set the TOS/DSCP bits
 *               in the IP header
 */
int wfaTGSetPrio(int sockfd, int tgUserPriority)
{
    int tosval;

    socklen_t size = sizeof(tosval);
    wGETSOFD(sockfd, IPPROTO_IP, IP_TOS, &tosval, &size);

    switch(tgUserPriority)
    {
    case TG_WMM_AC_BK:   // user priority "1"
        /*Change this value to the ported device*/
        tosval = TOS_BK;
        break;

    case TG_WMM_AC_VI:   // user priority "5"
        /*Change this value to the ported device*/
        tosval = TOS_VI;
        break;

    case TG_WMM_AC_UAPSD:
        tosval = 0x88;
        break;

    case TG_WMM_AC_VO:   // user priority "6"
        /*Change this value to the ported device*/
        tosval = TOS_VO;
        break;

    case TG_WMM_AC_BE:   // user priority "0"
        tosval = TOS_BE;
        break;

    /* For WMM-AC Program User Priority Defintions */
    case TG_WMM_AC_UP0:
        tosval = 0x00;
        break;

    case TG_WMM_AC_UP1:
        tosval = 0x20;
        break;

    case TG_WMM_AC_UP2:
        tosval = 0x40;
        break;

    case TG_WMM_AC_UP3:
        tosval = 0x60;
        break;

    case TG_WMM_AC_UP4:
        tosval = 0x80;
        break;
    case TG_WMM_AC_UP5:
        tosval = 0xa0;
        break;

    case TG_WMM_AC_UP6:
        tosval = 0xc0;
        break;

    case TG_WMM_AC_UP7:
        tosval = 0xe0;
        break;

    default:
        tosval = 0x00;
        /* default */
        ;
    }

#ifdef WFA_WMM_PS_EXT
    psTxMsg[1] = tosval;
#endif

    if ( sockfd > 0)
    {
        if(wSETSOCKOPT ( sockfd, IPPROTO_IP, IP_TOS, &tosval, sizeof(tosval)) != 0)
        {
            DPRINT_ERR(WFA_ERR, "wfaTGSetPrio: Failed to set IP_TOS\n");
        }
    }
    else
    {
        DPRINT_INFO(WFA_OUT, "wfaTGSetPrio::socket closed\n");
    }
    return (tosval == 0xE0)?0xD8:tosval;
}

/*
 * wfaSetThreadPrio():
 *    Set thread priorities
 *    It is an optional experiment if you decide not necessary.
 */
void wfaSetThreadPrio(int tid, int userPriority)
{
    struct sched_param tschedParam;
    pthread_attr_t tattr;

    wPT_ATTR_INIT(&tattr);
    wPT_ATTR_SETSCH(&tattr, SCHED_RR);

    switch(userPriority)
    {
    case TG_WMM_AC_BK:
        tschedParam.sched_priority = -1;
        break;
    case TG_WMM_AC_VI:
        tschedParam.sched_priority = 19-1;
        break;
    case TG_WMM_AC_VO:
        tschedParam.sched_priority = 19;
        break;
    case TG_WMM_AC_BE:
        tschedParam.sched_priority = 0;
    default:
        /* default */
        ;
    }

    wPT_ATTR_SETSCHPARAM(&tattr, &tschedParam);
}

/*
 * collects the traffic statistics from other threads and
 * sends the collected information to CA
 */
void  wfaSentStatsResp(int sock, BYTE *buf)
{
    int i, total=0, pkLen;
    tgStream_t *allStreams = gStreams;
    dutCmdResponse_t *sendStatsResp = (dutCmdResponse_t *)buf, *first;
    char buff[WFA_RESP_BUF_SZ];

    if(sendStatsResp == NULL)
        return;

    first = sendStatsResp;

    for(i = 0; i < WFA_MAX_TRAFFIC_STREAMS; i++)
    {
        if((allStreams->id != 0) && (allStreams->profile.direction == DIRECT_SEND) && (allStreams->state == WFA_STREAM_ACTIVE))
        {
            sendStatsResp->status = STATUS_COMPLETE;
            sendStatsResp->streamId = allStreams->id;
            printf("stats stream id %i\n", allStreams->id);
            wMEMCPY(&sendStatsResp->cmdru.stats, &allStreams->stats, sizeof(tgStats_t));

            sendStatsResp++;
            total++;
        }
        allStreams->state = WFA_STREAM_INACTIVE;
        allStreams++;
    }

#if 1
    printf("%u %u %llu %llu\n", first->cmdru.stats.txFrames,
           first->cmdru.stats.rxFrames,
           first->cmdru.stats.txPayloadBytes,
           first->cmdru.stats.rxPayloadBytes);
#endif

    wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, total*sizeof(dutCmdResponse_t),
                 (BYTE *)first, (BYTE *)buff);

    pkLen = WFA_TLV_HDR_LEN + total*sizeof(dutCmdResponse_t);
    printf("pkLen %i\n", pkLen);

#if 0
    for(i = 0; i< pkLen; i++)
        printf("%x ", buff[i]);

    printf("\n");
#endif

    if(wfaCtrlSend(sock, (BYTE *)buff, pkLen) != pkLen)
    {
        DPRINT_WARNING(WFA_WNG, "wfaCtrlSend Error\n");
    }

    return;
}

#ifdef WFA_WMM_PS_EXT
/*
 * sender(): This is a generic function to send a packed for the given dsc
 *               (ac:VI/VO/BE/BK), before sending the packet the function
 *               puts the station into the PS mode indicated by psave and
 *               sends the packet after sleeping for sllep_period
 */
int sender(char psave,int sleep_period, int userPriority)
{
    int r;

    PRINTF("\nsender::sleeping for %d userPriority=%d psSockFd=%d",sleep_period, userPriority, psSockfd);
    wfaSetDUTPwrMgmt(psave);
    wUSLEEP(sleep_period);
    create_apts_msg(APTS_DEFAULT, psTxMsg,wmmps_info.my_sta_id);
    wfaTGSetPrio(psSockfd, userPriority);
    r = wSENDTO(psSockfd, psTxMsg, msgsize, 0, (struct sockaddr *)&wmmps_info.psToAddr, sizeof(struct sockaddr));
    return r;
}
/*
 * wfaStaSndHello(): This function sends a Hello packet
 *                and sleeps for sleep_period, the idea is
 *                to keep sending hello packets till the console
 *                responds, the function keeps a check on the MAX
 *                Hellos and if number of Hellos exceed that it quits
 */
int WfaStaSndHello(char psave,int sleep_period,int *state)
{
    tgWMM_t *my_wmm = &wmm_thr[wmmps_info.ps_thread];

    usleep(sleep_period);
    wfaSetDUTPwrMgmt(psave);
    if(!(num_hello++))
        create_apts_msg(APTS_HELLO, psTxMsg,0);
    wfaTGSetPrio(psSockfd, 0);
    wSENDTO(psSockfd, psTxMsg, msgsize, 0, (struct sockaddr *)&wmmps_info.psToAddr, sizeof(struct sockaddr));


    wPT_MUTEX_LOCK(&my_wmm->thr_flag_mutex);
    if(my_wmm->thr_flag)
    {
        (*state)++;
        num_hello=0;
        my_wmm->thr_flag=0;
    }

    wPT_MUTEX_UNLOCK(&my_wmm->thr_flag_mutex);
    if(num_hello > MAXHELLO)
    {
        DPRINT_ERR(WFA_ERR, "Too many Hellos sent\n");
        gtgWmmPS = 0;
        num_hello=0;
        if(psSockfd != -1)
        {
            wCLOSE(psSockfd);
            psSockfd = -1;
        }
        wSIGNAL(SIGALRM, SIG_IGN);
    }

    return 0;
}

/*
 * wfaStaSndConfirm(): This function sends the confirm packet
 *                which is sent after the console sends the
 *                test name to the station
 */
int WfaStaSndConfirm(char psave,int sleep_period,int *state)
{
    static int num_hello=0;
    wfaSetDUTPwrMgmt(psave);
    if(!num_hello)
        create_apts_msg(APTS_CONFIRM, psTxMsg,0);
    wSENDTO(psSockfd, psTxMsg, msgsize, 0, (struct sockaddr *)&wmmps_info.psToAddr, sizeof(struct sockaddr));
    mpx("STA msg",psTxMsg,64);
    printf("Confirm Sent\n");

    (*state)++;

    return 0;
}

/*
 * WfaStaSndVO(): This function sends a AC_VO packet
 *                after the time specified by sleep_period
 *                and advances to the next state for the given test case
 */
int WfaStaSndVO(char psave,int sleep_period,int *state)
{
    int r;
    static int en=1;
    PRINTF("\r\nEnterring WfastasndVO %d",en++);
    if ((r=sender(psave,sleep_period,TG_WMM_AC_VO))>=0)
        (*state)++;
    else
        PRINTF("\r\nError\n");

    return 0;
}

/*
 * WfaStaSnd2VO(): This function sends two AC_VO packets
 *                after the time specified by sleep_period
 *                and advances to the next state for the given test case
 */
int WfaStaSnd2VO(char psave,int sleep_period,int *state)
{
    int r;
    static int en=1;

    PRINTF("\r\nEnterring WfastasndVO %d",en++);
    if ((r=sender(psave,sleep_period,TG_WMM_AC_VO))>=0)
    {
        r = wSENDTO(psSockfd, psTxMsg, msgsize, 0, (struct sockaddr *)&wmmps_info.psToAddr, sizeof(struct sockaddr));
        mpx("STA msg",psTxMsg,64);
        (*state)++;
    }
    else
        PRINTF("\r\nError\n");

    return 0;
}

/*
 * WfaStaSndVI(): This function sends a AC_VI packet
 *                after the time specified by sleep_period
 *                and advances to the next state for the given test case
 */
int WfaStaSndVI(char psave,int sleep_period,int *state)
{
    int r;
    static int en=1;

    PRINTF("\r\nEnterring WfastasndVI %d",en++);
    if ((r=sender(psave,sleep_period,TG_WMM_AC_VI))>=0)
        (*state)++;

    return 0;
}

/*
 * WfaStaSndBE(): This function sends a AC_BE packet
 *                after the time specified by sleep_period
 *                and advances to the next state for the given test case
 */
int WfaStaSndBE(char psave,int sleep_period,int *state)
{
    int r;
    static int en=1;

    PRINTF("\r\nEnterring WfastasndBE %d",en++);
    if ((r=sender(psave,sleep_period,TG_WMM_AC_BE))>=0)
        (*state)++;

    return 0;
}
/*
 * WfaStaSndBK(): This function sends a AC_BK packet
 *                after the time specified by sleep_period
 *                and advances to the next state for the given test case
 */
int WfaStaSndBK(char psave,int sleep_period,int *state)
{
    int r;
    static int en=1;

    PRINTF("\r\nEnterring WfastasndBK %d",en++);
    if ((r=sender(psave,sleep_period,TG_WMM_AC_BK))>=0)
        (*state)++;

    return 0;
}

/*
 * WfaStaSndVOCyclic(): The function is the traffic generator for the L.1 test
 *                      caseThis function sends 3000 AC_VO packet
 *                      after the time specified by sleep_period (20ms)
 */
int WfaStaSndVOCyclic(char psave,int sleep_period,int *state)
{
    int i;
    static int en=1;

    for(i=0; i<3000; i++)
    {
        PRINTF("\r\nEnterring WfastasndVOCyclic %d",en++);
        sender(psave,sleep_period,TG_WMM_AC_VO);
        if(!(i%50))
        {
            PRINTF(".");
            fflush(stdout);
        }
    }

    (*state)++;

    return 0;
}

/*
 * WfaStaWaitStop(): This function sends the stop packet on behalf of the
 *                   station, the idea is to keep sending the Stop
 *                   till a stop is recd from the console,the functions
 *                   quits after stop threshold reaches.
 */
int WfaStaWaitStop(char psave,int sleep_period,int *state)
{
    int myid=wmmps_info.ps_thread;
    PRINTF("\n Entering Sendwait");
    wUSLEEP(sleep_period);
    if(!num_stops)
    {
        wfaSetDUTPwrMgmt(psave);
        wfaTGSetPrio(psSockfd, TG_WMM_AC_BE);
    }

    num_stops++;
    create_apts_msg(APTS_STOP, psTxMsg,wmmps_info.my_sta_id);
    wSENDTO(psSockfd, psTxMsg, msgsize, 0, (struct sockaddr *)&wmmps_info.psToAddr, sizeof(struct sockaddr));
    mpx("STA msg",psTxMsg,64);

    wmm_thr[myid].stop_flag = 1;
    wPT_MUTEX_LOCK(&wmm_thr[myid].thr_stop_mutex);
    wPT_COND_SIGNAL(&wmm_thr[myid].thr_stop_cond);
    wPT_MUTEX_UNLOCK(&wmm_thr[myid].thr_stop_mutex);

    if(num_stops > MAX_STOPS)
    {
        DPRINT_ERR(WFA_ERR, "Too many stops sent\n");
        gtgWmmPS = 0;
        if(psSockfd != -1)
        {
            wCLOSE(psSockfd);
            psSockfd = -1;
        }
        wSIGNAL(SIGALRM, SIG_IGN);
    }

    return 0;
}

#endif

void * wfa_wmm_thread(void *thr_param)
{
    int myId = ((tgThrData_t *)thr_param)->tid;
    tgWMM_t *my_wmm = &wmm_thr[myId];
    tgStream_t *myStream = NULL;
    int myStreamId, i=0,rttime=0,difftime=0, rcvCount=0,sendCount=0;
    int mySock = -1, status, respLen = 0, nbytes = 0, ret=0, j=0;
    tgProfile_t *myProfile;
    pthread_attr_t tattr;
#ifdef WFA_WMM_PS_EXT
    tgThrData_t *tdata =(tgThrData_t *) thr_param;
    StationProcStatetbl_t  curr_state;
#endif

//#ifdef WFA_VOICE_EXT
    struct timeval lstime, lrtime;
    int asn = 1;  /* everytime it starts from 1, and to ++ */
//#endif

    wPT_ATTR_INIT(&tattr);
    wPT_ATTR_SETSCH(&tattr, SCHED_RR);

    while(1)
    {
        int sleepTotal=0,sendFailCount=0;
        DPRINT_INFO(WFA_OUT, "wfa_wmm_thread::begin while loop for each send/rcv before mutex lock\n");
        pthread_mutex_lock(&my_wmm->thr_flag_mutex);
        /* it needs to reset the thr_flag to wait again */
        while(my_wmm->thr_flag == 0)
        {
            printf("Mutex wait\n");
            /*
             * in normal situation, the signal indicates the thr_flag changed to
             * a valid number (stream id), then it will go out the loop and do
             * work.
             */
            wPT_COND_WAIT(&my_wmm->thr_flag_cond, &my_wmm->thr_flag_mutex);
        }
        wPT_MUTEX_UNLOCK(&my_wmm->thr_flag_mutex);
        myStreamId = my_wmm->thr_flag;
        my_wmm->thr_flag=0;

        /* use the flag as a stream id to file the profile */
        myStream = findStreamProfile(myStreamId);
        myProfile = &myStream->profile;

        if(myProfile == NULL)
        {
            status = STATUS_INVALID;
            wfaEncodeTLV(WFA_TRAFFIC_AGENT_SEND_RESP_TLV, 4, (BYTE *)&status, respBuf);
            respLen = WFA_TLV_HDR_LEN+4;
            /*
             * send it back to control agent.
             */
            continue;
        }

        DPRINT_INFO(WFA_OUT, "wfa_wmm_thread::Mutex unlocked\n");
        switch(myProfile->direction)
        {
        case DIRECT_SEND:
            mySock = wfaCreateUDPSock(myProfile->sipaddr, myProfile->sport);
            if (mySock < 0)
            {
               DPRINT_INFO(WFA_OUT, "wfa_wmm_thread SEND ERROR failed create UDP socket! \n");
               break;
            }

            mySock = wfaConnectUDPPeer(mySock, myProfile->dipaddr, myProfile->dport);
            sendThrId = myId;
            /*
             * Set packet/socket priority TOS field
             */
            wfaTGSetPrio(mySock, myProfile->trafficClass);

            /*
             * set a proper priority
             */
            wfaSetThreadPrio(myId, myProfile->trafficClass);

            /* if delay is too long, it must be something wrong */
            if(myProfile->startdelay > 0 && myProfile->startdelay<100)
            {
                wSLEEP(myProfile->startdelay);
            }

            /*
             * set timer fire up
             */
            if(myProfile->maxcnt == 0)
            {
                wSIGNAL(SIGALRM, tmout_stop_send);
                wALARM(myProfile->duration );
                DPRINT_INFO(WFA_OUT, "wfa_wmm_thread SEND set stop alarm for %d sec \n", myProfile->duration);
            }

            if(myProfile->profile == PROF_MCAST)
            {
                wfaSetSockMcastSendOpt(mySock);
            }

            if (myProfile->profile == PROF_IPTV || myProfile->profile == PROF_FILE_TX || myProfile->profile == PROF_MCAST)
            {
                int iOptVal, iOptLen;

                getsockopt(mySock, SOL_SOCKET, SO_SNDBUF, (char *)&iOptVal, (socklen_t *)&iOptLen);
                iOptVal = iOptVal * 16;
                setsockopt(mySock, SOL_SOCKET, SO_SNDBUF, (char *)&iOptVal, (socklen_t )iOptLen);

              if ( (myProfile->rate != 0 ) /* WFA_SEND_FIX_BITRATE_MAX_FRAME_RATE)*/ && 
                   (myProfile->pksize * myProfile->rate * 8 < WFA_SEND_FIX_BITRATE_MAX) &&
                   (myProfile->trafficClass != TG_WMM_AC_VO)  )
                 wfaSendBitrateData(mySock, myStreamId, respBuf, &respLen);
              else
              {
                 wfaSendLongFile(mySock, myStreamId, respBuf, &respLen);
              }

              /* wfaSendLongFile(mySock, myStreamId, respBuf, &respLen); */
                if(mySock != -1)
                {
                    wCLOSE(mySock);
                    mySock = -1;
                }
            }
            else if(myProfile->profile == PROF_TRANSC || myProfile->profile == PROF_START_SYNC || myProfile->profile == PROF_CALI_RTD)
            {
#if 0
                struct timeval nxtime, curtime;
                int ioflags = wFCNTL(mySock, F_GETFL, 0);
#endif
                struct timeval tmout;

                gtgTransac = myStreamId;
                sentTranPkts = 0;

#if 0
                gettimeofday(&nxtime, NULL);
                nxtime.tv_usec += 20000;   /* fixed 20 min sec timeout */
                if(nxtime.tv_usec >= 1000000)
                {
                    nxtime.tv_sec +=1;
                    nxtime.tv_usec -= 1000000;
                }
                wFCNTL(mySock, F_SETFL, ioflags | O_NONBLOCK);
#endif
                gettimeofday(&lstime,0);
                DPRINT_INFO(WFA_OUT, "Start sending traffic,at sec %d usec %d\n", (int )lstime.tv_sec, (int)lstime.tv_usec);


                tmout.tv_sec = 0;
                tmout.tv_usec = 15000;     // set for 15000 microsec timeout for rcv              
                ret = setsockopt(mySock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tmout, (socklen_t) sizeof(tmout)); 
                
                rcvCount=0; sendFailCount=0;
                j=0;  sendCount=0;
                sleepTotal = 0;
                while(gtgTransac != 0)
                {
					gettimeofday(&lstime, NULL);
#ifdef WFA_VOICE_EXT  					
                    /*
                     * If your device is BIG ENDIAN, you need to
                     * modify the the function calls
                     */
                    int2BuffBigEndian(asn++, &((tgHeader_t *)trafficBuf)->hdr[8]);
                    int2BuffBigEndian(lstime.tv_sec, &((tgHeader_t *)trafficBuf)->hdr[12]);
                    int2BuffBigEndian(lstime.tv_usec, &((tgHeader_t *)trafficBuf)->hdr[16]);
#else
                    j++;
                    i=0;
                    do
                    {

#endif /* WFA_VOICE_EXT */

                        if(gtgTransac != 0/* && nbytes <= 0 */)
                        {
                            if(respBuf == NULL)
                            {
                                DPRINT_INFO(WFA_OUT, "wfa_wmm_thread SEND,PROF_TRANSC::a Null respBuf\n");
                            }
                            memset(respBuf, 0, WFA_RESP_BUF_SZ);
                            respLen = 0;
                            memset(trafficBuf  ,0, MAX_UDP_LEN + 1);
                            if(wfaSendShortFile(mySock, myStreamId,
                                trafficBuf, 0, respBuf, &respLen) == DONE)
                            {
                                if(wfaCtrlSend(gxcSockfd, respBuf, respLen) != respLen)
                                {
                                    DPRINT_INFO(WFA_OUT, "wfa_wmm_thread SEND,PROF_TRANSC::wfaCtrlSend Error for wfaSendShortFile\n");
                                }
                                sendFailCount++;
                                i--;
                                usleep(1000);
                            }
                            else
                            {
                                i++;
                                sendCount++;
                            }

                            //sentTranPkts++; /* send routine already incresed this counter  */

                            if((myProfile->maxcnt>0) &&(sendCount == myProfile->maxcnt))
                            {
                                DPRINT_INFO(WFA_OUT, "wfa_wmm_thread SEND,PROF_TRANSC::meet maxcnt=%d; end loop\n",myProfile->maxcnt);
                                gtgTransac = 0; /* break snd/rcv wile loop  */
                                break;
                            }

                            nbytes = wfaRecvFile(mySock, myStreamId, (char  *)trafficBuf);
                            if(nbytes <= 0)
                            {/* Do not print any msg it will slow down process on snd/rcv  */
                            //setsockopt(mySock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tmout, (socklen_t) sizeof(tmout)); 
                            //printf("PROF_TRANSC::time out event, wfaRecvFile failed,resend a new packet ...\n");

                            //tmout.tv_sec = 0;
                            //tmout.tv_usec = 3000;    // set for 20 minlsec timeout              
                            //setsockopt(mySock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tmout, (socklen_t) sizeof(tmout)); 
#if 0  /* if your socket APIs does not support "recvfrom" timeout, this is the way to loop the descriptor */
                        gettimeofday(&curtime, NULL);
                        if(((curtime.tv_sec - nxtime.tv_sec) * 1000000 + (curtime.tv_usec -  nxtime.tv_usec)) < 20000)
                        {
                            continue;
                        }

                        nxtime.tv_usec = curtime.tv_usec += 20000;
                        if(nxtime.tv_usec >= 1000000)
                        {
                            nxtime.tv_sec = curtime.tv_sec +=1;
                            nxtime.tv_usec -= 1000000;
                        }
#endif
                              //continue;
                            }
                            else
                            {
                               rcvCount++;
                               nbytes = 0;
                            }
                        } /*  if gtgTransac != 0 */
#ifdef WFA_VOICE_EXT 
                        /*
                        * Roundtrip time delay:
                        *   1. retrieve the timestamp
                        *   2. calculate the Trt(1) roundtrip delay
                        *   3. store the minimum Trt(1)
                        *   4. store Cdut(t1) and Ctm(2)
                        */
                        gettimeofday(&lrtime, NULL);

                    /* get a round trip time */
                    rttime = wfa_ftime_diff(&lstime, &lrtime);

                    if(min_rttime > rttime)
                    {

                        min_rttime = rttime;
                        if(gtgCaliRTD != 0)
                        {
                            gtgPktRTDelay = min_rttime;
                        }
                    }

                        if(gtgCaliRTD != 0 )
                        {
                            usleep(20000); /* wait a few min seconds before retrying the next calibration */
                        }
#else
                        /*  not voice case  */ 
                        /*  for do-while loop for frame rate per sec */ 
 
                    }while ((i <= myProfile->rate + myProfile->rate/3) && (myProfile->rate !=0) && (gtgTransac != 0 )); 

					if(myProfile->maxcnt == 0)
                    {
	                    gettimeofday(&lrtime, NULL);
	                    rttime = wfa_itime_diff(&lstime, &lrtime);
	                    /*  we cover frame rate = 0 case without any sleep to continue push data */
	                    if (((difftime = 1000000 - rttime) > 0) && (myProfile->rate != 0))
	                    {
	                        if ( j < myProfile->duration)
	                        {
	                           usleep (difftime);
	                           sleepTotal = sleepTotal + difftime/1000;
	                        }
	                    }
	                    if (j > myProfile->duration + 2)
	                    {	/* avoid infinite loop  */
	                        DPRINT_INFO(WFA_OUT, "wfa_wmm_thread SEND over time %d sec, stop sending\n",myProfile->duration);
	                        break;
	                    }
					}
#endif /* WFA_VOICE_EXT */
                } /* while */

                if(mySock != -1)
                {
                    wCLOSE(mySock);
                    mySock = -1;
                }
                DPRINT_INFO(WFA_OUT, "wfa_wmm_thread SEND::Sending stats back, sendCount=%d rcvCount=%d sleepTotal in mil-sec=%d sendFailCount=%d frmRate=%d do count=%d\n", sendCount,rcvCount,sleepTotal,sendFailCount, myProfile->rate, j);

            }/* else if(myProfile->profile == PROF_TRANSC || myProfile->profile == PROF_START_SYNC || myProfile->profile == PROF_CALI_RTD) */

            wMEMSET(respBuf, 0, WFA_RESP_BUF_SZ);
            wSLEEP(1);

            /*
             * uses thread that is saved at last to pack the items and ships
             * it to CA.
             */

            if(myId == sendThrId)
            {
                wfaSentStatsResp(gxcSockfd, respBuf);
                printf("done stats\n");
                sendThrId = 0;
            }

            break;

        case DIRECT_RECV:
            /*
             * Test WMM-PS
             */
            if(myProfile->profile == PROF_UAPSD)
            {
#ifdef WFA_WMM_PS_EXT /* legacy code not used now  */
                wmmps_info.sta_test = B_D;
                wmmps_info.ps_thread = myId;
                wmmps_info.rcv_state = 0;
                wmmps_info.tdata = tdata;
                wmmps_info.dscp = wfaTGSetPrio(psSockfd, TG_WMM_AC_BE);
                tdata->state_num=0;
                /*
                 * default timer value
                 */

                while(gtgWmmPS>0)
                {
                    if(resetsnd)
                    {
                        tdata->state_num = 0;
                        resetsnd = 0;
                    }
                    if (wmmps_info.sta_test > LAST_TEST)
                        break;

                    tdata->state =  stationProcStatetbl[wmmps_info.sta_test];
                    curr_state = tdata->state[tdata->state_num];
                    curr_state.statefunc(curr_state.pw_offon,curr_state.sleep_period,&(tdata->state_num));
                }
#endif /* WFA_WMM_PS_EXT */
            }
            else if (myProfile->profile == PROF_IPTV || myProfile->profile == PROF_FILE_TX || myProfile->profile == PROF_MCAST)
            {
                char recvBuf[MAX_RCV_BUF_LEN+1];
                int iOptVal, iOptLen;
                struct timeval tmout;

#ifdef WFA_VOICE_EXT
                struct timeval currtime;
                FILE *e2eoutp = NULL;
                char e2eResults[124];
                int le2eCnt = 0;
#endif

                mySock = wfaCreateUDPSock(myProfile->dipaddr, myProfile->dport);
                if(mySock == -1)
                {
                    printf("Error open socket\n");
                    continue;
                }

                if (myProfile->profile == PROF_MCAST)
                {
                    int so = wfaSetSockMcastRecvOpt(mySock, myProfile->dipaddr);
                    if(so < 0)
                    {
                        DPRINT_ERR(WFA_ERR, "Join the multicast group failed\n");
                        wCLOSE(mySock);
                        continue;
                    }
                }

                tgSockfds[myStream->tblidx] = mySock;

#ifdef WFA_VOICE_EXT
                /* only for receive stream needs to create a stats storage */
                tgE2EStats_t *e2esp = NULL;
                int totalE2Cnt = 220 * WFA_G_CODEC_RATE;
                printf("init E2Cnt %i\n", totalE2Cnt);
                if(myProfile->profile == PROF_IPTV)
                {
                    e2esp = malloc(totalE2Cnt * sizeof(tgE2EStats_t));

                    if(e2esp == NULL)
                    {

                    }
                }
#endif

                /* increase the rec queue size */
                getsockopt(mySock, SOL_SOCKET, SO_RCVBUF, (char *)&iOptVal, (socklen_t *)&iOptLen);
                iOptVal = iOptVal * 10;
                setsockopt(mySock, SOL_SOCKET, SO_RCVBUF, (char *)&iOptVal, (socklen_t )iOptLen);

                /* set timeout for blocking receive */
                tmout.tv_sec = 0;
                tmout.tv_usec = 200000;   /* set the receive time out to 200 ms */
                setsockopt(mySock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tmout, (socklen_t) sizeof(tmout));

                wfaSetThreadPrio(myId, TG_WMM_AC_VO);   /* try to raise the receiver higher priority than sender */
                for(;;)
                {
                    nbytes = wfaRecvFile(mySock, myStreamId, (char *)recvBuf);
                    if(nbytes <= 0)
                    {
                        /* due to timeout */
                        if(tgSockfds[myStream->tblidx] >=0 )
                            continue;

                        break;
                    }

#ifdef WFA_VOICE_EXT
                    if(myProfile->profile == PROF_IPTV)
                    {
                        struct timeval ttval, currTimeVal;

                        int sn = bigEndianBuff2Int(&((tgHeader_t *)recvBuf)->hdr[8]);
                        ttval.tv_sec = bigEndianBuff2Int(&((tgHeader_t *)recvBuf)->hdr[12]);
                        ttval.tv_usec = bigEndianBuff2Int(&((tgHeader_t *)recvBuf)->hdr[16]);
                        gettimeofday(&currTimeVal, NULL);

                        /*
                         * take the end2end stats, limit to the max voice pkt number
                         */
                        if(le2eCnt < totalE2Cnt)
                        {
                            tgE2EStats_t *ep = e2esp + le2eCnt++;
                            ep->seqnum = sn;
                            ep->rsec = ttval.tv_sec;
                            ep->rusec = ttval.tv_usec;

                            ep->lsec = currTimeVal.tv_sec;
                            ep->lusec = currTimeVal.tv_usec;

                            if(ep->lusec  < 0)
                            {
                                ep->lsec -=1;
                                ep->lusec += 1000000;
                            }
                            else if(ep->lusec >= 1000000)
                            {
                                ep->lsec += 1;
                                ep->lusec -= 1000000;
                            }
                        }
                    }
#endif /* WFA_VOICE_EXT */
                    wfaSetThreadPrio(myId, TG_WMM_AC_BE); /* put it back down */
                } /* while */

                my_wmm->thr_flag = 0;

#ifdef WFA_VOICE_EXT
                if(myProfile->profile == PROF_IPTV)
                {
                    int j;

                    wGETTIMEOFDAY(&currtime, NULL);
                    sprintf(e2eResults, "/tmp/e2e%u-%i.txt", (unsigned int) currtime.tv_sec, myStreamId);
                    printf("file %s cnt %i\n", e2eResults, le2eCnt);
                    e2eoutp = fopen(e2eResults, "w+");
                    if(e2eoutp != NULL)
                    {
                        fprintf(e2eoutp, "roundtrip delay: %i\n", (int) (1000000*gtgPktRTDelay));
                        for(j = 0; j< le2eCnt; j++)
                        {
                            tgE2EStats_t *ep = e2esp+j;
                            fprintf(e2eoutp, "%i:%i:%i:%i:%i\n", ep->seqnum, ep->lsec, ep->lusec, ep->rsec, ep->rusec);
                        }
                        fclose(e2eoutp);
                    }

                    if(e2esp != NULL)
                        free(e2esp);
                }
#endif
            }
            else if(myProfile->profile == PROF_TRANSC || myProfile->profile == PROF_START_SYNC || myProfile->profile == PROF_CALI_RTD)
            {
                struct timeval tmout;

                mySock = wfaCreateUDPSock(myProfile->sipaddr, myProfile->sport);
                if(mySock < 0)
                {
                    /* return error */
                    my_wmm->thr_flag = 0;
                    continue;
                }

                tgSockfds[myStream->tblidx] = mySock;

                totalTranPkts = 0xFFFFFFF0;
                gtgTransac = myStreamId;

               /* set timeout for blocking receive */
               tmout.tv_sec = 0;
               tmout.tv_usec = 400000;   /* set the receive time out to 400 ms, 200ms is too short */
               setsockopt(mySock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tmout, (socklen_t) sizeof(tmout));

               while(gtgTransac != 0)
               {
                    memset(trafficBuf, 0, sizeof((char*)trafficBuf));

                    if(mySock != -1)
                    {
                        int i = gtgTransac;

                      nbytes = 0;

                      /* check for data as long as we are in a transaction */
                      while ((gtgTransac != 0) && (nbytes <= 0))
                      {
                          nbytes = wfaRecvFile(mySock, i, (char  *)trafficBuf);
                      }
                      /* It is the end of a transaction, go out of the loop */
                      if (gtgTransac == 0) break;
                   }
                    else
                    {
                        break;
                    }

#ifdef WFA_VOICE_EXT
                    /* for a transaction receiver, it just needs to send the local time back */
                    gettimeofday(&lstime, NULL);
                    int2BuffBigEndian(lstime.tv_sec, &((tgHeader_t *)trafficBuf)->hdr[12]);
                    int2BuffBigEndian(lstime.tv_usec, &((tgHeader_t *)trafficBuf)->hdr[16]);
#endif
                    memset(respBuf, 0, WFA_RESP_BUF_SZ);
                    respLen = 0;
                    if(wfaSendShortFile(mySock, gtgTransac, trafficBuf, nbytes, respBuf, &respLen) == DONE)
                    {
                        if(wfaCtrlSend(gxcSockfd, (BYTE *)respBuf, respLen)!=respLen)
                        {
                            DPRINT_WARNING(WFA_WNG, "wfaCtrlSend Error\n");
                        }
                    }
                }

                my_wmm->thr_flag = 0;
               //////////////////// Wifi Alliance Added
               if(mySock != -1)
               {
                   wCLOSE(mySock);
                   mySock = -1;
               }
               //////////////////// Wifi Alliance Added
           }
            break;

        default:
            DPRINT_ERR(WFA_ERR, "Unknown covered case\n");
        }

    }
}
