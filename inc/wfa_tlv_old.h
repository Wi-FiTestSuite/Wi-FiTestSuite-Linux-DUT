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
 * File: wfa_tlv.h
 *  definitions for each of command types.
 *  format:
 *  ---------------------------------------------------
 *  |  tag   | length |   value
 *  ---------------------------------------------------
 *   2 bytes   2bytes    defined by length
 */
#ifndef _WFA_TLV_H
#define _WFA_TLV_H

#include "wfa_types.h"

#define INTEGER_1 0x0001            /* byte */
#define INTEGER_2 0x0002            /* word */
#define OCTET_STRING 0x0003         /* string */

typedef struct _wfatlvHdr
{
    WORD tag;   /* tag/type */
    WORD len;   /* value length */
} wfaTLV;

#define WFA_TLV_HDR_LEN sizeof(wfaTLV)

/* Commands */
#define WFA_GET_VERSION_TLV                                         0x01

/* Generic TRAFFIC GENERATOR Commands */
#define WFA_TRAFFIC_SEND_PING_TLV        (WFA_GET_VERSION_TLV + 0x01)                  /* 2 */
#define WFA_TRAFFIC_STOP_PING_TLV        (WFA_TRAFFIC_SEND_PING_TLV + 0x01)            /* 3 */
#define WFA_TRAFFIC_AGENT_CONFIG_TLV     (WFA_TRAFFIC_STOP_PING_TLV + 0x01)            /* 4 */
#define WFA_TRAFFIC_AGENT_SEND_TLV       (WFA_TRAFFIC_AGENT_CONFIG_TLV + 0x01)         /* 5 */
#define WFA_TRAFFIC_AGENT_RECV_START_TLV (WFA_TRAFFIC_AGENT_SEND_TLV + 0x01)           /* 6 */
#define WFA_TRAFFIC_AGENT_RECV_STOP_TLV  (WFA_TRAFFIC_AGENT_RECV_START_TLV + 0x01)     /* 7 */
#define WFA_TRAFFIC_AGENT_RESET_TLV      (WFA_TRAFFIC_AGENT_RECV_STOP_TLV + 0x01)      /* 8 */
#define WFA_TRAFFIC_AGENT_STATUS_TLV     (WFA_TRAFFIC_AGENT_RESET_TLV + 0x01)          /* 9 */

/* STATION/DUT Commands */
#define WFA_STA_GET_IP_CONFIG_TLV        (WFA_TRAFFIC_AGENT_STATUS_TLV + 0x01)         /* 10 */
#define WFA_STA_SET_IP_CONFIG_TLV        (WFA_STA_GET_IP_CONFIG_TLV + 0x01)            /* 11 */
#define WFA_STA_GET_MAC_ADDRESS_TLV      (WFA_STA_SET_IP_CONFIG_TLV + 0x01)            /* 12 */
#define WFA_STA_SET_MAC_ADDRESS_TLV      (WFA_STA_GET_MAC_ADDRESS_TLV + 0x01)          /* 13 */
#define WFA_STA_IS_CONNECTED_TLV         (WFA_STA_SET_MAC_ADDRESS_TLV + 0x01)          /* 14 */
#define WFA_STA_VERIFY_IP_CONNECTION_TLV (WFA_STA_IS_CONNECTED_TLV + 0x01)             /* 15 */
#define WFA_STA_GET_BSSID_TLV            (WFA_STA_VERIFY_IP_CONNECTION_TLV + 0x01)     /* 16 */
#define WFA_STA_GET_STATS_TLV            (WFA_STA_GET_BSSID_TLV + 0x01)                /* 17 */
#define WFA_STA_SET_ENCRYPTION_TLV       (WFA_STA_GET_STATS_TLV + 0x01)                /* 18 */
#define WFA_STA_SET_PSK_TLV              (WFA_STA_SET_ENCRYPTION_TLV + 0x01)           /* 19 */
#define WFA_STA_SET_EAPTLS_TLV           (WFA_STA_SET_PSK_TLV + 0x01)                  /* 20 */
#define WFA_STA_SET_UAPSD_TLV            (WFA_STA_SET_EAPTLS_TLV + 0x01)               /* 21 */
#define WFA_STA_ASSOCIATE_TLV            (WFA_STA_SET_UAPSD_TLV + 0x01)                /* 22 */
#define WFA_STA_SET_EAPTTLS_TLV          (WFA_STA_ASSOCIATE_TLV + 0x01)                /* 23 */
#define WFA_STA_SET_EAPSIM_TLV           (WFA_STA_SET_EAPTTLS_TLV + 0x01)              /* 24 */
#define WFA_STA_SET_PEAP_TLV             (WFA_STA_SET_EAPSIM_TLV + 0x01)               /* 25 */
#define WFA_STA_SET_IBSS_TLV             (WFA_STA_SET_PEAP_TLV + 0x01)                 /* 26 */
#define WFA_STA_GET_INFO_TLV             (WFA_STA_SET_IBSS_TLV + 0x01)                 /* 27 */
#define WFA_DEVICE_GET_INFO_TLV          (WFA_STA_GET_INFO_TLV + 0x01)                 /* 28 */
#define WFA_DEVICE_LIST_IF_TLV           (WFA_DEVICE_GET_INFO_TLV + 0x01)              /* 29 */

