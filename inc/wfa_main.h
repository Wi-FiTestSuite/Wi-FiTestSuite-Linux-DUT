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


#ifndef _WFA_MAIN_H
#define _WFA_MAIN_H

#ifdef WFA_PC_CONSOLE
#define WFA_MAX_TRAFFIC_STREAMS           32

/* WMM THREADS NUMBER */
#define WFA_THREADS_NUM   32
#else /* for STAION */
#define WFA_MAX_TRAFFIC_STREAMS           8

/* WMM THREADS NUMBER */
#define WFA_THREADS_NUM   8
#endif

#define MAX_CMD_BUFF        1024
#define MAX_PARMS_BUFF      640

#define MAX_TRAFFIC_BUF_SZ  1536

#define WFA_BUFF_32         32
#define WFA_BUFF_64         64
#define WFA_BUFF_128        128
#define WFA_BUFF_512        512
#define WFA_BUFF_1K         1024
#define WFA_BUFF_4K         4096

#ifdef WFA_PC_CONSOLE
#define WFA_RESP_BUF_SZ    WFA_BUFF_4K
#else
#define WFA_RESP_BUF_SZ    WFA_BUFF_4K
#endif

#define WFA_CMD_STR_SZ      512

enum tg_port
{
    UDP_PORT_BE1 = 0,
    UDP_PORT_BE2 = 1,
    UDP_PORT_BK1 = 2,
    UDP_PORT_BK2 = 3,
    UDP_PORT_VI1 = 4,
    UDP_PORT_VI2 = 5,
    UDP_PORT_VO1 = 6,
    UDP_PORT_VO2 = 7,
    UDP_PORT_ND1 = 8,
    UDP_PORT_ND2 = 9
};

#ifndef _WINDOWS
#define min(a,b)        ((a) < (b) ? (a) : (b))
#define max(a,b)        ((a) > (b) ? (a) : (b))

#endif
#endif
