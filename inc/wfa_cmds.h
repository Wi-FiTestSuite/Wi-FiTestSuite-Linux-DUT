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
 * wfa_cmds.h:
 *   definitions of command types.
 */
#ifndef _WFA_CMDS_H
#define _WFA_CMDS_H

typedef struct _tg_ping_start
{
    char dipaddr[IPV6_ADDRESS_STRING_LEN];  /* destination/remote ip address */
    int  frameSize;
    float  frameRate;
    int  duration;
    int  type;
    int  qos;
    int  iptype;
    int  dscp;
} tgPingStart_t;

typedef struct ca_sta_set_ip_config
{
    char intf[WFA_IF_NAME_LEN];
    int isDhcp;
    char ipaddr[WFA_IP_ADDR_STR_LEN];
    char mask[WFA_IP_MASK_STR_LEN];
    char defGateway[WFA_IP_ADDR_STR_LEN];
    char pri_dns[WFA_IP_ADDR_STR_LEN];
    char sec_dns[WFA_IP_ADDR_STR_LEN];
} caStaSetIpConfig_t;

typedef struct ca_sta_verify_ip_connection
{
    char dipaddr[WFA_IP_ADDR_STR_LEN];
    int timeout;
} caStaVerifyIpConnect_t;

typedef struct ca_sta_set_encryption
{
    char intf[WFA_IF_NAME_LEN];
    char ssid[WFA_SSID_NAME_LEN];
    int encpType;
    char keys[4][32];  /* 26 hex */
    int activeKeyIdx;
} caStaSetEncryption_t;

typedef enum wfa_enableType
{
    eEnable = 1,
    eDisable
} wfaEnableType;

typedef struct ca_sta_set_mode
{
    char intf[WFA_IF_NAME_LEN];
    char ssid[WFA_SSID_NAME_LEN];
    char mode;
    int encpType;
    int channel;
    char keys[4][32];  /* 26 hex */
    int activeKeyIdx;
} caStaSetMode_t;

typedef struct ca_sta_set_psk
{
    char intf[WFA_IF_NAME_LEN];
    char ssid[WFA_SSID_NAME_LEN];
    BYTE passphrase[64];
    char keyMgmtType[16];  /* WPA-PSK */
    int encpType;    /* TKIP    */
    int pmf;               /* PMF enable or disable */
    char micAlg[16];
    char prog[16];
    BOOL prefer;
} caStaSetPSK_t;

typedef struct ca_sta_set_eaptls
{
    char intf[WFA_IF_NAME_LEN];
    char ssid[WFA_SSID_NAME_LEN];
    char username[32];
    char keyMgmtType[8];
    char encrptype[9];
    char trustedRootCA[128];
    char clientCertificate[128];
    int pmf;               /* PMF enable or disable */
    char micAlg[16];
} caStaSetEapTLS_t;

typedef struct ca_sta_set_eapttls
{
    char intf[WFA_IF_NAME_LEN];
    char ssid[WFA_SSID_NAME_LEN];
    char username[32];
    char passwd[16];
    char keyMgmtType[8];
    char encrptype[9];
    char trustedRootCA[32];
    char clientCertificate[32];
    int pmf;               /* PMF enable or disable */
    char micAlg[16];
    char prog[16];
    BOOL prefer;
} caStaSetEapTTLS_t;

typedef struct ca_sta_set_eapsim
{
    char intf[WFA_IF_NAME_LEN];
    char ssid[WFA_SSID_NAME_LEN];
    char username[32];
    char passwd[96];
    char keyMgmtType[8];
    char encrptype[9];
    char	tripletCount;
    char tripletSet[3][64];
    int pmf;               /* PMF enable or disable */
} caStaSetEapSIM_t;

typedef struct ca_sta_set_eappeap
{
    char intf[WFA_IF_NAME_LEN];
    char ssid[WFA_SSID_NAME_LEN];
    char username[32];
    char passwd[16];
    char keyMgmtType[8];
    char encrptype[9];
    char trustedRootCA[32];
    char innerEAP[16];
    int peapVersion;
    int pmf;               /* PMF enable or disable */
} caStaSetEapPEAP_t;

typedef struct ca_sta_set_eapfast
{
    char intf[WFA_IF_NAME_LEN];
    char ssid[WFA_SSID_NAME_LEN];
    char username[32];
    char passwd[16];
    char keyMgmtType[8];
    char encrptype[9];
    char trustedRootCA[32];
    char innerEAP[16];
    char	validateServer;
    char pacFileName[32];
    int pmf;               /* PMF enable or disable */
} caStaSetEapFAST_t;

typedef struct ca_sta_set_eapaka
{
    char intf[WFA_IF_NAME_LEN];
    char ssid[WFA_SSID_NAME_LEN];
    char username[64];
    char passwd[96];
    char keyMgmtType[8];
    char encrptype[9];
    char	tripletCount;
    char tripletSet[3][96];
    int pmf;               /* PMF enable or disable */
} caStaSetEapAKA_t;

enum sectype {
    SEC_TYPE_PSK = 1,
    SEC_TYPE_EAPTLS,
    SEC_TYPE_EAPTTLS,
    SEC_TYPE_EAPPEAP,
    SEC_TYPE_EAPSIM,
    SEC_TYPE_EAPFAST,
    SEC_TYPE_EAPAKA,
};

typedef struct ca_sta_set_security
{
    BYTE type; /* PSK, EAPx */
    char ssid[WFA_SSID_NAME_LEN];
    char keyMgmtType[8];
    char encpType[9];
    int  pmf;
    union _security
    {
        char                 passphrase[64];       /* PSK */
        caStaSetEapTLS_t     tls;
        caStaSetEapTTLS_t    ttls;
        caStaSetEapSIM_t     sim;
        caStaSetEapPEAP_t    peap;
        caStaSetEapAKA_t     aka;
        caStaSetEapFAST_t    fast;
    } secu;
} caStaSetSecurity_t;

