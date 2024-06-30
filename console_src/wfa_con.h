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
* APTS messages/tests
*/
#define B_D             1
#define B_H             2
#define B_B             3
#define B_M             4
#define M_D             5
#define B_Z             6
#define M_Y             7
#define L_1             8
#define A_Y             9   /* active mode version of M_Y */
#define B_W             10
#define A_J             11  /* Active test of sending 4 down */
#define M_V             12
#define M_U             13
#define A_U             14
#define M_L             15
#define B_K             16
#define M_B             17
#define M_K             18
#define M_W             19
#define LAST_TEST       M_W
#define APTS_DEFAULT    20  /* message codes, cmd code */
#define APTS_HELLO      21
#define APTS_BCST       22
#define APTS_CONFIRM    23
#define APTS_STOP       24
#define APTS_CK_BE      25
#define APTS_CK_BK      26
#define APTS_CK_VI      27
#define APTS_CK_VO      28
#define APTS_RESET      29
#define APTS_RESET_RESP 30
#define APTS_RESET_STOP 31
#define APTS_LAST       99
#define NSTA            20 /* number of trackable stations (id==0 held in reserve) */

/*
* internal table
*/
struct apts_msg
{
    char *name;         // name of test
    int cmd;            // msg num
    int param0;         // number of packet exchanges
    int param1;         // number of uplink frames
    int param2;         // number of downlink frames
    int param3;
};

/*
* wme
*/
#define TOS_VO7     0xE0    // 111 0  0000 (7)  AC_VO tos/dscp values
#define TOS_VO      0xD0    // 110 1  0000 (6)  AC_VO tos/dscp values  ???
#define TOS_VO6     0xC0    // 110 0  0000  
#define TOS_VO2     0xB8    // 101 1  1000  

#define TOS_VI      0xA0    // 101 0  0000 (5)  AC_VI
#define TOS_VI4     0x80    // 100 0  0000 (4)  AC_VI
#define TOS_VI5     0x88    // 100 0  1000

#define TOS_BE      0x00    // 000 0  0000 (0)  AC_BE
#define TOS_EE      0x60    // 011 0  0000 (3)  AC_BE

#define TOS_BK      0x20    // 001 0  0000 (1)  AC_BK
#define TOS_LE      0x40    // 010 0  0000  sync with win sta, but why use this name

struct station;
typedef int (*ConsoleStateFunctionPtr)(struct station*, unsigned int *, int ); //Recieved message buffer, length
typedef struct console_state_table
{
    ConsoleStateFunctionPtr statefunc;
} consoleProcStatetbl_t;

struct station
{
    // keep state for individual stations
    int cookie;             // cookie value exchanged with each staut
    unsigned long s_addr;       // station address
    int nsent, nrecv, nend, nerr,msgno;   // counters
    int alreadyCleared; // ignore this station if this value is 1
    char ipaddress[20];
    int state;
    int cmd;
    int myid;
    consoleProcStatetbl_t  *statefunc;
};
