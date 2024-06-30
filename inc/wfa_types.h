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
 * wfa_types.h:
 *   Defines general types and enum
 */
#ifndef _WFA_TYPES_H
#define _WFA_TYPES_H

#define WFA_IF_NAME_LEN 16
#define WFA_VERSION_LEN 32
#define WFA_SSID_NAME_LEN 64
#define WFA_IP_ADDR_STR_LEN  16
#define WFA_IP_MASK_STR_LEN  16
#define WFA_MAX_DNS_NUM      2
#define WFA_MAC_ADDR_STR_LEN 18
#define WFA_PROGNAME_LEN 8

#define WFA_CLI_CMD_RESP_LEN 128
#define WFA_P2P_DEVID_LEN 18
#define WFA_P2P_GRP_ID_LEN 128
#define WFA_WPS_PIN_LEN 256
#define WFA_PSK_PP_LEN	256

#define WFA_WFD_SESSION_ID_LEN 64
#define WFA_URL_STRING_LEN          256
#define WFA_EVT_ACTION_LEN 8


#define IF_80211   1
#define IF_ETH     2

/* WMM-AC APSD defines*/
#ifdef WFA_WMM_AC
#define DIR_NONE  0
#define DIR_UP    1
#define DIR_DOWN  2
#define DIR_BIDIR 3
#endif

typedef unsigned short WORD;
typedef unsigned char BYTE;

enum _response_staus
{
    STATUS_RUNNING = 0x0001,
    STATUS_INVALID = 0x0002,
    STATUS_ERROR = 0x0003,
    STATUS_COMPLETE = 0x0004,
};

typedef int BOOL;

#ifndef    TRUE
#define    FALSE       -1
#define    TRUE        0
#define    DONE        1
#endif

typedef enum returnTypes
{
    WFA_SUCCESS = 0,
    WFA_FAILURE = 1,
    WFA_ERROR = -1,
} retType_t;

enum wfa_state
{
    WFA_DISABLED = 0,
    WFA_ENABLED = 1,
    WFA_OPTIONAL = 1,
    WFA_REQUIRED = 2,
    WFA_F_REQUIRED = 3,            /* forced required */
    WFA_F_DISABLED = 4,            /* forced disabled */
    WFA_INVALID_BOOL = 0xFF
};
#endif