typedef struct ca_sta_set_systime
{
    BYTE month;
    BYTE date;
    WORD year;
    BYTE hours;
    BYTE minutes;
    BYTE seconds;
} caStaSetSystime_t;


/* DEV_SEND_FRAME  related definitions    */
/*  DEV_SEND_FRAME    LOC   */
enum
{
    LOC_TYPE_ANQPQUERY = 1,
    LOC_TYPE_NeighReportReq,
    LOC_TYPE_RadioMsntReq,
};

/*  DEV_SEND_FRAME    PMF   */
enum
{
    PMF_TYPE_DISASSOC = 1,
    PMF_TYPE_DEAUTH,
    PMF_TYPE_SAQUERY,
    PMF_TYPE_AUTH,
    PMF_TYPE_ASSOCREQ,
    PMF_TYPE_REASSOCREQ,
};

enum
{
    PMF_PROT_CORRECTKEY = 1,
    PMF_PROT_INCORRECTKEY,
    PMF_PROT_UNPROTECTED,
};

typedef struct pmf_frame
{
    BYTE eFrameName;
    BYTE eProtected;
    char staid[WFA_MAC_ADDR_STR_LEN]; /* sta mac addr */
    /* optional   */
    unsigned char sender_flag;
    char sender[8]; /* ap or sta */

    unsigned char bssid_flag;
    char bssid[WFA_MAC_ADDR_STR_LEN]; /* ap mac addr */


} pmfFrame_t;

/*   DEV_SEND_FRAME     TDLS  */
enum
{
    TDLS_TYPE_DISCOVERY = 1,
    TDLS_TYPE_SETUP,
    TDLS_TYPE_TEARDOWN,
    TDLS_TYPE_CHANNELSWITCH,
    TDLS_TYPE_NULLFRAME,
};

typedef struct tdls_frame
{
    BYTE eFrameName;
    char peer[WFA_MAC_ADDR_STR_LEN];
    /*  optional in the following  */
    unsigned char timeout_flag;
    int timeout;
    unsigned char switchtime_flag;
    int switchtime;
    unsigned char channel_flag;
    int channel;
    unsigned char offset_flag;
    char offset[4]; /* 20 or 40 Mhz */
    unsigned char status_flag;
    int status;     /* status code */
    unsigned char reason_flag;
    int reason;     /* reason code */
    unsigned char bssid_flag;
    char bssid[WFA_MAC_ADDR_STR_LEN];
} tdlsFrame_t;


/*  DEV_SEND_FRAME    VENT, voice ent   */
enum
{
    VENT_TYPE_NEIGREQ = 1,
    VENT_TYPE_TRANSMGMT,
};

typedef struct vent_frame
{
    BYTE type;
    char ssid[WFA_SSID_NAME_LEN];
} ventFrame_t;


/*  DEV_SEND_FRAME    WFD    */
enum
{
    WFD_FRAME_PRBREQ=1,
    WFD_FRAME_RTSP,
    WFD_FRAME_SERVDISC_REQ,
    WFD_FRAME_PRBREQ_TDLS_REQ,
    WFD_FRAME_11V_TIMING_MSR_REQ,
};

enum
{
    WFD_DEV_TYPE_SOURCE=1,
    WFD_DEV_TYPE_PSINK,
    WFD_DEV_TYPE_SSINK,

};

enum
{
    WFD_RTSP_PAUSE=1,
    WFD_RTSP_PLAY,
    WFD_RTSP_TEARDOWN,
    WFD_RTSP_TRIG_PAUSE,
    WFD_RTSP_TRIG_PLAY,
    WFD_RTSP_TRIG_TEARDOWN,
    WFD_RTSP_SET_PARAMETER,

};

enum setParamsTypes
{
    WFD_CAP_UIBC_KEYBOARD=1,
    WFD_CAP_UIBC_MOUSE=1,
    WFD_CAP_RE_NEGO,
    WFD_STANDBY,
    WFD_UIBC_SETTINGS_ENABLE,
    WFD_UIBC_SETTINGS_DISABLE,
    WFD_ROUTE_AUDIO,
    WFD_3D_VIDEOPARAM,
    WFD_2D_VIDEOPARAM,
};


typedef struct wfd_frame
{
    BYTE eframe;
    char sa[WFA_MAC_ADDR_STR_LEN];
    char da[WFA_MAC_ADDR_STR_LEN];
    /*  followings are optional  */
    unsigned char devtype_flag;
    BYTE eDevType;
    unsigned char rtspmsg_flag;
    BYTE eRtspMsgType;
    unsigned char wfdsessionid_flag;
    char wfdSessionID[WFA_WFD_SESSION_ID_LEN];
    unsigned char setparm_flag;
    int	eSetParams;
    unsigned char audioDest_flag;
    int	eAudioDest;
    unsigned char bssid_flag;
    char bssid[WFA_MAC_ADDR_STR_LEN];
    unsigned char msrReqAction_flag;
    int  eMsrAction;
    unsigned char capReNego_flag;
    int  ecapReNego;


} wfdFrame_t;

/*  DEV_SEND_FRAME    HS2-R2  , we are not care HS2 release 1, just imp HS2-R2*/

enum
{
    HS2_FRAME_ANQPQuery=1,
    HS2_FRAME_DLSRequest,
    HS2_FRAME_GARPReq,
    HS2_FRAME_GARPRes,
    HS2_FRAME_NeighAdv,
    HS2_FRAME_ARPProbe,
    HS2_FRAME_ARPAnnounce,
    HS2_FRAME_NeighSolicitReq,
    HS2_FRAME_ARPReply,
};

