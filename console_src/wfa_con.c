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
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <rpc/rpc.h>
#include <linux/ip.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "wfa_con.h"
unsigned int rmsg[512];         // rx msg buffer
unsigned int txmsg[512];        // tx msg buffer

struct itimerval waitval_state = { 40,0};
#define PORT    12345           // port for sending/receiving
#define MAXRETRY    3           // port for sending/receiving
#define SLEEP_PERIOD 10         //time to exit itself
#define INTERSTEP_PERIOD 5      //time allowed within steps
int port         = PORT;
int reset        = 0;
int tout         = 0;
int reset_recd   = 0;
int num_retry    = 0;
int tos_vo,tos_vi,tos_be,tos_bk;
int timeron=SLEEP_PERIOD;
int exitflag=0;
pthread_t time_thr;

struct apts_msg apts_msgs[] =
{
    {0, -1},
    {"B.D", B_D},
    {"B.H", B_H},
    {"B.B", B_B},
    {"B.M", B_M},
    {"M.D", M_D},
    {"B.Z", B_Z},
    {"M.Y", M_Y},
    {"L.1", L_1},
    {"A.Y", A_Y},
    {"B.W", B_W},
    {"A.J", A_J},
    {"M.V", M_V},
    {"M.U", M_U},
    {"A.U", A_U},
    {"M.L", M_L},
    {"B.K", B_K},
    {"M.B", M_B},
    {"M.K", M_K},
    {"M.W", M_W},
    {"APTS TX         ", APTS_DEFAULT },
    {"APTS Hello      ", APTS_HELLO },
    {"APTS Broadcast  ", APTS_BCST },
    {"APTS Confirm    ", APTS_CONFIRM},
    {"APTS STOP       ", APTS_STOP},
    {"APTS CK BE      ", APTS_CK_BE },
    {"APTS CK BK      ", APTS_CK_BK },
    {"APTS CK VI      ", APTS_CK_VI },
    {"APTS CK VO      ", APTS_CK_VO },
    {"APTS RESET      ", APTS_RESET },
    {"APTS RESET RESP ", APTS_RESET_RESP },
    {"APTS RESET STOP ", APTS_RESET_STOP },
    {0, 0 }     // APTS_LAST
};
struct station;
int WfaConRcvHello(struct station *,unsigned int *,int );
int WfaConRcvConf(struct station *,unsigned int *,int );
int WfaConRcvVOSnd(struct station *,unsigned int *,int );
int WfaConRcvVOSndCyclic(struct station *,unsigned int *,int );
int WfaConRcvVOSndE(struct station *,unsigned int *,int );
int WfaConRcvVOE(struct station *,unsigned int *,int );
int WfaConWaitStop(struct station *,unsigned int *,int );
int WfaConRcvVOSnd2VO(struct station *,unsigned int *,int );
int WfaConRcvVO(struct station *,unsigned int *,int );
int WfaConRcvVI(struct station *,unsigned int *,int );
int WfaConRcvBE(struct station *,unsigned int *,int );
int WfaConRcvBKE(struct station *,unsigned int *,int );
int WfaConRcvVIE(struct station *,unsigned int *,int );
int WfaConRcvVISndBE(struct station *,unsigned int *,int );
int WfaConRcvVISndBK(struct station *,unsigned int *,int );
int WfaConRcvVISnd(struct station *,unsigned int *,int );
int WfaConRcvVISndE(struct station *,unsigned int *,int );
int WfaConRcvVISndVOE(struct station *,unsigned int *,int );
int WfaConRcvConfSndVI(struct station *,unsigned int *,int );
int WfaConRcvVIOnlySndBcastE(struct station *,unsigned int *,int );
int WfaConRcvVOSndBcastE(struct station *,unsigned int *,int );
int WfaConRcvBESnd(struct station *,unsigned int *,int );
int WfaConRcvBESndBcastE(struct station *,unsigned int *,int );
int WfaConRcvVISndBcast (struct station *,unsigned int *,int );
int WfaConRcvBESndBcast (struct station *,unsigned int *,int );
int WfaConRcvVISndBcastE (struct station *,unsigned int *,int );
int WfaConRcvVOSndAllE(struct station *,unsigned int *,int );
int WfaConRcvVISndVIE(struct station *,unsigned int *,int );
int WfaConRcvVOSndVOE(struct station *,unsigned int *,int );
int WfaConRcvVOSndVO(struct station *,unsigned int *,int );
int WfaConRcvBESndE(struct station *,unsigned int *,int );

