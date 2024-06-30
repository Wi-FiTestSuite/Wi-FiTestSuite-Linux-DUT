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

enum cmd_tags
{
    /* Commands */
    WFA_GET_VERSION_TLV =                 0x01,

    /* Generic TRAFFIC GENERATOR Commands */
    WFA_TRAFFIC_SEND_PING_TLV,            /* 2 */
    WFA_TRAFFIC_STOP_PING_TLV,            /* 3 */
    WFA_TRAFFIC_AGENT_CONFIG_TLV,         /* 4 */
    WFA_TRAFFIC_AGENT_SEND_TLV,           /* 5 */
    WFA_TRAFFIC_AGENT_RECV_START_TLV,     /* 6 */
    WFA_TRAFFIC_AGENT_RECV_STOP_TLV,      /* 7 */
    WFA_TRAFFIC_AGENT_RESET_TLV,          /* 8 */
    WFA_TRAFFIC_AGENT_STATUS_TLV,         /* 9 */

    /* STATION/DUT Commands */
    WFA_STA_GET_IP_CONFIG_TLV,            /* 10 */
    WFA_STA_SET_IP_CONFIG_TLV,            /* 11 */
    WFA_STA_GET_MAC_ADDRESS_TLV,          /* 12 */
    WFA_STA_SET_MAC_ADDRESS_TLV,          /* 13 */
    WFA_STA_IS_CONNECTED_TLV,             /* 14 */
    WFA_STA_VERIFY_IP_CONNECTION_TLV,     /* 15 */
    WFA_STA_GET_BSSID_TLV,                /* 16 */
    WFA_STA_GET_STATS_TLV,                /* 17 */
    WFA_STA_SET_ENCRYPTION_TLV,           /* 18 */
    WFA_STA_SET_PSK_TLV,                  /* 19 */
    WFA_STA_SET_EAPTLS_TLV,               /* 20 */
    WFA_STA_SET_UAPSD_TLV,                /* 21 */
    WFA_STA_ASSOCIATE_TLV,                /* 22 */
    WFA_STA_SET_EAPTTLS_TLV,              /* 23 */
    WFA_STA_SET_EAPSIM_TLV,               /* 24 */
    WFA_STA_SET_PEAP_TLV,                 /* 25 */
    WFA_STA_SET_IBSS_TLV,                 /* 26 */
    WFA_STA_GET_INFO_TLV,                 /* 27 */
    WFA_DEVICE_GET_INFO_TLV,              /* 28 */
    WFA_DEVICE_LIST_IF_TLV,               /* 29 */

    WFA_STA_DEBUG_SET_TLV,                /* 30 */
    WFA_STA_SET_MODE_TLV,                 /* 31 */
    WFA_STA_UPLOAD_TLV,                   /* 32 */
    WFA_STA_SET_WMM_TLV,                  /* 33 */
    WFA_STA_REASSOCIATE_TLV,              /* 34 */
    WFA_STA_SET_PWRSAVE_TLV,              /* 35 */
    WFA_STA_SEND_NEIGREQ_TLV,             /* 36 */

    WFA_STA_PRESET_PARAMETERS_TLV,        /* 37 */
    WFA_STA_SET_EAPFAST_TLV,              /* 38 */
    WFA_STA_SET_EAPAKA_TLV,               /* 39 */
    WFA_STA_SET_SYSTIME_TLV,              /* 40 */

    WFA_STA_SET_11N_TLV,                  /* 41 */
    WFA_STA_SET_WIRELESS_TLV,             /* 42 */
    WFA_STA_SEND_ADDBA_TLV,               /* 43 */