#define WFA_STA_DEBUG_SET_TLV            (WFA_DEVICE_LIST_IF_TLV + 0x01)               /* 30 */
#define WFA_STA_SET_MODE_TLV             (WFA_STA_DEBUG_SET_TLV + 0x01)                /* 31 */
#define WFA_STA_UPLOAD_TLV               (WFA_STA_SET_MODE_TLV + 0x01)                 /* 32 */
#define WFA_STA_SET_WMM_TLV              (WFA_STA_UPLOAD_TLV + 0x01)                   /* 33 */
#define WFA_STA_REASSOCIATE_TLV          (WFA_STA_SET_WMM_TLV + 0x01)                  /* 34 */
#define WFA_STA_SET_PWRSAVE_TLV          (WFA_STA_REASSOCIATE_TLV + 0x01)              /* 35 */
#define WFA_STA_SEND_NEIGREQ_TLV         (WFA_STA_SET_PWRSAVE_TLV + 0x01)              /* 36 */

#define WFA_STA_COMMANDS_END             (WFA_STA_SEND_NEIGREQ_TLV + 0x01)             /* 37 */


/* Version response */
#define WFA_GET_VERSION_RESP_TLV         (WFA_STA_COMMANDS_END +       0x01)           /* 38 */

/* Generic Traffic Generator Responses */
#define WFA_TRAFFIC_SEND_PING_RESP_TLV   (WFA_GET_VERSION_RESP_TLV +  0x01)            /* 39 */
#define WFA_TRAFFIC_STOP_PING_RESP_TLV   (WFA_TRAFFIC_SEND_PING_RESP_TLV + 0x01)       /* 40 */
#define WFA_TRAFFIC_AGENT_CONFIG_RESP_TLV (WFA_TRAFFIC_STOP_PING_RESP_TLV + 0x01)      /* 41 */
#define WFA_TRAFFIC_AGENT_SEND_RESP_TLV  (WFA_TRAFFIC_AGENT_CONFIG_RESP_TLV + 0x01)    /* 42 */
#define WFA_TRAFFIC_AGENT_RECV_START_RESP_TLV (WFA_TRAFFIC_AGENT_SEND_RESP_TLV + 0x01) /* 43 */
#define WFA_TRAFFIC_AGENT_RECV_STOP_RESP_TLV (WFA_TRAFFIC_AGENT_RECV_START_RESP_TLV + 0x01)
#define WFA_TRAFFIC_AGENT_RESET_RESP_TLV (WFA_TRAFFIC_AGENT_RECV_STOP_RESP_TLV + 0x01) /* 45 */
#define WFA_TRAFFIC_AGENT_STATUS_RESP_TLV (WFA_TRAFFIC_AGENT_RESET_RESP_TLV + 0x01)    /* 46 */

