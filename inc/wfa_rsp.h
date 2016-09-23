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


#ifndef WFA_RSP_H
#define WFA_RSP_H

#include "wfa_ver.h"

#ifdef WFA_WMM_VOICE
#define WFA_INFO_BUFSIZE       128   /* used to upload test data */
#else
#define WFA_INFO_BUFSIZE       128
#endif

typedef struct _tg_ping_stop_resp
{
    int sendCnt;
    int repliedCnt;
} tgPingStopResp_t;

typedef struct ca_sta_get_ipconfig_resp
{
    int isDhcp;
    char ipaddr[WFA_IP_ADDR_STR_LEN];
    char mask[WFA_IP_MASK_STR_LEN];
    char dns[WFA_MAX_DNS_NUM][WFA_IP_ADDR_STR_LEN];
    char mac[WFA_MAC_ADDR_STR_LEN];

} caStaGetIpConfigResp_t;

typedef struct ca_sta_get_stats_resp
{
    int status;
    int txFrames;
    int rxFrames;
    int txMulticast;
    int rxMulticast;
    int fcsErrors ;
    int txRetries;
} caStaGetStatsResp_t;

typedef struct ca_device_get_info_resp
{
    char vendor[16];
    char model[16];
    char version[WFA_VERNAM_LEN];
    char firmware[16];
} caDeviceGetInfoResp_t;

typedef struct ca_sta_upload_resp
{
    short seqnum;
    short nbytes;
    char bytes[256];
} caStaUploadResp_t;

typedef struct ca_device_list_if_resp
{
    int status;
#define IF_80211   1
#define IF_ETH     2
    int iftype;
    char ifs[3][16];
} caDeviceListIFResp_t;

typedef struct ca_sta_cli_command_resp
{
    int status;
    short resFlag;
    char result[WFA_CLI_CMD_RESP_LEN];
} caStaCliCmdResp_t;
/* P2P */
typedef struct ca_P2p_sta_get_psk_resp
{
    char ssid[WFA_SSID_NAME_LEN];
    char passPhrase[WFA_PSK_PP_LEN];
} caP2pStaGetPskResp_t;

typedef struct ca_P2p_start_grp_form_resp
{
    char result[8];
    char grpId[WFA_P2P_GRP_ID_LEN];
} caP2pStartGrpFormResp_t;
/* P2P */

/* WFD */
typedef struct ca_sta_start_wfd_conn_resp
{
    char result[8];
    char wfdSessionId[WFA_WFD_SESSION_ID_LEN];
    char p2pGrpId[WFA_P2P_GRP_ID_LEN];
} caStaStartWfdConnResp_t;

typedef struct ca_sta_get_parameter_resp
{

    BYTE getParamType;
    char devList[128];
	char masterPref[8];
} caStaGetParameterResp_t;

/* WFD */

/* NFC */

typedef struct ca_sta_nfc_action_resp
{
   char result[8];
   char grpId[WFA_P2P_GRP_ID_LEN];
   int peerRole;   
} caStaNfcActionResp_t;
/* NFC */

/* WFDS */

typedef struct wfds_serviceAdv_info
{
	char servName[32];
	long int advtID;
	char serviceMac[WFA_P2P_DEVID_LEN];	
} wfdsServAdvInfo_t;


typedef struct ca_sta_ConnSess_cmd_resp
{
	long int sessionID;
	char result[8];
	char grpId[WFA_P2P_GRP_ID_LEN];

} caStaConnSessCmdResp_t;



enum {
	eServiceAvilable= 0,
	eServiceNotAvailable=1,
};

typedef struct ca_sta_SearchResult_Event
{
	long int searchID;
	char serviceMac[WFA_MAC_ADDR_STR_LEN];	 
	long int advID; 
	char serviceName[32];	 
	WORD serviceStatus;
} caStaSearchResultEvent_t;

typedef struct ca_sta_SearchTerminated_Event
{
	long int searchID;
} caStaSearchTerminatedEvent_t;

enum {
	eAdvertised= 1,
	eNotAdvertised,
};

typedef struct ca_sta_AdvStatus_Event
{
	long int advID; 
	WORD status;
} caStaAdvertiseStatusEvent_t;


typedef struct ca_sta_SessionRequest_Event
{
	long int advID; 
	char sessionMac[WFA_MAC_ADDR_STR_LEN];	 
	long int sessionID;
} caStaSessionRequestEvent_t;

enum {
	eSessionStateOpen= 1,
	eSessionStateInitiated,
	eSessionStateRequested,
	eSessionStateClosed,	
};