    WFA_STA_SEND_COEXIST_MGMT_TLV,        /* 44 */
    WFA_STA_SET_RIFS_TEST_TLV,            /* 45 */
    WFA_STA_RESET_DEFAULT_TLV,            /* 46 */
    WFA_STA_DISCONNECT_TLV,               /* 47 */
    WFA_STA_DEV_SEND_FRAME_TLV,           /* 48 */
    WFA_STA_SET_SECURITY_TLV,             /* 49 */
    /* P2P */
    WFA_STA_P2P_GET_DEV_ADDRESS_TLV,      /* 50 */
    WFA_STA_P2P_SETP2P_TLV,               /* 51 */
    WFA_STA_P2P_CONNECT_TLV,              /* 52 */
    WFA_STA_P2P_START_AUTO_GO_TLV,        /* 53 */
    WFA_STA_P2P_START_GRP_FORMATION_TLV,  /* 54 */
    WFA_STA_P2P_DISSOLVE_TLV,             /* 55 */
    WFA_STA_P2P_SEND_INV_REQ_TLV,         /* 56 */
    WFA_STA_P2P_ACCEPT_INV_REQ_TLV,       /* 57 */
    WFA_STA_P2P_SEND_PROV_DIS_REQ_TLV,    /* 58 */
    WFA_STA_WPS_SETWPS_PBC_TLV,           /* 59 */
    WFA_STA_WPS_READ_PIN_TLV,             /* 60 */
    WFA_STA_WPS_ENTER_PIN_TLV,            /* 61 */
    WFA_STA_P2P_GET_PSK_TLV,              /* 62 */
    WFA_STA_P2P_RESET_TLV,                /* 63 */
    WFA_STA_WPS_READ_LABEL_TLV,           /* 64 */
    WFA_STA_P2P_GET_IP_CONFIG_TLV,        /* 65 */
    WFA_STA_P2P_SEND_SERVICE_DISCOVERY_REQ_TLV, /* 66 */
    WFA_STA_P2P_SEND_PRESENCE_REQ_TLV,    /* 67 */
    WFA_STA_P2P_SET_SLEEP_TLV,            /* 68 */
    WFA_STA_P2P_SET_OPPORTUNISTIC_PS_TLV, /* 69 */

    WFA_STA_P2P_ADD_ARP_TABLE_ENTRY_TLV,      /* 70 */
    WFA_STA_P2P_BLOCK_ICMP_RESPONSE_TLV,      /* 71 */

/*TDLS or PMF */
   	WFA_STA_SET_RADIO_TLV,                /* 72 */
   	WFA_STA_SET_RFEATURE_TLV,             /* 73 */
/*Display */
   	WFA_STA_START_WFD_CONNECTION_TLV,	/* 74 */
   	WFA_STA_CLI_CMD_TLV,					/* 75 */
   	WFA_STA_CONNECT_GO_START_WFD_TLV,	 /* 76 */
   	WFA_STA_GENERATE_EVENT_TLV,	 		/* 77 */
   	WFA_STA_REINVOKE_WFD_SESSION_TLV,	/* 78 */
   	WFA_STA_GET_PARAMETER_TLV,	/* 79 */
/*P2P NFC*/
   	WFA_STA_NFC_ACTION_TLV,	/* 80 */

/*WFDS*/
	WFA_STA_INVOKE_COMMAND_TLV, /* 81 */
	WFA_STA_MANAGE_SERVICE_TLV, /* 82 */
	WFA_STA_GET_EVENTS_TLV, /* 83 */
	WFA_STA_GET_EVENT_DETAILS_TLV, /* 84 */
	
   WFA_STA_SET_EAPAKAPRIME_TLV,           /* 80 */
   WFA_STA_SET_EAPPWD_TLV,                /* 81 */
   WFA_STA_COMMANDS_END,                  /* 82 */
  
   WFA_STA_EXEC_ACTION_TLV,			/* 86 */
   WFA_STA_SCAN_TLV, /* 87 */
};


enum resp_tags
{
    /* Version response */
    WFA_GET_VERSION_RESP_TLV = 0x01,            /* 01 */