typedef struct hs2_frame
{
    char sDestMac[WFA_MAC_ADDR_STR_LEN];
    char padNotUsed1[2];
    BYTE eframe;
    /*  following are optional  */
    char bAnqpCapList;
    char bNaiRealmList;
    char b3gppInfo;
    char bDomainList;
    char bHsCapList;
    char bOperName;
    char bNaiHomeRealmList;
    char bVenueName;
    char bRoamingCons;
    char bEssDisassocImm;
    char bWanMat;
    char bOpClass;
    char bOsuProviderList;
    char bNetAuthType;
    char padNotUsed2;
    int  nDisassocTimer;
    int  nFrames;
    char sSessInfoUrl[WFA_URL_STRING_LEN];
    char sDevName[WFA_SSID_NAME_LEN];
    char sIconRequest[WFA_SSID_NAME_LEN];
    char sSenderMac[WFA_MAC_ADDR_STR_LEN];
    char padNotUsed3[2];
    char sDestIp[IPV6_ADDRESS_STRING_LEN];
    char sSenderIp[IPV6_ADDRESS_STRING_LEN];
} hs2Frame_t;

typedef struct loc_frame
{
    char sDestMac[WFA_MAC_ADDR_STR_LEN];
    BYTE eframe;
    char baskForLocCivic;
    char baskForLCI;
    char baddress3;
    char bmsntType;
    char bmaxAgeSubelem;
    char brandInterval;
    char bminApcount;
    char baskForPublicIdentifierURI_FQDN;
} locFrame_t;

enum
{
    PROG_TYPE_GEN = 1,
    PROG_TYPE_PMF,
    PROG_TYPE_TDLS,
    PROG_TYPE_VENT,
    PROG_TYPE_WFD,
	PROG_TYPE_WFDS,
    PROG_TYPE_HS2,
    PROG_TYPE_HS2_R2,
	PROG_TYPE_NAN,
	PROG_TYPE_LOC,
};

typedef struct ca_sta_dev_sendframe
{
    BYTE program;
    union _frametype
    {
        pmfFrame_t  pmf;
        tdlsFrame_t tdls;
        ventFrame_t vent;
        wfdFrame_t  wfd;
        hs2Frame_t  hs2_r2;
		locFrame_t loc;
    } frameType;
} caStaDevSendFrame_t;

typedef struct ca_sta_start_wfd_conn
{
    char intf[WFA_IF_NAME_LEN];
    BYTE peer_count;
    char peer[2][WFA_P2P_DEVID_LEN];
    unsigned char init_wfd_flag;
    BYTE init_wfd;
    unsigned char intent_val_flag;
    BYTE intent_val;
    unsigned char oper_chn_flag;
    WORD oper_chn;
    unsigned char coupledSession_flag;
    WORD coupledSession;
} caStaStartWfdConn_t;

typedef struct ca_sta_connect_go_start_wfd
{
    char intf[WFA_IF_NAME_LEN];
    char grpid[WFA_P2P_GRP_ID_LEN];
    char devId[WFA_P2P_DEVID_LEN];
} caStaConnectGoStartWfd_t;

enum
{
    eInvitationSend = 1,
    eInvitationAccept,
};

typedef struct ca_sta_reinvoke_wfd_session
{
    char intf[WFA_IF_NAME_LEN];
    unsigned char grpid_flag;
    char grpid[WFA_P2P_GRP_ID_LEN];
    char peer[WFA_MAC_ADDR_STR_LEN];
    BYTE wfdInvitationAction;
} caStaReinvokeWfdSession_t;

enum {
	eDiscoveredDevList = 1,
	eOpenPorts,
	eMasterPref,
};


typedef struct ca_sta_get_parameter
{
    char intf[WFA_IF_NAME_LEN];
    BYTE program;
    BYTE getParamValue;
} caStaGetParameter_t;

typedef struct ca_sta_nfc_action
{
   char intf[WFA_IF_NAME_LEN];
   WORD nfcOperation;
   unsigned char intent_val_flag;
   WORD intent_val;
   unsigned char oper_chn_flag;
   WORD oper_chn;	
   unsigned char ssid_flag;
   char ssid[WFA_SSID_NAME_LEN];  
   unsigned char nfc_init_flag;
   WORD nfc_init;	   
} caStaNfcAction_t;

enum {
	eNfcWriteSelect = 1,
	eNfcWriteConfig,
	eNfcWritePasswd,		
	eNfcReadTag,
    eNfcHandOver,
    eNfcWpsHandOver,
    
};

enum {
	eCmdPrimTypeAdvt = 1,
	eCmdPrimTypeSeek,
	eCmdPrimTypeCancel,
	eCmdPrimTypeConnSession,   
	eCmdPrimTypeConfirmSession,
	eCmdPrimTypeSetSessionReady,   
	eCmdPrimTypeBoundPort,   
	eCmdPrimTypeServiceStatusChange,   
	eCmdPrimTypeCloseSession,
	
};

enum {
	eServiceNameOOB= 1,
	eServiceNameSend,
	eServiceNameDisplay,
	eServiceNamePlay,
	eServiceNamePrint,	
};

enum {
	eServiceRoleTx= 1,
	eServiceRoleRx,	
};

enum {
	ePrimitiveCmdType= 1,
	eMessageCmdType,
};

enum {
	eMsgReqSession= 1,
	eMsgRmvSession,
	eMsgRejSession,	
	eMsgAddedSession,
};
enum {
	eSearchResult= 1,
	eSearchTerminated,
	eAdvertiseStatus,	
	eSessionRequest,
	eConnectStatus,
	eSessionStatus,	
	ePortStatus,	
};


typedef struct sta_cmdType_Primitive_Adv
{
   WORD PrimType;
   unsigned char serviceName_flag;
   char serviceName[64];
   unsigned char autoAccept_flag;
   WORD autoAccpet;
   unsigned char serviceInfo_flag;
   char serviceInfo[64];
   unsigned char serviceStatus_flag;
   WORD serviceStaus;   
}staCmdTypePrimitiveAdv;

typedef struct sta_cmdType_Primitive_Seek
{
   WORD PrimType;
   unsigned char serviceName_flag;
   char serviceName[64];
   unsigned char exactSearch_flag;
   char exactSearch;
   unsigned char macAddress_flag;
   char macaddress[WFA_MAC_ADDR_STR_LEN];   
   unsigned char serviceInfo_flag;
   char serviceInfo[64];
}staCmdTypePrimitiveSeek;