char traceflag=1;   // enable debug packet tracing

consoleProcStatetbl_t consoleProcStatetbl[LAST_TEST+1][10] =
{
    /* Ini*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVOSnd,WfaConWaitStop,0,0,0,0,0,0},
    /* B.D*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVOSnd,WfaConRcvVOE,WfaConWaitStop,WfaConWaitStop ,0,0,0,0},
    /* B.H*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVOSnd2VO,WfaConRcvVOE,WfaConWaitStop      ,WfaConWaitStop      ,0,0,0,0},
    /* B.B*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVO   ,WfaConRcvVI               ,WfaConRcvBE         ,WfaConRcvBKE        ,WfaConWaitStop,0,0,0},
    /* B.M*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVIE   ,WfaConWaitStop            ,0,0,0,0,0,0},

    /* M.D */
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVISndBE    ,WfaConRcvVISndBK ,WfaConRcvVISnd      ,WfaConRcvVISndVOE    ,WfaConWaitStop      ,0,0,0},
    /* B.Z*/
    {WfaConRcvHello,WfaConRcvConfSndVI,WfaConRcvVOSndBcastE,WfaConWaitStop      ,0,0,0,0},
    /* M.Y*/{WfaConRcvHello,WfaConRcvConf,WfaConRcvVISnd,WfaConRcvVO,WfaConRcvBESnd,WfaConRcvBESndBcastE,WfaConWaitStop,0,0,0},
    /* L.1*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVOSndCyclic, WfaConWaitStop      ,0,0,0,0,0,0},
    /* A.Y*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVISnd    ,WfaConRcvVO                 ,WfaConRcvBESnd        ,WfaConRcvBE         ,WfaConRcvBESndBcastE,WfaConWaitStop,0,0},
    /* B.W*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVISndBcast ,WfaConRcvVISndBcast       ,WfaConRcvVIE,WfaConWaitStop      ,0,0,0,0},
    /* A.J*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVO ,WfaConRcvVOSndAllE    ,WfaConWaitStop      ,0,0,0,0,0},

    /* M.V*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVISnd    ,WfaConRcvBESnd  ,WfaConRcvVISndE   ,WfaConWaitStop      ,0,0,0,0},
    /* M.U*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVISnd    ,WfaConRcvBESnd  ,WfaConRcvVOSnd    ,WfaConRcvVOSndE   ,WfaConWaitStop      ,0,0,0},
    /* A.U*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVISnd    ,WfaConRcvBE    ,WfaConRcvBESnd    ,WfaConRcvBE         ,WfaConRcvVOSnd    ,WfaConRcvVOE,WfaConWaitStop,0},
    /* M.L*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvBESndE   ,WfaConWaitStop ,0,0,0,0,0,0},

    /* B.K*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVISnd    ,WfaConRcvBESnd   ,WfaConRcvVIE    ,WfaConWaitStop      ,0,0,0,0},
    /* M.B*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVO       ,WfaConRcvVI     ,WfaConRcvBE         ,WfaConRcvBKE        ,WfaConWaitStop      ,0,0,0},
    /* M.K*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVISnd    ,WfaConRcvBESnd ,WfaConRcvVIE   ,WfaConWaitStop      ,0,0,0,0},
    /* M.W*/
    {WfaConRcvHello,WfaConRcvConf,WfaConRcvVISndBcast ,WfaConRcvBESndBcast,WfaConRcvVIE,WfaConWaitStop      ,0,0,0,0}
};

