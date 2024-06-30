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

#ifndef WFA_CA_RESP_H
#define WFA_CA_RESP_H

typedef int (*dutCommandRespFuncPtr)(BYTE *cmdBuf);

int caCmdNotDefinedYet(BYTE *cmdBuf);
int wfaStaVerifyIpConnectResp(BYTE *cmdBuf);
int wfaStaSetIpConfigResp(BYTE *cmdBuf);
int wfaStaIsConnectedResp(BYTE *cmdBuf);
int wfaStaGetIpConfigResp(BYTE *cmdBuf);
int wfaGetVersionResp(BYTE *cmdBuf);
int wfaStaGetInfoResp(BYTE *cmdBuf);
int wfaTrafficAgentConfigResp(BYTE *cmdBuf);
int wfaTrafficAgentSendResp(BYTE *cmdBuf);
int wfaTrafficAgentRecvStartResp(BYTE *cmdBuf);
int wfaTrafficAgentRecvStopResp(BYTE *cmdBuf);
int wfaTrafficAgentResetResp(BYTE *cmdBuf);
int wfaTrafficAgentPingStartResp(BYTE *cmdBuf);
int wfaTrafficAgentPingStopResp(BYTE *cmdBuf);
int wfaStaGetMacAddressResp(BYTE *cmdBuf);
int wfaStaGetBSSIDResp(BYTE *cmdBuf);
int wfaStaSetEncryptionResp(BYTE *cmdBuf);
int wfaStaSetEapTLSResp(BYTE *cmdBuf);
int wfaStaSetPSKResp(BYTE *cmdBuf);
int wfaStaSetEapTTLSResp(BYTE *cmdBuf);
int wfaStaSetEapSIMResp(BYTE *cmdBuf);
int wfaStaSetEapPEAPResp(BYTE *cmdBuf);
int wfaStaSetSecurityResp(BYTE *cmdBuf);
int wfaStaAssociateResp(BYTE *cmdBuf);
int wfaStaScanResp(BYTE *cmdBuf);
int wfaStaSetIBSSResp(BYTE *cmdBuf);
int wfaStaGetStatsResp(BYTE *cmdBuf);
int wfaDeviceGetInfoResp(BYTE *cmdBuf);
int wfaDeviceListIFResp(BYTE *cmdBuf);
int wfaStaDevSendFrameResp(BYTE *cmdBuf);
int wfaStaDebugSetResp(BYTE *cmdBuf);
void wfaStaDebugHexDump(BYTE *cmdBuf);
int wfaStaSetModeResp(BYTE *cmdBuf);
int wfaStaUploadResp(BYTE *cmdBuf);
int wfaStaSetUAPSDResp(BYTE *cmdBuf);
int wfaStaSetModeResp(BYTE *cmdBuf);
int wfaStaSetWMMResp(BYTE *cmdBuf);
int wfaStaPresetParametersResp(BYTE *cmdBuf);
int wfaStaSetEapFASTResp(BYTE *cmdBuf);
int wfaStaSetEapAKAResp(BYTE * cmdBuf);
int wfaStaSetSystimeResp(BYTE * cmdBuf);

int wfaStaSet11nResp(BYTE * cmdBuf);
int wfaStaSetWirelessResp(BYTE * cmdBuf);
int wfaStaSendADDBAResp(BYTE * cmdBuf);
int wfaStaCoexMgmtResp(BYTE * cmdBuf);
int wfaStaRifsTestResp(BYTE * cmdBuf);
int wfaStaResetDefaultResp(BYTE * cmdBuf);
int wfaStaDisconnectResp(BYTE * cmdBuf);

int wfaStandardReturn(BYTE *cmdBuf);

int wfaStaGenericResp(BYTE *cmdBuf);
/* P2P */
int 	wfaStaGetP2pDevAddressResp(BYTE *cmdBuf);
int 	wfaStaSetP2pResp(BYTE *cmdBuf);
int 	wfaStaP2pConnectResp(BYTE *cmdBuf);
int 	wfaStaStartAutoGO(BYTE *cmdBuf);
int 	wfaStaP2pStartGrpFormResp(BYTE *cmdBuf);
int 	wfaStaP2pDissolveResp(BYTE *cmdBuf);
int 	wfaStaSendP2pInvReqResp(BYTE *cmdBuf);
int 	wfaStaAcceptP2pInvReqResp(BYTE *cmdBuf);
int 	wfaStaSendP2pProvDisReqResp(BYTE *cmdBuf);
int 	wfaStaSetWpsPbcResp(BYTE *cmdBuf);
int 	wfaStaWpsReadPinResp(BYTE *cmdBuf);
int 	wfaStaWpsEnterPinResp(BYTE *cmdBuf);
int 	wfaStaGetPskResp(BYTE *cmdBuf);
int 	wfaStaP2pResetResp(BYTE *cmdBuf);
int 	wfaStaWpsReadLabelResp(BYTE *cmdBuf);
int 	wfaStaGetP2pIpConfigResp(BYTE *cmdBuf);
int 	wfaStaSendServiceDiscoveryReqResp(BYTE *cmdBuf);
int 	wfaStaSendP2pPresenceReqResp(BYTE *cmdBuf);
int 	wfaStaSetSleepReqResp(BYTE *cmdBuf);
int 	wfaStaSetOpportunisticPsReqResp(BYTE *cmdBuf);
int     wfaStaAddArpTableEntryResp(BYTE * cmdBuf);
int     wfaStaBlockICMPResponseResp(BYTE * cmdBuf);
/* P2P */
int     wfaStaStartWfdConnectionResp(BYTE * cmdBuf);
int		wfaStaCliCmdResp(BYTE * cmdBuf);
int		wfaStaConnectGoStartWfdResp(BYTE * cmdBuf);
int		wfaStaGenericResp(BYTE * cmdBuf);
int     wfaStaGetParameterResp(BYTE * cmdBuf);
int     wfaStaNfcActionResp(BYTE * cmdBuf);
int     wfaStaInvokeCommandResp(BYTE * cmdBuf);
int     wfaStaManageServiceResp(BYTE * cmdBuf);
int     wfaStaGetEventsResp(BYTE * cmdBuf);
int     wfaStaGetEventDataResp(BYTE * cmdBuf);
int 	wfaStaExecActionResp(BYTE *cmdBuf);


#endif