typedef struct sta_cmdType_Primitive_Cancel
{
   WORD PrimType;
   unsigned char cancelMethod_flag;
   WORD cancelMethod; // use the enums of the service to indicate the API's   
}staCmdTypePrimitiveCancel;

typedef struct sta_cmdType_Primitive_ConnectSession
{
   WORD PrimType;
   unsigned char serviceMac_flag;
   char serviceMac[WFA_MAC_ADDR_STR_LEN];   
   unsigned char advId_flag;
   long int advID;   
   unsigned char sessionInfo_flag;
   char sessionInfo[64];
   unsigned char networkRole_flag;
   WORD networkRole;
   unsigned char connCapInfo_flag;
   WORD connCapInfo;
   unsigned char ssid_flag;
   char ssid[64];
   unsigned char operChn_flag;
   int operChn;
}staCmdTypePrimitiveConnectSession;

typedef struct sta_cmdType_Primitive_ConfirmSession
{
   WORD PrimType;
   unsigned char sessionMac_flag;
   char sessionMac[WFA_MAC_ADDR_STR_LEN];   
   unsigned char sessionID_flag;
   long int sessionID;   
   unsigned char confirmed_flag;
   WORD confirmed;
}staCmdTypePrimitiveConfirmSession;
typedef struct sta_cmdType_Primitive_SetSessionReady
{
   WORD PrimType;
   unsigned char sessionMac_flag;
   char sessionMac[WFA_MAC_ADDR_STR_LEN];   
   unsigned char sessionID_flag;
   long int sessionID;   
}staCmdTypePrimitiveSetSessionReady;
typedef struct sta_cmdType_Primitive_BoundPort
{
   WORD PrimType;
   unsigned char sessionMac_flag;
   char sessionMac[WFA_MAC_ADDR_STR_LEN];   
   unsigned char sessionID_flag;
   long int sessionID;   
   unsigned char port_flag;
   WORD port;
}staCmdTypePrimitiveBoundPort;
typedef struct sta_cmdType_Primitive_SerivceStatusChange
{
   WORD PrimType;
   unsigned char advId_flag;
   long int advID;   
   unsigned char serviceStatus_flag;
   WORD serviceStatus;
}staCmdTypePrimitiveServiceStatusChange;

typedef struct sta_cmdType_Primitive_CloseSession
{
   WORD PrimType;
   unsigned char sessionMac_flag;
   char sessionMac[WFA_MAC_ADDR_STR_LEN];   
   unsigned char sessionID_flag;
   long int sessionID;   
}staCmdTypePrimitiveCloseSession;

typedef struct sta_cmdType_Message
{
   WORD opcode;
   unsigned char sessionId_flag;
   long int sessionID;
   unsigned char sessionMac_flag;
   char sessionMac[WFA_MAC_ADDR_STR_LEN]; 
}staCmdTypeMessage;

//typedef struct ca_sta_get_events
//{
//   char intf[WFA_IF_NAME_LEN];
//   BYTE program;
//} caStaGetEvents_t;


typedef struct ca_sta_get_event_details
{
	char intf[WFA_IF_NAME_LEN];
	BYTE program;
	WORD eventId;

} caStaGetEventDetails_t;

typedef struct ca_sta_invoke_command
{
   char intf[WFA_IF_NAME_LEN];
   WORD program;
   WORD cmdType;
   union _InvokeCmds
   {
   		union _PrimType
		{
		   	WORD PrimType;
			staCmdTypePrimitiveAdv AdvPrim;
			staCmdTypePrimitiveSeek SeekPrim;
			staCmdTypePrimitiveCancel CancelPrim;
			staCmdTypePrimitiveConnectSession ConnSessPrim;	
			staCmdTypePrimitiveConfirmSession ConfSessPrim;
			staCmdTypePrimitiveSetSessionReady SetSessRdyPrim;
			staCmdTypePrimitiveBoundPort BoundPortPrim;
			staCmdTypePrimitiveServiceStatusChange ServStatusChngPrim;
			staCmdTypePrimitiveCloseSession CloseSessPrim;
   		}primtiveType;		
		staCmdTypeMessage Msg ;
   }InvokeCmds;
} caStaInvokeCmd_t;

enum {
	eWfdsMgtActionsTransfer= 1,
	eWfdsMgtActionsPause,
	eWfdsMgtActionsResume,
	eWfdsMgtActionsModify,
	eWfdsMgtActionsCancel,
	eWfdsMgtActionsAmidClose,
	eWfdsMgtActionsClose,
	eWfdsMgtActionsReceive,
	eWfdsMgtActionsPlay,
	eWfdsMgtActionsDisplay,
	eWfdsMgtActionsGetPrintAttr,
	eWfdsMgtActionsPrintJobOper,
	eWfdsMgtActionsGetJobAttr,
	eWfdsMgtActionsCreateJobOper,
	eWfdsMgtActionsSendPrintDoc,
	eWfdsMgtActionsDoNothing,
	
};

enum {
	ePclmPdr= 1,
	ePwgPdr,
};
typedef struct sta_wfds_ManageService
{
   WORD serviceName;
   WORD serviceRole;
   //optional args
   unsigned char serviceMac_flag;
   char serviceMac[WFA_MAC_ADDR_STR_LEN];   
   unsigned char advId_flag;
   long int advID;   
   unsigned char sessionInfo_flag;
   char sessionInfo[64];
   unsigned char networkRole_flag;
   WORD networkRole;
   unsigned char connCapInfo_flag;
   WORD connCapInfo;
   unsigned char mngActions_flag;
   unsigned char numMngActions;   
   WORD mgtActions[8];
   unsigned char sendFileList_flag;
   unsigned char numFiles;
   char fileList[2][16];
   unsigned char modSendFileList_flag;
   unsigned char numModFiles;   
   char modFileList[2][16];      
   unsigned char PdlType_flag;
   WORD PdlType;
   
  } staWfdsMngService;

typedef struct ca_sta_manage_service
{
   char intf[WFA_IF_NAME_LEN];
   WORD program;
   union _MngServiceCmds
   {
		staWfdsMngService MgtServ;
   }MngCmds;
} caStaMngServ_t;