typedef struct ca_sta_SessionStatus_Event
{
	long int sessionID; 
	char sessionMac[WFA_MAC_ADDR_STR_LEN];	 
	WORD state;
} caStaSessionStatusEvent_t;
enum {
	eNetworkRoleRejected= 1,
	eServiceRequestReceived,
	eServiceRequestDifferred,
	eServiceRequestAccepted,
	eServiceRequestFailed,
	eGroupFormationStarted,
	eGroupFormationComplete,
	eGroupFormationFailed,
};

typedef struct ca_sta_ConnectStatus_Event
{
	long int sessionID; 
	char sessionMac[WFA_MAC_ADDR_STR_LEN];	 
	WORD status;
} caStaConnectStatusEvent_t;

enum {
	eLocalPortAllowed= 1,
	eLocalPortBlocked,
	eFailure,
	eRemotePortAllowed,	
};

typedef struct ca_sta_PortStatus_Event
{
	long int sessionID; 
	char sessionMac[WFA_MAC_ADDR_STR_LEN];	 
	WORD port;
	WORD status;
} caStaPortStatusEvent_t;


typedef struct ca_sta_GetEventDetails_cmd_resp
{
	WORD eventID;
	union _EventType
	{
		caStaSearchResultEvent_t searchResult;
		caStaSearchTerminatedEvent_t searchTerminated;
		caStaAdvertiseStatusEvent_t advStatus;
		caStaSessionRequestEvent_t sessionReq;
		caStaSessionStatusEvent_t sessionStatus;
		caStaConnectStatusEvent_t connStatus;
		caStaPortStatusEvent_t portStatus;
	}getEventDetails;

} caStaGetEventDetailsCmdResp_t;


typedef struct ca_sta_GetEvents_cmd_resp
{
	char result[512];
} caStaGetEventListCmdResp_t;


typedef struct ca_sta_invoke_Seek_cmd_resp
{
	long int searchID;
} caStaInvokeSeekCmdResp_t;
typedef struct ca_sta_invoke_Advrt_cmd_resp
{
	int numServInfo;
	wfdsServAdvInfo_t servAdvInfo[5];
} caStaInvokeAdvrtCmdResp_t;

typedef struct ca_sta_invoke_cmd_resp
{
	int invokeCmdRspType;
	union _wfdsInvokeCmd
	{
		caStaInvokeAdvrtCmdResp_t advRsp;
		caStaInvokeSeekCmdResp_t seekRsp;
		caStaConnSessCmdResp_t connSessResp;

	}invokeCmdResp;
 
} caStaInvokeCmdResp_t;
/* WFDS */

typedef struct ca_sta_exec_action_resp
{
	char mac[18];
} caStaExecActionResp_t;

typedef struct ca_sta_get_events_resp
{
	char eventName[64];
	unsigned int remoteInstanceID;
	unsigned int localInstanceID;
	char mac[18];
} caStaGetEventsResp_t;

typedef struct dut_cmd_response
{
    int status;
    int streamId;
    union _cmdru
    {
        tgStats_t stats;
        tgPingStopResp_t pingStp;
        caStaGetIpConfigResp_t getIfconfig;
        caStaGetStatsResp_t ifStats;
        caDeviceGetInfoResp_t devInfo;
        caDeviceListIFResp_t ifList;
        caStaUploadResp_t  uld;
	   caStaGetEventsResp_t getEvents;
	   caStaExecActionResp_t execAction;
        char version[WFA_VERSION_LEN];
        char info[WFA_INFO_BUFSIZE];
        char bssid[WFA_MAC_ADDR_STR_LEN];
        char mac[WFA_MAC_ADDR_STR_LEN];
        /* P2P */
        char devid[WFA_P2P_DEVID_LEN];
        char grpid[WFA_P2P_GRP_ID_LEN];
        char p2presult[8];
        char wpsPin[WFA_WPS_PIN_LEN];
        caP2pStaGetPskResp_t pskInfo;
        caP2pStartGrpFormResp_t grpFormInfo;
        /* P2P */
        /* WFD */
        caStaStartWfdConnResp_t	wfdConnInfo;
        caStaGetParameterResp_t getParamValue;

/* WFD */
/* NFC */
	   caStaNfcActionResp_t staNfcAction;
/* NFC */
/* WFDS */
	   caStaInvokeCmdResp_t staInvokeCmd;
	   caStaConnSessCmdResp_t staManageServ;
	   caStaGetEventListCmdResp_t staGetEvents;	   
	   caStaGetEventDetailsCmdResp_t staGetEventDetails;	   

/* WFDS */

       int connected;
   } cmdru;
}dutCmdResponse_t;

#endif