    /* Generic Traffic Generator Responses */
    WFA_TRAFFIC_SEND_PING_RESP_TLV,             /* 02 */
    WFA_TRAFFIC_STOP_PING_RESP_TLV,             /* 03 */
    WFA_TRAFFIC_AGENT_CONFIG_RESP_TLV,          /* 04 */
    WFA_TRAFFIC_AGENT_SEND_RESP_TLV,            /* 05 */
    WFA_TRAFFIC_AGENT_RECV_START_RESP_TLV,      /* 06 */
    WFA_TRAFFIC_AGENT_RECV_STOP_RESP_TLV,       /* 07 */
    WFA_TRAFFIC_AGENT_RESET_RESP_TLV,           /* 08 */
    WFA_TRAFFIC_AGENT_STATUS_RESP_TLV,          /* 09 */

    /* STATION/DUT Responses */
    WFA_STA_GET_IP_CONFIG_RESP_TLV,             /* 10 */
    WFA_STA_SET_IP_CONFIG_RESP_TLV,             /* 11 */
    WFA_STA_GET_MAC_ADDRESS_RESP_TLV,           /* 12 */
    WFA_STA_SET_MAC_ADDRESS_RESP_TLV,           /* 13 */
    WFA_STA_IS_CONNECTED_RESP_TLV,              /* 14 */
    WFA_STA_VERIFY_IP_CONNECTION_RESP_TLV,      /* 15 */
    WFA_STA_GET_BSSID_RESP_TLV,                 /* 16 */
    WFA_STA_GET_STATS_RESP_TLV,                 /* 17 */
    WFA_STA_SET_ENCRYPTION_RESP_TLV,            /* 18 */
    WFA_STA_SET_PSK_RESP_TLV,                   /* 19 */
    WFA_STA_SET_EAPTLS_RESP_TLV,                /* 20 */
    WFA_STA_SET_UAPSD_RESP_TLV,                 /* 21 */
    WFA_STA_ASSOCIATE_RESP_TLV,                 /* 22 */
    WFA_STA_SET_EAPTTLS_RESP_TLV,               /* 23 */
    WFA_STA_SET_EAPSIM_RESP_TLV,                /* 24 */
    WFA_STA_SET_PEAP_RESP_TLV,                  /* 25 */
    WFA_STA_SET_IBSS_RESP_TLV,                  /* 26 */
    WFA_STA_GET_INFO_RESP_TLV,                  /* 27 */
    WFA_DEVICE_GET_INFO_RESP_TLV,               /* 28 */
    WFA_DEVICE_LIST_IF_RESP_TLV,                /* 29 */

    WFA_STA_DEBUG_SET_RESP_TLV,                 /* 30 */
    WFA_STA_SET_MODE_RESP_TLV,                  /* 31 */
    WFA_STA_UPLOAD_RESP_TLV,                    /* 32 */
    WFA_STA_SET_WMM_RESP_TLV,                   /* 33 */
    WFA_STA_REASSOCIATE_RESP_TLV,               /* 34 */
    WFA_STA_SET_PWRSAVE_RESP_TLV,               /* 35 */
    WFA_STA_SEND_NEIGREQ_RESP_TLV,              /* 36 */

    WFA_STA_PRESET_PARAMETERS_RESP_TLV,         /* 37 */
    WFA_STA_SET_EAPFAST_RESP_TLV,               /* 38 */
    WFA_STA_SET_EAPAKA_RESP_TLV,	               /* 39 */
    WFA_STA_SET_SYSTIME_RESP_TLV,               /* 40 */

    WFA_STA_SET_11N_RESP_TLV,                   /* 41 */
    WFA_STA_SET_WIRELESS_RESP_TLV,              /* 42 */
    WFA_STA_SET_SEND_ADDBA_RESP_TLV,            /* 43 */