enum {
	eUibcGen = 1,
	eUibcHid,
    eFrameSkip,
    eInputContent,
    eI2cRead,
    eI2cWrite,
    eIdrReq,
};

enum
{
    eSingleTouchEvent = 1,
    eMultiTouchEvent,
    eKeyBoardEvent,
    eMouseEvent,
    eBtEvent,
};

enum
{
    eProtected = 1,
    eUnprotected,
    eProtectedVideoOnly,
};

enum trigger {
    eFTMsession = 1,
    eANQPQUERY,
};

enum formatBwFTM {
    eHT20 = 9,
	eHT40_5G = 11,
	eVHT20 = 10,
	eVHT40 = 12,
	eVHT80 = 13,
};

enum event {
    eDiscoveryResult = 1,
    eReplied,
    ePublishTerminated,
    eSubscribeTerminated,
    eFollowUpReceive,
};

enum method {
    ePublish = 1,
    eSubscribe,
    eFollowUp,
};

enum methodtype {
    eUnsolicited = 1,
    eSolicited,
	eActive,
    ePassive,
    eCancel,
};

typedef struct wfd_generate_event
{
    BYTE type;
    BYTE wfdSessionIdflag;
    char wfdSessionID[WFA_WFD_SESSION_ID_LEN];
    BYTE wfdUibcEventTypeflag;
    BYTE wfdUibcEventType;
    BYTE wfdUibcEventPrepareflag;
    BYTE wfdUibcEventPrepare;
    BYTE wfdFrameSkipRateflag;
    BYTE wfdInputContentTypeflag;
    BYTE wfdInputContentType;
    BYTE wfdI2cDataflag;
    char wfdI2cData[32];

} caWfdStaGenEvent_t;


typedef struct ca_sta_generate_event
{
    char intf[WFA_IF_NAME_LEN];
    BYTE program;
    caWfdStaGenEvent_t wfdEvent;
} caStaGenEvent_t;


//#ifdef WFA_STA_TB
typedef enum wfa_supplicant_names
{
    eWindowsZeroConfig = 1,
    eMarvell,
    eIntelProset,
    eWpaSupplicant,
    eCiscoSecureClient,
    eOpen1x,
    eDefault
} wfaSupplicantName;

typedef struct ca_sta_set_p2p
{
    char intf[WFA_IF_NAME_LEN];

    unsigned char listen_chn_flag;
    WORD listen_chn;

    unsigned char p2p_mode_flag;
    char p2p_mode[16];

    unsigned char presistent_flag;
    int presistent;

    unsigned char intra_bss_flag;
    int intra_bss;

    unsigned char noa_duration_flag;
    int noa_duration;

    unsigned char noa_interval_flag;
    int noa_interval;

    unsigned char noa_count_flag;
    int noa_count;

    unsigned char concurrency_flag;
    int concurrency;

    unsigned char p2p_invitation_flag;
    int p2p_invitation;

    unsigned char bcn_int_flag;
    int bcn_int;

    unsigned char ext_listen_time_int_flag;
    int ext_listen_time_int;

    unsigned char ext_listen_time_period_flag;
    int ext_listen_time_period;

    unsigned char discoverability_flag;
    int discoverability;


    unsigned char service_discovry_flag;
    int service_discovery;

    unsigned char crossconnection_flag;
    int crossconnection;

    unsigned char p2pmanaged_flag;
    int p2pmanaged;

    unsigned char go_apsd_flag;
    int go_apsd;

    unsigned char discover_type_flag;
    int discoverType;

} caStaSetP2p_t;

typedef struct ca_sta_p2p_connect
{
    char intf[WFA_IF_NAME_LEN];

    char grpid[WFA_P2P_GRP_ID_LEN];
    char devId[WFA_P2P_DEVID_LEN];
} caStaP2pConnect_t;

typedef struct ca_sta_start_auto_go
{
    char intf[WFA_IF_NAME_LEN];
    WORD oper_chn;
    unsigned char ssid_flag;
    char ssid[WFA_SSID_NAME_LEN];
    unsigned char rtsp_flag;
    WORD rtsp;

} caStaStartAutoGo_t;


typedef struct ca_sta_p2p_start_grp_formation
{
    char intf[WFA_IF_NAME_LEN];
    char devId[WFA_P2P_DEVID_LEN];
    WORD intent_val;
    WORD init_go_neg;
    unsigned char oper_chn_flag;
    WORD oper_chn;
    unsigned char ssid_flag;
    char ssid[WFA_SSID_NAME_LEN];
} caStaP2pStartGrpForm_t;

typedef struct ca_sta_p2p_dissolve
{
    char intf[WFA_IF_NAME_LEN];
    char grpId[WFA_P2P_GRP_ID_LEN];
} caStaP2pDissolve_t;

typedef struct ca_sta_send_p2p_inv_req
{
    char intf[WFA_IF_NAME_LEN];
    char devId[WFA_P2P_DEVID_LEN];
    char grpId[WFA_P2P_GRP_ID_LEN];
    int reinvoke;
} caStaSendP2pInvReq_t;

typedef struct ca_sta_accept_p2p_inv_req
{
    char intf[WFA_IF_NAME_LEN];
    char devId[WFA_P2P_DEVID_LEN];
    char grpId[WFA_P2P_GRP_ID_LEN];
    int reinvoke;
} caStaAcceptP2pInvReq_t;


typedef struct ca_sta_send_p2p_prov_dis_req
{
    char intf[WFA_IF_NAME_LEN];
    char confMethod[16];
    char devId[WFA_P2P_DEVID_LEN];
} caStaSendP2pProvDisReq_t;

typedef struct ca_sta_set_wps_pbc
{
    char intf[WFA_IF_NAME_LEN];
    unsigned char grpid_flag;
    char grpId[WFA_P2P_GRP_ID_LEN];
} caStaSetWpsPbc_t;

