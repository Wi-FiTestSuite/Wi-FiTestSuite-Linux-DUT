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
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>
#include "wfa_con.h"
extern struct sockaddr_in target;
extern struct sockaddr_in from;
extern struct sockaddr_in local;
extern struct sockaddr dst;           // sock declarations
extern unsigned char dscp, ldscp;		// new/last dscp output values
extern unsigned int rmsg[512];           // rx msg buffer
extern struct station stations[NSTA];
extern int port,rd,sd,can_quit  ;
extern struct apts_msg apts_msgs[];
extern char traceflag;				// enable debug packet tracing
int tos_vo,tos_vi,tos_be,tos_bk;
void exit(int);
char getipclass(unsigned long ip)
{

    if (  (  ip & 0x80 ) == 0x00 ) // 0xxxx
        return 'A';
    else if (  ( ip  & 0xC0 ) == 0x80 ) // 10xxx
        return 'B';
    else if (  (  ip & 0xE0 ) == 0xC0 ) // 110xx
        return 'C';
    else if (  (  ip & 0xF0 ) == 0xE0 ) // 1110x
        return 'D';
    else if (  (  ip & 0xF8 ) == 0xF0 ) // 11110
        return 'E';
    else
        return 'E';
}
int setup_addr(char *name, struct sockaddr *dst)
{
    struct hostent *h;
    char *array[5];
    char c;
    char *s;
    int d, r;
    unsigned long in =0;
    unsigned char b;
    int fd;
    struct sockaddr_in *addr_ptr;;
    struct ifreq interface;
    char IP[INET6_ADDRSTRLEN];
    char Mask[INET6_ADDRSTRLEN];
    int one = 1,j;

#ifndef __CYGWIN__
    printf( "setup_addr: entering\n" );
    printf("Automatically discover IP Address of eth0 interface, since no broadcast IP address has been specified by you.\n");
    /* tells ioctl which interface to query */
    strcpy(interface.ifr_name, "eth0");
    /* need a socket to use ioctl */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    /* get interface ip address */
    if (ioctl(fd, SIOCGIFADDR, &interface) < 0)
    {
        perror("ERROR (ioctl (SIOCGIFADDR)), no IP address for eth0");
        exit(1);
    }


    /* kinda ugly, but this allows us to use inet_ntop */
    addr_ptr = (struct sockaddr_in *) &interface.ifr_addr;

    /* INET protocol family */
    addr_ptr->sin_family = AF_INET;

    /* copy ip address */
    inet_ntop(AF_INET, &addr_ptr->sin_addr, IP, INET_ADDRSTRLEN);

    char class= getipclass((unsigned long)addr_ptr->sin_addr.s_addr);
    if(class < 'A' || class >'C')
    {
        printf("Bad IP Class %c addr %d for %s\n",class,addr_ptr->sin_addr.s_addr,IP);
        exit(-6);
    }
    printf("IP of eth0: %s\n", IP);

    ioctl(fd,SIOCGIFNETMASK,&interface);
    /* inet_ntoa::These functions are deprecated because they don't handle IPv6!
    Use inet_ntop() or inet_pton() instead!   */
    inet_ntop(AF_INET, &( ((struct sockaddr_in*)&interface.ifr_addr)->sin_addr), Mask, INET_ADDRSTRLEN);
    printf("setup_addr::Mask=%s  \n", Mask);
    char* temp;
    int ss=0;
    temp=strtok(Mask,". ");
    if ( temp != NULL)
    {
        while(!strncmp(temp,"255",3) && ss<5)
        {
            ss++;
            temp=strtok(NULL,". ");
            if (temp == NULL)
            {
                printf("setup_addr::while loop detected null pointer\n");
                break;
            }
        }
    }
    else
    {
        printf("setup_addr::Mask=%s, temp is NULL \n", Mask);
    }

    printf(" This is a Class %c IP Address \n",class);

    char ip[INET_ADDRSTRLEN];
    int count = 0;
    int index = 0;
    int num_of_255=ss;
    int subnet_places=4-ss;
    while(count< subnet_places)
    {
        ip[index] = IP[index];
        if(ip[index] == '.')
            count ++;
        index++;
    }
    for(j=0; j<num_of_255; j++)
    {
        ip[index++] = '2';
        ip[index++] = '5';
        ip[index++] = '5';
        ip[index++] = '.';
    }
    ip[index - 1] = '\0';

    name = ip;
    printf("BROADCAST dst %s\n", name);
#endif

    if (is_ipdotformat(name))
    {
        // check for dot format addr
        strvec_sep(name, array, 5, ".");
        for(d=0; d<4; d++)
        {
            b = atoi(array[d]);
            in |= (b<<(d*8));
        }
        target.sin_addr.s_addr = in;
    }
    else
    {
        h = gethostbyname(name);                // try name lookup
        if (h)
        {
            memcpy((caddr_t)&target.sin_addr.s_addr, h->h_addr, h->h_length);
        }
        else
        {
            fprintf(stderr, "name lookup failed for (%s)\n", name);
            exit(-1);
        }
    }
    pbaddr("dst:", target.sin_addr.s_addr);
    target.sin_family = AF_INET;
    target.sin_port =htons( port);
    memcpy((caddr_t)dst, (caddr_t)&target, sizeof(target));
    r = setsockopt(rd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));
    if ( r<0 )
    {
        perror("multicast mode socket setup 1");
    }
    printf("multicast mode r %d\n", r);
    from.sin_family = AF_INET;
    from.sin_port = htons(port);
    from.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    r = bind(sd, (struct sockaddr *)&from, sizeof(from));
    if (r<0)
    {
        perror("bind call failed");
        exit(-1);
    }
    return(r);
}
void setup_socket()
{
    char name[256];
    int dscp = 0;
    int stype = SOCK_DGRAM;
    int sproto = 0;
    int r;

    dscp = tos_be;
    if ((sd=socket(AF_INET, stype, sproto)) < 0)
    {
        perror("socket");
        exit(-1);
    }
    if ((rd=socket(AF_INET, stype, sproto)) < 0)
    {
        perror("socket");
        exit(-1);
    }
    set_dscp(dscp);
    /* Setup broadcast*/
    if ((r=setup_addr(name, &dst))<0)
    {
        fprintf(stderr, "can't map address (%s)\n", name);
        exit(-1);
    }
}
int set_dscp(int new_dscp)
{
    int r;

    if ((r=setsockopt(sd, SOL_IP, IP_TOS, &new_dscp, sizeof(dscp)))<0)
    {
        perror("can't set dscp/tos field");
        printf("can't set dscp/tos field 0x%x", new_dscp);
        exit(-1);
    }
    dscp = ldscp = new_dscp;
    rmsg[1] = dscp;
    usleep(100000);
    return(new_dscp);
}
int set_dscp0(int new_dscp)
{
    int r;

    if ((r=setsockopt(sd, SOL_IP, IP_TOS, &new_dscp, sizeof(dscp)))<0)
    {
        perror("can't set dscp/tos field");
        printf("can't set dscp/tos field 0x%x", new_dscp);
        exit(-1);
    }
    dscp = ldscp = new_dscp;
    rmsg[1] = dscp;
    return(new_dscp);
}
struct apts_msg * apts_lookup(char *s)
{
    struct apts_msg *t;

