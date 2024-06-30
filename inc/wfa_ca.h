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

#ifndef _WFA_CA_H_
#define _WFA_CA_H_

#define ENCRYPT_NONE         0
#define ENCRYPT_WEP          1
#define ENCRYPT_TKIP         2
#define ENCRYPT_AESCCMP      3
#define ENCRYPT_AESCCMP_TKIP 4

#define WMMAC_UPLINK         0
#define WMMAC_DOWNLINK       1
#define WMMAC_BIDIR          3
#define GROUP_WMMAC          0
#define GROUP_WMMCONF        1
#define WMMAC_ADDTS          0
#define WMMAC_DELTS          1
#define WMMAC_AC_BE          0
#define WMMAC_AC_BK          1
#define WMMAC_AC_VI          2
#define WMMAC_AC_VO          3

extern int wfaStaAssociate(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaIsConnected(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaGetIpConfig(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetIpConfig(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaVerifyIpConnection(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaGetMacAddress(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetMacAddr(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);

extern int wfaStaGetBSSID(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaGetStats(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaSetEncryption(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetEapTLS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetPSK(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaGetInfo(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaDeviceGetInfo(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaDeviceListIF(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetEapTTLS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetEapSim(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetPEAP(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetEapSIM(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetPEAP(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetUAPSD(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetIBSS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaDebugSet(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetMode(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaUpload(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetWMM(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);

extern int wfaStaPresetParams(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetEapFAST(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetEapAKA(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetSystime(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaResetTspecs(char* ifname);
extern int wfaStaSet11n(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetWireless(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSendADDBA(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetRIFS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSendCoExistMGMT(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaResetDefault(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaDisconnect(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaReAssociate(int, BYTE*, int*, BYTE*);

extern int wfaStaSetPwrSave(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSendNeigReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaTestBedCmd(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSendFrame(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaGetP2pDevAddress(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetP2p(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaP2pConnect(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaStartAutoGo(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaP2pStartGrpFormation(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);

extern int wfaStaP2pDissolve(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSendP2pInvReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaAcceptP2pInvReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSendP2pProvDisReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetWpsPbc(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);

extern int wfaStaWpsReadPin(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaWpsEnterPin(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaGetPsk(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaP2pReset(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaWpsReadLabel(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaGetP2pIpConfig(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSendServiceDiscoveryReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSendP2pPresenceReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetSleepReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetOpportunisticPsReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaAddArpTableEntry(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaBlockICMPResponse(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaDevSendFrame(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetRadio(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetRFeature(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaSetSecurity(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaScan(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);

extern int wfaStaStartWfdConnection(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaCliCommand(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaConnectGoStartWfd(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaGenerateEvent(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaReinvokeWfdSession(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaGetParameter(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaNfcAction(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaInvokeCommand(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaManageService(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaGetEvents(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaGetEventDetails(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);
extern int wfaStaExecAction(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf);

#endif