typedef struct ca_sta_wps_read_pin
{
    char intf[WFA_IF_NAME_LEN];
    unsigned char grpid_flag;
    char grpId[WFA_P2P_GRP_ID_LEN];
} caStaWpsReadPin_t;

typedef struct ca_sta_wps_read_label
{
    char intf[WFA_IF_NAME_LEN];
    unsigned char grpid_flag;
    char grpId[WFA_P2P_GRP_ID_LEN];
} caStaWpsReadLabel_t;

typedef struct ca_sta_wps_enter_pin
{
    char intf[WFA_IF_NAME_LEN];
    char wpsPin[WFA_WPS_PIN_LEN];
    unsigned char grpid_flag;
    char grpId[WFA_P2P_GRP_ID_LEN];
} caStaWpsEnterPin_t;

typedef struct ca_sta_get_psk
{
    char intf[WFA_IF_NAME_LEN];
    char grpId[WFA_P2P_GRP_ID_LEN];
} caStaGetPsk_t;

typedef struct ca_sta_get_p2p_ip_config
{
    char intf[WFA_IF_NAME_LEN];
    char grpId[WFA_P2P_GRP_ID_LEN];
} caStaGetP2pIpConfig_t;

typedef struct ca_sta_send_service_discovery_req
{
    char intf[WFA_IF_NAME_LEN];
    char devId[WFA_P2P_DEVID_LEN];
} caStaSendServiceDiscoveryReq_t;

typedef struct ca_sta_send_p2p_presence_req
{
    char intf[WFA_IF_NAME_LEN];
    long long int duration;
    long long int interval;
} caStaSendP2pPresenceReq_t;

typedef struct ca_sta_add_arp_table_entry
{
    char intf[WFA_IF_NAME_LEN];
    char macaddress [WFA_MAC_ADDR_STR_LEN];
    char ipaddress [WFA_MAC_ADDR_STR_LEN];
} caStaAddARPTableEntry_t;

typedef struct ca_sta_block_icmp_reponse
{
    char intf[WFA_IF_NAME_LEN];
    char ipaddress [WFA_MAC_ADDR_STR_LEN];
    char grpId[WFA_P2P_GRP_ID_LEN];
} caStaBlockICMPResponse_t;


typedef struct ca_sta_set_sleep
{
    char intf[WFA_IF_NAME_LEN];
    char grpId[WFA_P2P_GRP_ID_LEN];
} caStaSetSleep_t;

typedef struct ca_sta_set_opportunistic_ps
{
    char intf[WFA_IF_NAME_LEN];
    int ctwindow;
    char grpId[WFA_P2P_GRP_ID_LEN];
} caStaSetOpprPs_t;

/* P2P */

typedef enum wfa_preambleType
{
    eLong = 1,
    eShort
} wfaPreambleType;

typedef enum wfa_WirelessMode
{
    eModeB = 1,
    eModeBG,
    eModeA,
    eModeABG,
    eModeAN,
    eModeGN,
    eModeNL,
    eModeAC,
} wfaWirelessMode;

typedef enum wfa_reset_prog
{
    eResetProg11n =1,
} wfaResetProg;

typedef enum wfa_tdlsMode
{
    eDef = 0,
    eHiLoMac = 1,
    eExistLink,
    eAPProhibit,
    eWeakSec,
    eIgnChnlSWProh,  /* if it is present, ignore channel switch prohibit */
} wfaTDLSMode;

typedef enum wfa_wfdDevType
{
    eSource = 1,
    ePSink,
    eSSink,
    eDual,
} wfaWfdDevType;

typedef enum wfa_UiInput
{
    eKeyBoard = 1,
    eMouse,
    eBt,
    eJoyStick,
    eSingleTouch,
    eMultiTouch,
} wfaUiInput;

typedef enum wfa_AudioModes
{
    eMandatoryAudioMode = 1,
    eDefaultAudioMode,
} wfaAudioModes;




typedef enum wfa_VideoFormats
{
    eCEA = 1,
    e640x480p60,
    e720x480p60,
    e20x480i60,
    e720x576p50,
    e720x576i50,
    e1280x720p30,
    e1280x720p60,
    e1920x1080p30,
    e1920x1080p60,
    e1920x1080i60,
    e1280x720p25,
    e1280x720p50,
    e1920x1080p25,
    e1920x1080p50,
    e1920x1080i50,
    e1280x720p24,
    e1920x1080p24,

    eVesa,
    e800x600p30,
    e800x600p60,
    e1024x768p30,
    e1024x768p60,
    e1152x864p30,
    e1152x864p60,
    e1280x768p30,
    e1280x768p60,
    e1280x800p30,
    e1280x800p60,
    e1360x768p30,
    e1360x768p60,
    e1366x768p30,
    e1366x768p60,
    e1280x1024p30,
    e1280x1024p60,
    e1400x1050p30,
    e1400x1050p60,
    e1440x900p30,
    e1440x900p60,
    e1600x900p30,
    e1600x900p60,
    e1600x1200p30,
    e1600x1200p60,
    e1680x1024p30,
    e1680x1024p60,
    e1680x1050p30,
    e1680x1050p60,
    e1920x1200p30,
    e1920x1200p60,

    eHH,
    e800x480p30,
    e800x480p60,
    e854x480p30,
    e854x480p60,
    e864x480p30,
    e864x480p60,
    e640x360p30,
    e640x360p60,
    e960x540p30,
    e960x540p60,
    e848x480p30,
    e848x480p60,
} wfavideoFormats;

typedef enum wfa_wfdsPresetTypes
{	
   eAcceptPD= 1,
   eRejectPD,
   eIgnorePD,
   eRejectSession,
} wfaWfdsPresetTypes;

typedef enum wfa_wfdsConnCapInfo
{	
   eWfdsGO= 1,
   eWfdsCLI,
   eWfdsNewGO,
   eWfdsNew,
   eWfdsCliGO,
} wfaWfdsConnCapInfo;