/* STATION/DUT Responses */
#define WFA_STA_GET_IP_CONFIG_RESP_TLV (WFA_TRAFFIC_AGENT_STATUS_RESP_TLV + 0x01)      /* 47 */
#define WFA_STA_SET_IP_CONFIG_RESP_TLV (WFA_STA_GET_IP_CONFIG_RESP_TLV + 0x01)         /* 48 */
#define WFA_STA_GET_MAC_ADDRESS_RESP_TLV (WFA_STA_SET_IP_CONFIG_RESP_TLV + 0x01)       /* 49 */
#define WFA_STA_SET_MAC_ADDRESS_RESP_TLV (WFA_STA_GET_MAC_ADDRESS_RESP_TLV + 0x01)     /* 50 */
#define WFA_STA_IS_CONNECTED_RESP_TLV (WFA_STA_SET_MAC_ADDRESS_RESP_TLV + 0x01)        /* 51 */
#define WFA_STA_VERIFY_IP_CONNECTION_RESP_TLV  (WFA_STA_IS_CONNECTED_RESP_TLV + 0x01)  /* 52 */
#define WFA_STA_GET_BSSID_RESP_TLV (WFA_STA_VERIFY_IP_CONNECTION_RESP_TLV + 0x01)      /* 53 */
#define WFA_STA_GET_STATS_RESP_TLV (WFA_STA_GET_BSSID_RESP_TLV + 0x01)                 /* 54 */
#define WFA_STA_SET_ENCRYPTION_RESP_TLV (WFA_STA_GET_STATS_RESP_TLV + 0x01)
#define WFA_STA_SET_PSK_RESP_TLV (WFA_STA_SET_ENCRYPTION_RESP_TLV + 0x01)
#define WFA_STA_SET_EAPTLS_RESP_TLV (WFA_STA_SET_PSK_RESP_TLV + 0x01)
#define WFA_STA_SET_UAPSD_RESP_TLV (WFA_STA_SET_EAPTLS_RESP_TLV + 0x01)
#define WFA_STA_ASSOCIATE_RESP_TLV (WFA_STA_SET_UAPSD_RESP_TLV + 0x01)                 /* 59 */
#define WFA_STA_SET_EAPTTLS_RESP_TLV   (WFA_STA_ASSOCIATE_RESP_TLV + 0x01)             /* 60 */
#define WFA_STA_SET_EAPSIM_RESP_TLV   (WFA_STA_SET_EAPTTLS_RESP_TLV + 0x01)
#define WFA_STA_SET_PEAP_RESP_TLV      (WFA_STA_SET_EAPSIM_RESP_TLV + 0x01)
#define WFA_STA_SET_IBSS_RESP_TLV         (WFA_STA_SET_PEAP_RESP_TLV + 0x01)
#define WFA_STA_GET_INFO_RESP_TLV      (WFA_STA_SET_IBSS_RESP_TLV + 0x01)              /* 64 */
#define WFA_DEVICE_GET_INFO_RESP_TLV  (WFA_STA_GET_INFO_RESP_TLV + 0x01)
#define WFA_DEVICE_LIST_IF_RESP_TLV (WFA_DEVICE_GET_INFO_RESP_TLV + 0x01)              /* 66 */

#define WFA_STA_DEBUG_SET_RESP_TLV  (WFA_DEVICE_LIST_IF_RESP_TLV + 0x01)               /* 67 */
#define WFA_STA_SET_MODE_RESP_TLV   (WFA_STA_DEBUG_SET_RESP_TLV + 0x01)                /* 68 */
#define WFA_STA_UPLOAD_RESP_TLV     (WFA_STA_SET_MODE_RESP_TLV + 0x01)                 /* 69 */
#define WFA_STA_SET_WMM_RESP_TLV    (WFA_STA_UPLOAD_RESP_TLV + 0x01)                   /* 70 */
#define WFA_STA_REASSOCIATE_RESP_TLV  (WFA_STA_UPLOAD_RESP_TLV + 0x01)                 /* 71 */
#define WFA_STA_SET_PWRSAVE_RESP_TLV      (WFA_STA_REASSOCIATE_RESP_TLV + 0x01)        /* 72 */
#define WFA_STA_SEND_NEIGREQ_RESP_TLV    (WFA_STA_SET_PWRSAVE_RESP_TLV + 0x01)         /* 73 */

#define WFA_STA_RESPONSE_END (WFA_STA_SEND_NEIGREQ_RESP_TLV + 0x01)                    /* 74 */

#define WFA_TLV_END WFA_STA_RESPONSE_END
// New STA/DUT commands

#define WFA_STA_NEW_COMMANDS_START		0x100                                  /* 256 */
#define WFA_STA_PRESET_PARAMETERS_TLV	    (WFA_STA_NEW_COMMANDS_START + 0x01)        /* 257 */
#define WFA_STA_SET_EAPFAST_TLV		    (WFA_STA_PRESET_PARAMETERS_TLV + 0x01)     /* 258 */
#define WFA_STA_SET_EAPAKA_TLV		    (WFA_STA_SET_EAPFAST_TLV + 0x01)           /* 259 */
#define WFA_STA_SET_SYSTIME_TLV		    (WFA_STA_SET_EAPAKA_TLV + 0x01)            /* 260 */