    WFA_STA_SEND_COEXIST_MGMT_RESP_TLV,         /* 44 */
    WFA_STA_SET_RIFS_TEST_RESP_TLV,             /* 45 */
    WFA_STA_RESET_DEFAULT_RESP_TLV,             /* 46 */
    WFA_STA_DISCONNECT_RESP_TLV,                /* 47 */
    WFA_STA_DEV_SEND_FRAME_RESP_TLV,            /* 48 */
    WFA_STA_SET_SECURITY_RESP_TLV,              /* 49 */
    /* P2P */
    WFA_STA_P2P_GET_DEV_ADDRESS_RESP_TLV,      /* 50 */
    WFA_STA_P2P_SETP2P_RESP_TLV,               /* 51 */
    WFA_STA_P2P_CONNECT_RESP_TLV,              /* 52 */
    WFA_STA_P2P_START_AUTO_GO_RESP_TLV,        /* 53 */
    WFA_STA_P2P_START_GRP_FORMATION_RESP_TLV,  /* 54 */
    WFA_STA_P2P_DISSOLVE_RESP_TLV,             /* 55 */
    WFA_STA_P2P_SEND_INV_REQ_RESP_TLV,         /* 56 */
    WFA_STA_P2P_ACCEPT_INV_REQ_RESP_TLV,       /* 57 */
    WFA_STA_P2P_SEND_PROV_DIS_REQ_RESP_TLV,    /* 58 */
    WFA_STA_WPS_SETWPS_PBC_RESP_TLV,           /* 59 */
    WFA_STA_WPS_READ_PIN_RESP_TLV,             /* 60 */
    WFA_STA_WPS_ENTER_PIN_RESP_TLV,            /* 61 */
    WFA_STA_P2P_GET_PSK_RESP_TLV,              /* 62 */
    WFA_STA_P2P_RESET_RESP_TLV,                /* 63 */
    WFA_STA_WPS_READ_LABEL_RESP_TLV,           /* 64 */
    WFA_STA_P2P_GET_IP_CONFIG_RESP_TLV,        /* 65 */
    WFA_STA_P2P_SEND_SERVICE_DISCOVERY_REQ_RESP_TLV, /* 66 */
    WFA_STA_P2P_SEND_PRESENCE_REQ_RESP_TLV,    /* 67 */
    WFA_STA_P2P_SET_SLEEP_RESP_TLV,            /* 68 */
    WFA_STA_P2P_SET_OPPORTUNISTIC_PS_RESP_TLV, /* 69 */

    WFA_STA_P2P_ADD_ARP_TABLE_ENTRY_RESP_TLV,  /* 70 */
    WFA_STA_P2P_BLOCK_ICMP_RESPONSE_RESP_TLV,  /* 71 */
    WFA_STA_SET_RADIO_RESP_TLV,                 /* 72 */
    WFA_STA_SET_RFEATURE_RESP_TLV,              /* 73 */

    WFA_STA_START_WFD_CONNECTION_RESP_TLV,      /* 74 */
    WFA_STA_CLI_CMD_RESP_TLV,					/* 75 */
    WFA_STA_CONNECT_GO_START_WFD_RESP_TLV,      /* 76 */
	WFA_STA_GENERATE_EVENT_RESP_TLV,			/* 77 */
	WFA_STA_REINVOKE_WFD_SESSION_RESP_TLV,		/* 78 */
	WFA_STA_GET_PARAMETER_RESP_TLV,		/* 79 */
	
	WFA_STA_NFC_ACTION_RESP_TLV,		/* 80 */
	
	WFA_STA_INVOKE_CMD_RESP_TLV,		/* 81 */
	WFA_STA_MANAGE_SERVICE_RESP_TLV,		/* 82 */
	WFA_STA_GET_EVENTS_RESP_TLV,		/* 83 */
	WFA_STA_GET_EVENT_DETAILS_RESP_TLV,		/* 84 */
    WFA_STA_SET_EAPAKAPRIME_RESP_TLV,              /* 80 */
    WFA_STA_SET_EAPPWD_RESP_TLV,                   /* 81 */
    WFA_STA_RESPONSE_END,                        /* 82 */
	WFA_STA_EXEC_ACTION_RESP_TLV,					/* 86 */
	WFA_STA_SCAN_RESP_TLV, 					/* 87 */
};

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