typedef struct ca_sta_preset_parameters
{
    char intf[WFA_IF_NAME_LEN];
    wfaSupplicantName supplicant;

    BYTE programFlag;
    WORD program;


    BYTE rtsFlag;
    WORD rtsThreshold;
    BYTE fragFlag;
    WORD fragThreshold;
    BYTE preambleFlag;
    wfaPreambleType preamble;
    BYTE modeFlag;
    wfaWirelessMode wirelessMode;
    BYTE psFlag;
    BYTE legacyPowerSave;
    BYTE wmmFlag;
    BYTE wmmState;
    BYTE reset;
    BYTE ht;    // temperary for high throughput
    BYTE ftoa;
    BYTE ftds;
    BYTE activescan;
    WORD oper_chn;
    BYTE tdls;
    BYTE tdlsMode;

    BYTE tdlsFlag;

   BYTE wfdDevTypeFlag;
   BYTE wfdDevType ;
   BYTE wfdUibcGenFlag;
   BYTE wfdUibcGen ;
   BYTE wfdUibcHidFlag;
   BYTE wfdUibcHid ;
   BYTE wfdUiInputFlag;
   BYTE wfdUiInputs ;   
   BYTE wfdUiInput[3] ;
   BYTE wfdHdcpFlag;
   BYTE wfdHdcp ;
   BYTE wfdFrameSkipFlag;
   BYTE wfdFrameSkip ;
   BYTE wfdAvChangeFlag;
   BYTE wfdAvChange ;
   BYTE wfdStandByFlag;
   BYTE wfdStandBy ;
   BYTE wfdInVideoFlag;
   BYTE wfdInVideo ;
   BYTE wfdVideoFmatFlag;
   BYTE wfdInputVideoFmats;
   BYTE wfdVideoFmt[64];
   BYTE wfdAudioFmatFlag;
   BYTE wfdAudioFmt ;   
   BYTE wfdI2cFlag;
   BYTE wfdI2c ;
   BYTE wfdVideoRecoveryFlag;
   BYTE wfdVideoRecovery ;   
   BYTE wfdPrefDisplayFlag;
   BYTE wfdPrefDisplay ;   
   BYTE wfdServiceDiscoveryFlag;
   BYTE wfdServiceDiscovery ;   
   BYTE wfd3dVideoFlag;
   BYTE wfd3dVideo ;   
   BYTE wfdMultiTxStreamFlag;
   BYTE wfdMultiTxStream ;   
   BYTE wfdTimeSyncFlag;
   BYTE wfdTimeSync ;   
   BYTE wfdEDIDFlag;
   BYTE wfdEDID ;   
   BYTE wfdUIBCPrepareFlag;
   BYTE wfdUIBCPrepare ;      
   BYTE wfdCoupledCapFlag;
   BYTE wfdCoupledCap ; 
   BYTE wfdOptionalFeatureFlag;
   BYTE wfdSessionAvail ; 
   BYTE wfdSessionAvailFlag;
   BYTE wfdDeviceDiscoverability ; 
   BYTE wfdDeviceDiscoverabilityFlag;

   // WFDS
   BYTE wfdsType;
   BYTE wfdsConnectionCapability;
   BYTE wfdsConnectionCapabilityFlag;
   
  
   
} caStaPresetParameters_t;

typedef struct ca_sta_set_11n
{
    char intf[WFA_IF_NAME_LEN];
    BOOL _40_intolerant;
    BOOL addba_reject;
    BOOL ampdu;
    BOOL amsdu;
    BOOL greenfield;
    BOOL sgi20;
    unsigned short stbc_rx;
    unsigned short smps;
    char width[8];
    char mcs_fixedrate[4];
    BOOL mcs32;
    BOOL rifs_test;
    unsigned char txsp_stream;
    unsigned char rxsp_stream;
} caSta11n_t;

typedef struct ca_sta_set_wireless
{
    char intf[WFA_IF_NAME_LEN];
    char program[WFA_PROGNAME_LEN];
    char band [8];
#define NOACK_BE       0
#define NOACK_BK       1
#define NOACK_VI       2
#define NOACK_VO       3
    unsigned char noAck[4];
} caStaSetWireless_t;


typedef struct ca_sta_send_addba
{
    char intf[WFA_IF_NAME_LEN];
    unsigned short tid;
} caStaSetSendADDBA_t;

typedef struct ca_sta_set_rifs
{
    char intf [WFA_IF_NAME_LEN];
    unsigned int action;

} caStaSetRIFS_t;

typedef struct ca_sta_send_coexist_mgmt
{
    char intf[WFA_IF_NAME_LEN];
    char type[16];
    char value[16];
} caStaSendCoExistMGMT_t;
//#endif

typedef struct ca_sta_set_uapsd
{
    char intf[WFA_IF_NAME_LEN];
    char ssid[WFA_SSID_NAME_LEN];
    int maxSPLength;
    BYTE acBE;
    BYTE acBK;
    BYTE acVI;
    BYTE acVO;
    int  type;
    char peer[18];
} caStaSetUAPSD_t;

typedef struct ca_sta_set_ibss
{
    char intf[WFA_IF_NAME_LEN];
    char ssid[WFA_SSID_NAME_LEN];
    int channel;
    int encpType;
    char keys[4][32];
    int activeKeyIdx;
} caStaSetIBSS_t;

typedef struct sta_upload
{
    int type;
    int next;     /* sequence number, 0 is the last one */
} caStaUpload_t;

typedef struct sta_debug_set
{
    unsigned short level;
    unsigned short state;
} staDebugSet_t;
typedef struct config
{
    BYTE wmm;
    int  rts_thr ;
    int  frag_thr ;
} wmmconf_t;

typedef struct wmm_tsinfo
{
    unsigned int Reserved1 :1;
    unsigned int TID       :4;
    unsigned int direction :2;
    unsigned int dummy1    :1;
    unsigned int dummy2    :1;
    unsigned int Reserved2 :1;
    unsigned int PSB       :1;
    unsigned int UP        :3;
    unsigned int infoAck   :2;
    unsigned int Reserved4 :1;
    unsigned int Reserved5 :6;
    unsigned int bstSzDef :1;
} wmmtsinfo_t;

