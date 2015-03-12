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
 * File: wfa_cmdtbl.c
 *   The file contains a predefined function array. The command process and
 *   execution functions of a DUT traffic generator and control will be
 *   registered in the array/table by the order of the defined commands TLV
 *   values.
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "wfa_debug.h"
#include "wfa_types.h"
#include "wfa_main.h"
#include "wfa_tlv.h"
#include "wfa_tg.h"
#include "wfa_miscs.h"
#include "wfa_ca.h"
#include "wfa_sock.h"
#include "wfa_agt.h"
#include "wfa_rsp.h"

/* extern defined variables */
extern int gxcSockfd, btSockfd;

int NotDefinedYet(int len, unsigned char *params, int *respLen, BYTE *respBuf);
extern int agtCmdProcGetVersion(int len, BYTE *parms, int *respLen, BYTE *respBuf);

extern unsigned short wfa_defined_debug;

/* globally define the function table */
xcCommandFuncPtr gWfaCmdFuncTbl[WFA_STA_COMMANDS_END] =
{
    /* Traffic Agent Commands */
    NotDefinedYet,            /*    None                               (0) */
    agtCmdProcGetVersion,     /*    WFA_GET_VERSION_TLV                (1) */
    wfaTGSendPing,            /*    WFA_TRAFFIC_SEND_PING_TLV          (2) */
    wfaTGStopPing,            /*    WFA_TRAFFIC_STOP_PING_TLV          (3) */
    wfaTGConfig,              /*    WFA_TRAFFIC_AGENT_CONFIG_TLV       (4) */
    wfaTGSendStart,           /*    WFA_TRAFFIC_AGENT_SEND_TLV         (5) */
    wfaTGRecvStart,           /*    WFA_TRAFFIC_AGENT_RECV_START_TLV   (6) */
    wfaTGRecvStop,            /*    WFA_TRAFFIC_AGENT_RECV_STOP_TLV    (7) */
    wfaTGReset,               /*    WFA_TRAFFIC_AGENT_RESET_TLV        (8) */
    NotDefinedYet,            /*    WFA_TRAFFIC_AGENT_STATUS_TLV       (9) */

    /* Control and Configuration Commands */
    wfaStaGetIpConfig,        /*    WFA_STA_GET_IP_CONFIG_TLV          (10)*/
    wfaStaSetIpConfig,        /*    WFA_STA_SET_IP_CONFIG_TLV          (11)*/
    wfaStaGetMacAddress,      /*    WFA_STA_GET_MAC_ADDRESS_TLV        (12)*/
    wfaStaSetMacAddr,         /*    WFA_STA_SET_MAC_ADDRESS_TLV        (13)*/
    wfaStaIsConnected,        /*    WFA_STA_IS_CONNECTED_TLV           (14)*/
    wfaStaVerifyIpConnection, /*    WFA_STA_VERIFY_IP_CONNECTION_TLV   (15)*/
    wfaStaGetBSSID,           /*    WFA_STA_GET_BSSID_TLV              (16)*/
    wfaStaGetStats,           /*    WFA_STA_GET_STATS_TLV              (17)*/
    wfaSetEncryption,         /*    WFA_STA_SET_ENCRYPTION_TLV         (18)*/
    wfaStaSetPSK,             /*    WFA_STA_SET_PSK_TLV                (19)*/
    wfaStaSetEapTLS,          /*    WFA_STA_SET_EAPTLS_TLV             (20)*/
    wfaStaSetUAPSD,           /*    WFA_STA_SET_UAPSD_TLV              (21)*/
    wfaStaAssociate,          /*    WFA_STA_ASSOCIATE_TLV              (22)*/
    wfaStaSetEapTTLS,         /*    WFA_STA_SET_EAPTTLS_TLV            (23)*/
    wfaStaSetEapSIM,          /*    WFA_STA_SET_EAPSIM_TLV             (24)*/
    wfaStaSetPEAP,            /*    WFA_STA_SET_PEAP_TLV               (25)*/
    wfaStaSetIBSS,            /*    WFA_STA_SET_IBSS_TLV               (26)*/
    wfaStaGetInfo,            /*    WFA_STA_GET_INFO_TLV               (27)*/
    wfaDeviceGetInfo,         /*    WFA_DEVICE_GET_INFO_TLV            (28)*/
    wfaDeviceListIF,          /*    WFA_DEVICE_LIST_IF_TLV]            (29)*/
    wfaStaDebugSet,           /*    WFA_STA_DEBUG_SET                  (30)*/
    wfaStaSetMode,            /*    WFA_STA_SET_MODE                   (31)*/
    wfaStaUpload,             /*    WFA_STA_UPLOAD                     (32)*/
    wfaStaSetWMM,             /*    WFA_STA_SETWMM                     (33)*/
    wfaStaReAssociate,        /*    WFA_STA_REASSOCIATE                (34)*/
    wfaStaSetPwrSave,         /*    WFA_STA_SET_PWRSAVE                (35)*/
#ifdef WFA_STA_TB
    wfaStaSendNeigReq,        /*    WFA_STA_SEND_NEIGREQ               (36)*/
    wfaStaPresetParams,       /*    WFA_STA_PRESET_PARAMETERS          (37)*/
#else
    wfaStaTestBedCmd,         /*    WFA_STA_SEND_NEIGREQ               (36)*/
    wfaStaTestBedCmd,         /*    WFA_STA_PRESET_PARAMETERS          (37)*/
#endif
    wfaStaSetEapFAST,	      /*    WFA_STA_SET_EAPFAST_TLV	       (38)*/
    wfaStaSetEapAKA,	      /*    WFA_STA_SET_EAPAKA_TLV             (49)*/
    wfaStaSetSystime,	      /*    WFA_STA_SET_SYSTIME_TLV	       (40)*/
#ifdef WFA_STA_TB
    wfaStaSet11n,  	      /*    WFA_STA_SET_11n_TLV	    	       (41)*/
    wfaStaSetWireless,	      /*    WFA_STA_SET_WIRELESS_TLV	       (42)*/
    wfaStaSendADDBA,	      /*    WFA_STA_SEND_ADDBA_TLV	       (43)*/
    wfaStaSendCoExistMGMT,    /*    WFA_STA_SET_COEXIST_MGMT_TLV       (44)*/
    wfaStaSetRIFS,            /*    WFA_STA_SET_RIFS_TEST_TLV          (45)*/
    wfaStaResetDefault,       /*    WFA_STA_RESET_DEFAULT_TLV          (46)*/
    wfaStaDisconnect,         /*    WFA_STA_DISCONNECT_TLV             (47)*/
#else
    wfaStaTestBedCmd,  	      /*    WFA_STA_SET_11n_TLV	    	       (41)*/
    wfaStaTestBedCmd,	      /*    WFA_STA_SET_WIRELESS_TLV	       (42)*/
    wfaStaTestBedCmd,	      /*    WFA_STA_SEND_ADDBA_TLV	       (43)*/
    wfaStaTestBedCmd,         /*    WFA_STA_SET_COEXIST_MGMT_TLV       (44)*/
    wfaStaTestBedCmd,         /*    WFA_STA_SET_RIFS_TEST_TLV          (45)*/
    wfaStaTestBedCmd,         /*    WFA_STA_RESET_DEFAULT_TLV          (46)*/
    wfaStaDisconnect,         /*    WFA_STA_DISCONNECT_TLV             (47)*/
#endif
    wfaStaDevSendFrame,          /*    WFA_STA_DEV_SEND_FRAME_TLV              (48)*/
    wfaStaSetSecurity,        /*    WFA_STA_SET_SECURITY_TLV           (49)*/
    wfaStaGetP2pDevAddress,   /*    WFA_STA_GET_P2P_DEV_ADDRESS_TLV    (50)*/
    wfaStaSetP2p,	      /*    WFA_STA_SET_P2P_TLV	               (51)*/
    wfaStaP2pConnect,         /*    WFA_STA_P2P_CONNECT_TLV            (52)*/
    wfaStaStartAutoGo, 	      /* WFA_STA_START_AUTO_GO                 (53)*/
    wfaStaP2pStartGrpFormation, /*    WFA_STA_P2P_START_GRP_FORMATION_TLV      (54)*/

    wfaStaP2pDissolve,          /*    WFA_STA_P2P_DISSOLVE_TLV                 (55)*/
    wfaStaSendP2pInvReq,        /*    WFA_STA_SEND_P2P_INV_REQ_TLV             (56)*/
    wfaStaAcceptP2pInvReq,	/*    WFA_STA_ACCEPT_P2P_INV_REQ_TLV           (57)*/
    wfaStaSendP2pProvDisReq,    /*    WFA_STA_SEND_P2P_PROV_DIS_REQ_TLV    (58)*/
    wfaStaSetWpsPbc,            /*    WFA_STA_SET_WPS_PBC_TLV              (59)*/

    wfaStaWpsReadPin,           /*    WFA_STA_WPS_READ_PIN_TLV             (60)*/
    wfaStaWpsEnterPin,          /*    WFA_STA_WPS_ENTER_PIN_TLV	           (61)*/
    wfaStaGetPsk,               /*    WFA_STA_GET_PSK_TLV                  (62)*/
    wfaStaP2pReset,             /*    WFA_STA_P2P_RESET_TLV                (63)*/
    wfaStaWpsReadLabel,         /*    WFA_STA_WPS_READ_LABEL_TLV           (64)*/
    wfaStaGetP2pIpConfig,       /*    WFA_STA_GET_P2P_IP_CONFIG_TLV        (65)*/
    wfaStaSendServiceDiscoveryReq,    /*    WFA_STA_SEND_SERVICE_DISCOVERY_REQ_TLV                (66)*/
    wfaStaSendP2pPresenceReq,         /*    WFA_STA_SEND_P2P_PRESENCE_REQ_TLV                     (67)*/
    wfaStaSetSleepReq,                /*    WFA_STA_SEND_P2P_PRESENCE_REQ_TLV                     (68)*/
    wfaStaSetOpportunisticPsReq,             /*    WFA_STA_SET_OPPORTUNISTIC_PS_TLV               (69)*/

    wfaStaAddArpTableEntry,    /*    WFA_STA_ADD_ARP_TABLE_ENTRY_TLV      (70)*/
    wfaStaBlockICMPResponse,    /*    WFA_STA_BLOCK_ICMP_RESPONSE_TLV      (71)*/
    wfaStaSetRadio,           /*    WFA_STA_SET_RADIO_TLV              (72)*/
    wfaStaSetRFeature,        /*    WFA_STA_RFEATURE_TLV               (73)*/

	wfaStaStartWfdConnection,   /*    WFA_STA_START_WFD_CONNECTION_TLV               (74)*/
	wfaStaCliCommand,         /*   WFA_STA_CLI_CMD_TLV            (75)*/
	wfaStaConnectGoStartWfd,   /*    WFA_STA_CONNECT_GO_START_WFD_TLV               (76)*/
	wfaStaGenerateEvent,         /*   WFA_STA_GENERATE_EVENT_TLV            (77)*/
	wfaStaReinvokeWfdSession,         /*   WFA_STA_REINVOKE_WFD_SESSION_TLV            (78)*/
	wfaStaGetParameter,         /*   WFA_STA_GET_PARAMETER_TLV            (79)*/
	wfaStaNfcAction,         /*   WFA_STA_NFC_ACTION_TLV            (80)*/
	wfaStaInvokeCommand,         /*   WFA_STA_INVOKE_COMMAND_TLV            (81)*/
	wfaStaManageService,         /*   WFA_STA_MANAGE_SERVICE_TLV            (82)*/	
	wfaStaGetEvents,         /*   WFA_STA_GET_EVENTS_TLV            (83)*/
	wfaStaGetEventDetails,         /*   WFA_STA_GET_EVENT_DETAILS_TLV            (84)*/	

};


/*
 * NotDefinedYet(): a dummy function
 */
int NotDefinedYet(int len, unsigned char *params, int *respLen, BYTE *respBuf)
{
    DPRINT_WARNING(WFA_WNG, "The command processing function not defined.\n");


    /* need to send back a response */

    return WFA_SUCCESS;
}