unsigned int fromlen;           // sizeof socket struct
unsigned char dscp, ldscp;      // new/last dscp output values
struct sockaddr dst;            // sock declarations
struct sockaddr_in target;
struct sockaddr_in from;
struct sockaddr_in local;
int sockflags;                  // socket call flag

int nsta=0;                     // Number of stations
struct station stations[NSTA];
char *procname;                 // dst system name or ip address
int sd,rd;                      // socket descriptor
void WfaConResetAll();

void IAmDead()
{
    printf("Time to Die...\n");
    exit(-10);
}
void WfaConResetAll()
{
    int r;
    reset=1;
    printf("\nEntering WfaConResetAll:: ");
    alarm(0);
    num_retry++;
    if(num_retry > MAXRETRY)
        IAmDead();
    if(reset_recd)
    {
        reset_recd = 0;
        set_dscp(tos_be);
        create_apts_msg(APTS_RESET_RESP, txmsg,0);
        txmsg[1] = tos_be;
        r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        if (traceflag) mpx("CMD send\n", txmsg, 64);
        printf("\nsent RESET RESP\n");
    }
    else
    {

        int resp_recd=0;
        create_apts_msg(APTS_RESET, txmsg,0);
        txmsg[1] = tos_be;
        r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        if (traceflag) mpx("CMD send\n", txmsg, 64);
        printf("\nsent RESET \n");
        while(!resp_recd)
        {
            r = recvfrom(sd, rmsg, sizeof(rmsg), 0, (struct sockaddr *)&from, &fromlen);
            if (r<0)
            {
                perror("rcv error:");
                exit(1);
            }
            if(rmsg[10] != APTS_RESET_RESP)
                continue;
            if (traceflag) mpx("CMD recd\n", rmsg, 64);
            alarm(0);
            resp_recd=1;
        }/* while  */
    }
}

void resettimer(int period)
{
    timeron=period;
}