typedef struct wmm_tspec
{
    wmmtsinfo_t      tsinfo;
    BOOL Fixed;//The MSDU Fixed Bit
    unsigned short size;//The MSDU Size
    unsigned short maxsize;//Maximum MSDU Size
    unsigned int   min_srvc;//The minimum Service Interval
    unsigned int   max_srvc;//The maximum Service Interval
    unsigned int inactivity;//Inactivity Interval
    unsigned int suspension;//The Suspension Interval
    unsigned int srvc_strt_tim;//The Service Start Time
    unsigned int mindatarate;//The Minimum Data Rate
    unsigned int meandatarate;//The Minimum Data Rate
    unsigned int peakdatarate;//The Minimum Data Rate
    unsigned int burstsize;//The Maximum Burst Size
    unsigned int delaybound;//The Delay Bound
    unsigned int PHYrate;//The minimum PHY Rate
    float sba;//The Surplus Bandwidth Allownce
    unsigned short medium_time;//The medium time
} wmmtspec_t;

typedef struct wmmac_addts
{
    BYTE       dialog_token;
    BYTE       accesscat;
    wmmtspec_t tspec;
} wmmacadd_t;

typedef struct ca_sta_set_wmm
{
    char intf[WFA_IF_NAME_LEN];
    BYTE group;
    BYTE action;
#ifdef WFA_WMM_AC
    BYTE       send_trig;
    char       dipaddr[WFA_IP_ADDR_STR_LEN];
    BYTE       trig_ac;
#endif

    union _action
    {
        wmmconf_t   config;
        wmmacadd_t  addts;
        BYTE        delts;
    } actions;
} caStaSetWMM_t;

typedef struct ca_sta_set_pwrsave
{
    char intf[WFA_IF_NAME_LEN];
    char mode[8];
} caStaSetPwrSave_t;

typedef struct ca_sta_scan
{
    char intf[WFA_IF_NAME_LEN];
} caStaScan_t;

typedef struct ca_sta_disconnect
{
    char intf[WFA_IF_NAME_LEN];
} caStaDisconnect_t;

typedef struct ca_sta_reset_default
{
    char intf[WFA_IF_NAME_LEN];
    char prog[8];
    char type[8];
} caStaResetDefault_t;

typedef struct ca_dev_info
{
    BYTE fw;
} caDevInfo_t;

typedef struct ca_sta_associate
{
    char ssid[WFA_SSID_NAME_LEN];
    char bssid[18];
    unsigned char wps;
} caStaAssociate_t;

typedef enum wfa_onoffType
{
    WFA_OFF = 0,
    WFA_ON = 1,
} wfaOnOffType;

typedef struct ca_sta_set_radio
{
    wfaOnOffType mode;
} caStaSetRadio_t;

typedef struct ca_sta_rfeature
{
    char prog[8];
    wfaEnableType uapsd;
    char peer[18]; /* peer mac addr */
    wfaEnableType tpktimer;
    char chswitchmode[16];
    int offchnum;
    char secchoffset[16];
} caStaRFeat_t;

typedef struct ca_sta_exec_action
{
   /*  sta_exec_action  NAN */
   
   char intf[WFA_IF_NAME_LEN];
   BYTE prog;
   char nanOp[8];
   char masterPref[8];
   char randFactor[8];
   char hopCount[8];
   char highTsf[8];
   char methodType[16];
   char furtherAvailInd[8];
   char mac[WFA_MAC_ADDR_STR_LEN];
   char band[8];
   unsigned short fiveGHzOnly;
   char publishType[16];
   char subscribeType[16];
   char serviceName[64];
   unsigned short sdfTxDw;
   unsigned short sdfDelay;
   char rxMatchFilter[64];
   char txMatchFilter[64];
   unsigned short discRangeLtd;
   unsigned short discRangeIgnore;
   unsigned short includeBit;
   unsigned short srfType;
   unsigned int remoteInstanceID;
   unsigned int localInstanceID;
   
   /*  sta_exec_action  LOC */
   
   char destMac[WFA_MAC_ADDR_STR_LEN];
   char trigger[16];
   unsigned short askForLocCivic;
   unsigned short askForLCI;
   unsigned short burstsExponent;
   unsigned short asap;
   unsigned short formatBwFTM;
   
} caStaExecAction_t;

typedef struct ca_sta_get_events
{
	char intf[WFA_IF_NAME_LEN];
	BYTE program;
	char action[8];
} caStaGetEvents_t;

typedef struct dut_commands
{
    char intf[WFA_IF_NAME_LEN];
    union _cmds
    {
        int streamId;
        int iftype;
        tgProfile_t profile;
        tgPingStart_t startPing;
        char resetProg[16];
        char macaddr[18];
        caStaAssociate_t assoc;
        char ssid[WFA_SSID_NAME_LEN];
        caStaSetIpConfig_t ipconfig;
        caStaVerifyIpConnect_t verifyIp;
        caStaSetEncryption_t wep;
        caStaSetPSK_t        psk;
        caStaSetEapTLS_t     tls;
        caStaSetEapTTLS_t    ttls;
        caStaSetEapSIM_t     sim;
        caStaSetEapPEAP_t    peap;
        caStaSetEapAKA_t     aka;
        caStaSetEapFAST_t    fast;
        caStaSetSecurity_t   setsec;
        caStaSetUAPSD_t      uapsd;
        caStaSetIBSS_t       ibss;
        caStaUpload_t        upload;
        caStaSetWMM_t        setwmm;
        staDebugSet_t        dbg;
        caDevInfo_t          dev;
        caStaDevSendFrame_t     sf;
        caStaSetRadio_t      sr;
        caStaRFeat_t         rfeat;
	   caStaExecAction_t	sact;
	   caStaGetEvents_t		sevts;
	   caStaScan_t		scan;
	   caStaDisconnect_t discont;
	   caStaResetDefault_t rdef;
	   caStaPresetParameters_t presetparams;
    } cmdsu;
} dutCommand_t;


extern int buildCommandProcessTable(void);

#endif