    for (t=&apts_msgs[1]; s && t->cmd; t++)
    {
        if (t->name && strcmp(t->name, s)==0)
        {
            return(t);
        }
    }
    fprintf(stderr, "APTS Test(%s) unknown\n", s);
    fprintf(stderr, "available tests are:\n");
    for (t=&apts_msgs[LAST_TEST]; t->cmd; t++)
    {
        if (t->name)
        {
            fprintf(stderr, "\t%s\n", t->name);
        }
    }
    exit(-1);
}
void create_apts_msg(int msg, unsigned int txbuf[],int id)
{
    struct apts_msg *t;

    t = &apts_msgs[msg];
    txbuf[ 1] = 0;
    txbuf[ 2] = 0;
    txbuf[ 3] = 0;
    txbuf[ 4] = 0;
    txbuf[ 5] = 0;
    txbuf[ 6] = t->param0;
    txbuf[ 7] = t->param1;
    txbuf[ 8] = t->param2;
    txbuf[ 9] = id;
    txbuf[ 10] = t->cmd;
    strcpy((char *)&txbuf[11], t->name);
    if (traceflag) printf("create_apts_msg (%s)\n", t->name);
}
/*  general used for rcv msg checking */
int expectedmsgrcd(unsigned int *rmsg,unsigned long type,int tos)
{
    char type_ok = 0, acBuf[4];
    char tos_ok = 0;
    int  r = 0;

    memset(acBuf, 0, 4);
    switch ( tos)
    {
    case TOS_VO7:
    case TOS_VO:
    case TOS_VO6:
    case TOS_VO2:
        if ( rmsg[1] ==TOS_VO7 || rmsg[1]==TOS_VO || rmsg[1]==TOS_VO6  || rmsg[1] ==TOS_VO2)
        {
            tos_ok = 1;
        }
        break;
    case TOS_VI:
    case TOS_VI4:
    case TOS_VI5:
        if ( rmsg[1] ==TOS_VI || rmsg[1]==TOS_VI4 || rmsg[1]==TOS_VI5)
        {
            tos_ok = 1;
        }
        break;
    case TOS_BE:
    case TOS_EE:
        if ( rmsg[1] ==TOS_BE || rmsg[1]==TOS_EE )
        {
            tos_ok = 1;
        }
        break;
    case TOS_BK:
    case TOS_LE:
        if ( rmsg[1] ==TOS_BK || rmsg[1]==TOS_LE )
        {
            tos_ok = 1;
        }
        break;
    default:
        printf("\nexpectedmsgrcd not know tos=0x%x\n", tos);
    }
    if ( tos_ok == 0)
    {
        /* check what we got  */
        switch ( rmsg[1])
        {
        case TOS_VO7:
        case TOS_VO:
        case TOS_VO6:
        case TOS_VO2:
            strcat(acBuf, "VO");
            break;
        case TOS_VI:
        case TOS_VI4:
        case TOS_VI5:
            strcat(acBuf, "VI");
            break;
        case TOS_BE:
        case TOS_EE:
            strcat(acBuf, "BE");
            break;
        case TOS_BK:
        case TOS_LE:
            strcat(acBuf, "BK");
            break;
        default:
            printf("\nexpectedmsgrcd check unexpected TOS:not know rmsg[1]=0x%x\n", rmsg[1]);
        }
    }
    type_ok=((rmsg[10] == type)? 1:0);

    r = (type_ok && tos_ok);
    if(!r)
    {
        printf("\nexpectedmsgrcd no match: rmsg[10] %u expect type %lu rmsg[1] as tos 0x%x expect tos 0x%x\n",
               rmsg[10], type,rmsg[1], tos);
        printf("expectedmsgrcd::rcv %s as repeate pkt for previous state\n",acBuf);
    }

    return r;

}
/*  L.1 test case used only in wfa_sndrcv.c WfaConRcvVOSndCyclic */
int expectedmsgrcdl1(unsigned int *rmsg,unsigned long type,int tos)
{
    char type_ok;
    char tos_ok = 0;
    int r;
    char dsc;

    type_ok=((rmsg[10] == type)? 1:0);

    if ( rmsg[1] ==TOS_VO7 || rmsg[1]==TOS_VO || rmsg[1]==TOS_VO6  || rmsg[1] ==TOS_VO2)
        tos_ok=1;
    else if ( rmsg[1] ==TOS_BE || rmsg[1]==TOS_EE )
        tos_ok = 1;

    r = (type_ok && tos_ok);
    if((!r)&&(rmsg[10] != APTS_STOP))
        WfaConResetAll();
    return r;

}
int assign_sta_id(unsigned int addr)
{
    printf("\nassign_sta_id for addr=0x%x\n", addr);
    int id;
    for(id=0; id<NSTA; id++)
    {
        if (stations[id].s_addr == 0)
        {
            stations[id].s_addr = addr;
            printf("\nAssign station id=%d for addr=0x%x\n", id, addr);
            break;
        }
    }
    if(id == NSTA)
    {
        id = -1;
        printf("assign_sta_id, err id==NSTA\n");
    }
    if(id > 0)
    {
        can_quit=0;
        printf("assign_sta_id, id=%d, set can_quit=0\n",id);
    }
    return(id);
}
int get_sta_id(unsigned int addr)
{
    printf("\nget_sta_id for 0x%x\n", addr);
    int id;
    for(id=0; id<NSTA; id++)
    {
        if (stations[id].s_addr == addr)
        {
            break;
        }
    }
    if(id == NSTA)
    {
        id = -1;
        printf("get_sta_id: could not get sta id \n");
    }
    return(id);
}
is_ipdotformat(char *s)
{
    int d;

    for(d=0; *s; s++)
    {
        if (*s=='.')
            d++;
    }
    return(d==3);
}
int  strvec_sep(s, array, n, sep)
char *s, *array[], *sep;
int n;
{
    char *p;
    static char buf[2048];
    int i;

    strncpy(buf, s, sizeof(buf));

    p = strtok(buf, sep);

    for (i=0; p && i<n; )
    {
        array[i++] = p;
        if (i==n)
        {
            i--;
            break;
        }
        p = strtok(0, sep);
    }
    array[i] = 0;
    return(i);
}

pbaddr(s, in)
char *s;
unsigned long int in;
{
    unsigned char a, b, c, d;

    a = in&0xff;
    b = (in>>8)&0xff;
    c = (in>>16)&0xff;
    d = (in>>24)&0xff;
    printf("%s %d.%d.%d.%d\n", s, a, b, c, d);
}