void* timerthread(void* period)
{
    int* per=(int*)period;
    exitflag=1;
    while(1)
    {
        sleep(1);
        timeron--;
        if(!timeron && exitflag)
        {
            printf("time out\n");
            exit(1);
        }
    }
}
main(int argc, char **argv)
{
    int r, flags=0, n, i, id,base=10, bKeepFrom = 0;
    struct apts_msg *testcase;
    struct station *sta = NULL;
    char IP[INET_ADDRSTRLEN],string[128];
    char *str,*endptr;
    FILE *tmpfile;
    consoleProcStatetbl_t func;

    for(i=1; i<argc; i++)
    {
        if(argv[i][0]=='-')
        {
            if(argv[i][1]=='t')
            {
                i++;
                timeron=atoi(argv[i]);
                pthread_create(&time_thr,NULL,timerthread,NULL);
            }
            else
            {
                printf("Unrecognized options\n");
                exit(1);
            }
        }
        else
        {
            procname = argv[i]; // gather non-option args here
            if( (strncmp(procname,"L.1AP", 4) == 0 ))
            {
                procname[3] = '\0';
                bKeepFrom = 1;

            }
            else
            {
                bKeepFrom = 0;
            }
        }
    }
    setup_socket();
    testcase = (struct apts_msg *) apts_lookup(procname);
    sockflags = MSG_DONTWAIT;
    tos_vo = 0xD0;
    tos_vi = 0xA0;
    tos_be = 0x00;
    tos_bk = 0x20;
    tmpfile = fopen("tos.txt","r+");
    if(tmpfile == NULL)
    {
        printf("Can not find the tos file,proceeding with default values\n");
    }
    else
    {
        while(fgets(string,128,tmpfile) != NULL)
        {
            if(strstr(string,"#"))
                continue;
            if(strstr(string,"0x"))
            {
                base = 16;
            }
            str = strtok(string,",");
            tos_vo = strtol(str,&endptr,base);
            str = strtok(NULL,",");
            tos_vi = strtol(str,&endptr,base);
            str = strtok(NULL,",");
            tos_be = strtol(str,&endptr,base);
            str = strtok(NULL,",");
            tos_bk = strtol(str,&endptr,base);
        }
    }
    printf("Using TOS: VO=0x%x,VI=0x%x,BE=0x%x,BK=0x%x\n",
           tos_vo,tos_vi,tos_be,tos_bk);

    traceflag=1;
    while (1)
    {
        if(nsta)
        {
            printf("\nWaiting in state %d\n",sta->state);
        }
        if (( sta== NULL) || (bKeepFrom == 1))
            r = recvfrom(sd, rmsg, sizeof(rmsg), flags, (struct sockaddr *)&from, &fromlen);
        else
            r = recv(sd, rmsg, sizeof(rmsg), flags);

        resettimer(INTERSTEP_PERIOD*10);
        if (r<0)
        {
            perror("rcv error:");
            exit(1);
        }
        alarm(0);
        tout=0;

        /* check some cases  */
        if (traceflag && strcmp(procname,"L.1"))
        {
            printf( "APTS Received #    length:%d\n",  r );
            mpx("APTS RX", rmsg, 64);
        }
        // Do not process unless from remote
        if (from.sin_addr.s_addr==0 || from.sin_addr.s_addr==local.sin_addr.s_addr)
        {
            printf( "Received 0 / local\n" );
            continue;
        }
        if (from.sin_addr.s_addr==target.sin_addr.s_addr)
        {
            printf( "Received BROADCAST\n" );
            continue;
        }
        if (rmsg[10]==APTS_BCST)
        {
            printf( "Received BROADCAST, skipping\n" );
            continue;
        }
        /* check some cases  */

        printf("\r\n cmd is %d",rmsg[11]);

        if (rmsg[10]==APTS_HELLO)
        {

            if((id = get_sta_id(from.sin_addr.s_addr))>=0)
            {
                if(!reset)
                    continue;
                printf("\n HELLO after reset");

            }
            else if((id = assign_sta_id(from.sin_addr.s_addr))<0)
            {
                printf( "Can not assign id,sta list full");
                continue;
            }

            sta = &stations[id];
            bzero(sta->ipaddress, 20);
            inet_ntop(AF_INET, &(from.sin_addr), IP, INET_ADDRSTRLEN);
            printf("ip is %s\n",IP);
            strcpy( &(sta->ipaddress[0]), IP);
            sta->cmd =  testcase->cmd;
            sta->cookie = 0;
            sta->nsent = 0;
            sta->nerr = 0;
            sta->msgno = 0;
            sta->myid = id;
            sta->alreadyCleared = 0;
            printf("new_station: size(%d) id=%02d IP address:%s\n", r, id, sta->ipaddress);
            if(reset)
                reset = 0;
            else
                nsta++;
            printf( "New STA = %d\n", nsta );
            sta->state = 0;
            sta->statefunc = consoleProcStatetbl[sta->cmd];

        }/*  if (rmsg[10]==APTS_HELLO)  */
        else
        {
            if(reset)
                continue;
            if((id = get_sta_id(from.sin_addr.s_addr))<0)
            {
                inet_ntop(AF_INET, &(from.sin_addr), IP, INET_ADDRSTRLEN);
                printf("\r\n Unexpected message rcd from ip s_addr=%s sta id=%d", IP, id);
                continue;
            }
            sta = &stations[id];
        }/* else  */

        if(rmsg[10] == APTS_RESET)
        {
            reset_recd = 1;
            printf("\nRcv RESET from STA, sent RESET back to STA, exit\n");
            WfaConResetAll();
            exit(0);
        }
        if(rmsg[10] == APTS_RESET_STOP)
        {
            printf("\r\n Recd Kill from Sta\n");
            exit(0);
        }

        func = (sta->statefunc)[sta->state];
        if(!sta->alreadyCleared)
        {
            func.statefunc(sta,rmsg,r);
        }

    }/* while loop  */
}/*  main */

