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
#include "wfa_con.h"
#include <sys/time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
extern struct sockaddr dst;         // sock declarations
extern unsigned int rmsg[512];      // rx msg buffer
extern unsigned int txmsg[512];	    // tx msg buffer
extern int sd,rd;				    // socket descriptor
extern int sockflags;			    // socket call flags
extern struct station stations[NSTA];
extern struct sockaddr_in from;
extern char traceflag;				// enable debug packet tracing
extern int tos_vo,tos_vi,tos_be,tos_bk;
int can_quit=1;
void exit(int);

int WfaConRcvHello(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_HELLO,tos_be))
    {
        printf("\r\nHello not rcv or BAD TOS\n");
    }
    else
    {
        create_apts_msg(sta->cmd, txmsg,sta->myid);
        r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send(Hello)\n", txmsg, 64);
    }

}
int WfaConSndVI(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    set_dscp(tos_vi);
    txmsg[0] = ++(sta->msgno);
    create_apts_msg(APTS_DEFAULT, txmsg,sta->myid);
    txmsg[1] = tos_vi;
    r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
    sta->state++;
    if (traceflag) mpx("CMD send(SndVI)\n", txmsg, 64);
    printf("sent VI\n");

}
int WfaConRcvConf(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_CONFIRM,tos_be))
    {
        printf("\r\nConfirm not rcv or BAD TOS\n");
    }
    else
    {
        sta->state++;
        printf("\r\nRcv Confirm\n");
    }
}
int WfaConRcvConfSndVI(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_CONFIRM,tos_be))
    {
        printf("\r\nConfirm not rcv or BAD TOS\n");
    }
    else
    {
        set_dscp(tos_vi);
        txmsg[0] = ++(sta->msgno);
        create_apts_msg(APTS_DEFAULT, txmsg,sta->myid);
        txmsg[1] = tos_vi;
        r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send (VI)\n", txmsg, 64);
        printf("Rcv Confirm sent VI\n");
    }
}
int WfaConRcvVOSnd(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vo))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_vo);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send (RcvVOSnd)\n", rmsg, 64);
        printf("Rcv VO sent VO\n");
    }
}
/* L.1 test case  */
int WfaConRcvVOSndCyclic(struct station *sta,unsigned int *rmsg,int length)
{
    int r,id, dscpLocal = 0;
    static int count=0;
    if(!expectedmsgrcdl1(rmsg,APTS_DEFAULT,tos_vo)) // call L.1 case check routine in wfa_util.c
    {

        if(!expectedmsgrcdl1(rmsg,APTS_STOP,tos_be)) //call L.1 case check routine in wfa_util.c
            printf("\r\nWfaConRcvVOSndCyclic::expected STOP message not rcv or BAD TOS BE\n");
        else
        {
            count = 0;
            for(id=0; id<NSTA; id++)
            {
                if (stations[id].s_addr == 0)
                {
                    break;
                }
                else
                {
                    printf("\n sta count is %d\n",stations[id].msgno);
                    count+=stations[id].msgno;
                }
            }

            txmsg[0] = count;
            printf("\n count is %d\n",count);
            set_dscp(tos_be);
            create_apts_msg(APTS_STOP, txmsg,sta->myid);
            txmsg[1] = tos_be;
            if(can_quit)
            {
                if(!sta->alreadyCleared)
                {
                    strcpy((char *)&txmsg[11], "APTSL1 STOP");
                    r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
                    if (traceflag) mpx("CMD send\n", txmsg, 64);
                    sleep(5);
                    printf("\n >>> EXISTING >>> ret=%d\n",r);
                    exit(0);
                }
                else
                    r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
                if (traceflag) mpx("CMD send\n", txmsg, 64);
            }
            else
            {
                r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
                if (traceflag) mpx("CMD send\n", txmsg, 64);

                sta->alreadyCleared = 0;
                can_quit=1;
            }
        }
    }
    else
    {
        count++;
        printf("\r\n Rcv pkt #%d ",count);
        rmsg[0] = ++(sta->msgno);
        dscpLocal=set_dscp0(tos_vo);  // no sleep setting
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        printf("sent dscp VO, return dscp=0x%x\n", dscpLocal);
        if ( count >= 3000)
        {
            //No state change for L.1::sta->state++;
            //count = 0;
        }
    }
}
int WfaConRcvVOSndE(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vo))
    {
        printf("\r\nExpected message not rcv or BAD TOS\n");
    }
    else
    {
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_vo);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        if (traceflag) mpx("CMD send(VO)\n", rmsg, 64);
        printf("Rcv VO sent VO\n");
        txmsg[0] = ++(sta->msgno);
        set_dscp(tos_be);
        create_apts_msg(APTS_STOP, txmsg,sta->myid);
        txmsg[1] = tos_be;
        r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send STOP(RcvVOSndE) BE\n", txmsg, 64);

    }
}
int WfaConRcvVOSndAllE(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vo))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_vo);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        if (traceflag) mpx("CMD send\n", rmsg, 64);
        printf("Rcv VO sent VO\n");

        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_vi);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        if (traceflag) mpx("CMD send\n", rmsg, 64);
        printf("sent VI\n");

        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_be);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        if (traceflag) mpx("CMD send\n", rmsg, 64);
        printf("sent BE\n");

        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_bk);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        if(r < 0)
        {
            printf("\nhere u go:dead\n");
        }
        if (traceflag) mpx("CMD send(BK)\n", rmsg, 64);
        {
            printf("sent BK\n");
        }

        txmsg[0] = ++(sta->msgno);
        set_dscp(tos_bk);
        create_apts_msg(APTS_STOP, txmsg,sta->myid);

        txmsg[1] = tos_bk;
        r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        if (traceflag) mpx("CMD send STOP(RcvVOSndAllE)\n", txmsg, 64);
        sta->state++;
    }
}
int WfaConRcvBESnd(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_be))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_be);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send(BE)\n", rmsg, 64);
        printf("Rcv BE sent BE\n");
    }
}
int WfaConRcvBESndE(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_be))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_be);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        if (traceflag) mpx("CMD send(BE)\n", rmsg, 64);
        printf("Rcv BE sent BE\n");
        create_apts_msg(APTS_STOP, txmsg,sta->myid);
        txmsg[0] = ++(sta->msgno);
        r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send STOP(RcvBESndE)\n", txmsg, 64);
    }
}
int WfaConRcvVISndBE(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vi))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_be);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send(BE)\n", rmsg, 64);
        printf("Rcv VI sent BE\n");
    }
}
int WfaConRcvVISndBK(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vi))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_bk);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send(BK)\n", rmsg, 64);
        printf("Rcv VI sent BK\n");
    }
}
int WfaConRcvVOSndBcastE(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vo))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        usleep(300000);
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_be);
        r = sendto(rd, rmsg, 190, sockflags, (struct sockaddr *)&dst, sizeof(dst));
        if (traceflag) mpx("CMD send\n", rmsg, 64);
        printf("Rcv VO sent Broadcast\n");
        txmsg[0] = ++(sta->msgno);
        create_apts_msg(APTS_STOP, txmsg,sta->myid);
        r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send STOP(RcvVOSndBcastE)\n", txmsg, 64);
    }
}
int WfaConRcvVISndBcastE(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vi))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_be);
        r = sendto(rd, rmsg, 190, sockflags, (struct sockaddr *)&dst, sizeof(dst));
        if (traceflag) mpx("CMD send\n", rmsg, 64);
        printf("Rcv VI, sent Broadcast\n");
        txmsg[0] = ++(sta->msgno);
        create_apts_msg(APTS_STOP, txmsg,sta->myid);
        r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send STOP(RcvVISndBcastE)\n", txmsg, 64);
    }
}
/* rcv VI echo back VI, sent brct  */
int WfaConRcvVISndBcast(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vi))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_vi);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_be);
        r = sendto(rd, rmsg, 190, sockflags, (struct sockaddr *)&dst, sizeof(dst));
        if (traceflag) mpx("CMD send brdcst BE\n", rmsg, 64);
        printf("Rcv/sent VI, sent Broadcast\n");
        sta->state++;
    }
}
int WfaConRcvBESndBcast(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_be))
    {
        printf("\r\nexpected message BE not rcv or BAD TOS\n");
    }
    else
    {
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_be);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        rmsg[0] = ++(sta->msgno);
        r = sendto(rd, rmsg, 190, sockflags, (struct sockaddr *)&dst, sizeof(dst));
        if (traceflag) mpx("CMD send brdcst BE\n", rmsg, 64);
        printf("Rcv/sent BE, sent Broadcast\n");
        sta->state++;
    }
}
int WfaConRcvBESndBcastE(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_be))
    {
        printf("\r\nexpected message BE not rcv or BAD TOS\n");
    }
    else
    {
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_be);
        r = sendto(rd, rmsg, 190, sockflags, (struct sockaddr *)&dst, sizeof(dst));
        if (traceflag) mpx("CMD send brdcst\n", rmsg, 64);
        printf("Rcv BE sent Broadcast\n");
        txmsg[0] = ++(sta->msgno);
        create_apts_msg(APTS_STOP, txmsg,sta->myid);
        r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send STOP(RcvBESndBcastE)\n", txmsg, 64);
    }
}
/* rcv VI send VI  */
int WfaConRcvVISnd(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vi))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_vi);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send\n", rmsg, 64);
        printf("Rcv VI sent VI (echo)\n");
    }
}
int WfaConRcvVISndE(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vi))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_vi);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        if (traceflag) mpx("CMD send\n", rmsg, 64);
        printf("rcv VI sent VI\n");

        txmsg[0] = ++(sta->msgno);
        create_apts_msg(APTS_STOP, txmsg,sta->myid);
        set_dscp(tos_be);
        r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send STOP(RcvVISndE)\n", txmsg, 64);
    }
}
int WfaConRcvVISndVOE(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vi))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        rmsg[0] = ++(sta->msgno);
        set_dscp(tos_vo);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        if (traceflag) mpx("CMD send\n", rmsg, 64);
        printf("rcv VI sent VO\n");

        txmsg[0] = ++(sta->msgno);
        create_apts_msg(APTS_STOP, txmsg,sta->myid);
        set_dscp(tos_be);
        r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send STOP(RcvVISndVOE)\n", txmsg, 64);
    }
}
int WfaConRcvVOE(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vo))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        printf("rcv VO\n");
        usleep(1000000);
        create_apts_msg(APTS_STOP, txmsg,sta->myid);

        txmsg[0] = ++(sta->msgno);
        set_dscp(tos_be);
        r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send STOP(RcvVOE)\n", txmsg, 64);
    }
}
int WfaConRcvVIE(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vi))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        printf("rcv VI\n");
        rmsg[0] = ++(sta->msgno);
        create_apts_msg(APTS_STOP, txmsg,sta->myid);
        set_dscp(tos_be);
        r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send STOP(RcvVIE)\n", txmsg, 64);
    }
}
int WfaConRcvVOSnd2VO(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    printf("\nEntering WfaConRcvVOSnd2VO\n ");
    if(expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vo)  )
    {
        set_dscp(tos_vo);
        rmsg[0] = ++(sta->msgno);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));

        rmsg[0] = ++(sta->msgno);
        if (traceflag) mpx("CMD send\n", rmsg, 64);
        r = sendto(sd, rmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send\n", rmsg, 64);
        printf("rcv VO sent 2 VO\n");
    }
    else
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
}
int WfaConRcvVI(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vi))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        printf("rcv VI\n");
        sta->state++;
    }
}
int WfaConRcvVO(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_vo))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        printf("rcv VO\n");
        sta->state++;
    }
}
int WfaConRcvBE(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_be))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        printf("rcv BE\n");
        sta->state++;
    }
}
int WfaConRcvBKE(struct station *sta,unsigned int *rmsg,int length)
{
    int r;
    if(!expectedmsgrcd(rmsg,APTS_DEFAULT,tos_bk))
    {
        printf("\r\nexpected message not rcv or BAD TOS\n");
    }
    else
    {
        printf("rcv BK\n");
        rmsg[0] = ++(sta->msgno);
        create_apts_msg(APTS_STOP, txmsg,sta->myid);
        r = sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
        sta->state++;
        if (traceflag) mpx("CMD send STOP(RcvBKE)\n", txmsg, 64);
    }
}
int WfaConWaitStop(struct station *sta,unsigned int *rmsg,int length)
{

    printf("WfaConWaitStop: send stop to STA again\n");
    rmsg[0] = ++(sta->msgno);
    create_apts_msg(APTS_STOP, txmsg,sta->myid);
    sendto(sd, txmsg, 190, sockflags, (struct sockaddr *)&from, sizeof(from));
    if (traceflag) mpx("CMD send STOP in WfaConWaitStop\n", txmsg, 64);

    if(expectedmsgrcd(rmsg,APTS_STOP,tos_be)  )
    {
        if(can_quit)
            exit(0);
        else
            can_quit=1;

    }
    else
    {
        printf("\r\nSTOP not rcv or BAD TOS\n");
    }
}