#define WFA_STA_SET_11N_TLV		    (WFA_STA_SET_SYSTIME_TLV + 0x01)           /* 261 */
#define WFA_STA_SET_WIRELESS_TLV	    (WFA_STA_SET_11N_TLV + 0x01)               /* 262 */
#define WFA_STA_SEND_ADDBA_TLV		    (WFA_STA_SET_WIRELESS_TLV + 0x01)          /* 263 */

#define WFA_STA_SEND_COEXIST_MGMT_TLV       (WFA_STA_SEND_ADDBA_TLV + 0x01)            /* 264 */
#define WFA_STA_SET_RIFS_TEST_TLV           (WFA_STA_SEND_COEXIST_MGMT_TLV +0x01)      /* 265 */
#define WFA_STA_RESET_DEFAULT_TLV           (WFA_STA_SET_RIFS_TEST_TLV + 0x01)         /* 266 */
#define WFA_STA_DISCONNECT_TLV              (WFA_STA_RESET_DEFAULT_TLV +0x01)          /* 267 */

#define WFA_STA_NEW_COMMANDS_END            (WFA_STA_DISCONNECT_TLV + 0x01)            /* 268 */


//New STA/DUT command responses
#define WFA_STA_NEW_COMMANDS_RESPONSE_START	    0x200 /* 512 */
#define WFA_STA_PRESET_PARAMETERS_RESP_TLV  (WFA_STA_NEW_COMMANDS_RESPONSE_START + 0x01)   /* 513 */
#define WFA_STA_SET_EAPFAST_RESP_TLV	    (WFA_STA_PRESET_PARAMETERS_RESP_TLV + 0x01)    /* 514 */
#define WFA_STA_SET_EAPAKA_RESP_TLV	    (WFA_STA_SET_EAPFAST_RESP_TLV + 0x01)          /* 515 */
#define WFA_STA_SET_SYSTIME_RESP_TLV	    (WFA_STA_SET_EAPAKA_RESP_TLV + 0x01)           /* 516 */

#define WFA_STA_SET_11N_RESP_TLV    	    (WFA_STA_SET_SYSTIME_RESP_TLV + 0x01)          /* 517 */
#define WFA_STA_SET_WIRELESS_RESP_TLV	    (WFA_STA_SET_11N_RESP_TLV + 0x01)              /* 518 */
#define WFA_STA_SET_SEND_ADDBA_RESP_TLV	    (WFA_STA_SET_WIRELESS_RESP_TLV + 0x01)         /* 519 */

#define WFA_STA_SEND_COEXIST_MGMT_RESP_TLV  (WFA_STA_SET_SEND_ADDBA_RESP_TLV + 0x01)
#define WFA_STA_SET_RIFS_TEST_RESP_TLV      (WFA_STA_SEND_COEXIST_MGMT_RESP_TLV +0x01)
#define WFA_STA_RESET_DEFAULT_RESP_TLV      (WFA_STA_SET_RIFS_TEST_RESP_TLV + 0x01)
#define WFA_STA_DISCONNECT_RESP_TLV         (WFA_STA_RESET_DEFAULT_RESP_TLV + 0x01)

#define WFA_STA_NEW_COMMANDS_RESPONSE_END   (WFA_STA_DISCONNECT_RESP_TLV + 0x01)






#define WFA_TLV_HEAD_LEN 1+2

extern WORD wfaGetTag(BYTE *tlv_data);
extern void wfaSetTag(BYTE *tlv_data, BYTE new_tag);
extern WORD wfaGetTLVLen(BYTE *tlv_data);
extern WORD wfaGetValueLen(BYTE *tlv_data);
extern BOOL wfaGetValue(BYTE *pstr, int value_len, BYTE *tlv_data);
extern BOOL wfaIsValidTag(BYTE the_tag);
extern void wfaAliasByTag(BYTE the_tag, char *aliasStr);
extern BOOL wfaDecodeTLV(BYTE *tlv_data, int tlv_len, WORD *ptlv_tag, int *ptlv_val_len, BYTE *ptlv_value);
extern BOOL wfaEncodeTLV(WORD the_tag, WORD the_len, BYTE *the_value, BYTE *tlv_data);

extern WORD wfaGetValueType(BYTE the_tag, BYTE *tlv_data);

#endif
