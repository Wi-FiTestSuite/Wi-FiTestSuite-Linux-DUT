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
 *      File: wfa_cmdproc.c
 *      Library functions to handle all string command parsing and convert it
 *      to an internal format for DUT. They should be called by Control Agent
 *      and Test console while receiving commands from CLI or TM
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "wfa_debug.h"
#include "wfa_types.h"
#include "wfa_tlv.h"
#include "wfa_tg.h"
#include "wfa_ca.h"
#include "wfa_cmds.h"
#include "wfa_miscs.h"
#include "wfa_agtctrl.h"

extern int gSock;
extern void printProfile(tgProfile_t *);
int wfaStandardBoolParsing (char *str);

/* command KEY WORD String table */
typeNameStr_t keywordStr[] =
{
    { KW_PROFILE,      "profile",       NULL},
    { KW_DIRECTION,    "direction",     NULL},
    { KW_DIPADDR,      "destination",   NULL},
    { KW_DPORT,        "destinationport",  NULL},
    { KW_SIPADDR,      "source",        NULL},
    { KW_SPORT,        "sourceport",    NULL},
    { KW_FRATE,        "framerate",     NULL},
    { KW_DURATION,     "duration",      NULL},
    { KW_PLOAD,        "payloadsize",   NULL},
    { KW_TCLASS,       "trafficClass",  NULL},    /* It is to indicate WMM traffic pattern */
    { KW_STREAMID,     "streamid",      NULL},
    { KW_STARTDELAY,   "startdelay",    NULL},     /* It is used to schedule multi-stream test such as WMM */
    { KW_NUMFRAME,     "numframes",     NULL},
    { KW_USESYNCCLOCK, "useSyncClock",  NULL},
    { KW_USERPRIORITY, "userpriority",  NULL},
    { KW_MAXCNT,       "maxcnt",        NULL},
    { KW_TAGNAME,      "tagName",	    NULL}
};

/* profile type string table */
typeNameStr_t profileStr[] =
{
    { PROF_FILE_TX, "file_transfer", NULL},
    { PROF_MCAST,   "multicast",     NULL},
    { PROF_IPTV,    "iptv",          NULL},       /* This is used for WMM, confused? */
    { PROF_TRANSC,  "transaction",   NULL},       /* keep for temporary backward compat., will be removed */
    { PROF_START_SYNC,    "start_sync",    NULL},
    { PROF_CALI_RTD,    "cali_rtd",    NULL},
    { PROF_UAPSD,  "uapsd",   NULL}
};

/* direction string table */
typeNameStr_t direcStr[] =
{
    { DIRECT_SEND,  "send",          NULL},
    { DIRECT_RECV,  "receive",       NULL}
};

/*
 *  getParamValueInt(): fetch the parameter value based on the parameter name, 
 *                      and stored in provided place holder after converting to
 *                      integer from string.
 *  input:        pcmdStr -- a string pointer to the command string
 *  input:        pParam --  a string pointer to the parameter name
 *  input:        paramValue --  a int pointer to hold the parameter value
 *  return        on success 0 else -1
 */
int getParamValueInt(char *pcmdStr, char *pParam, int *paramValue)
{
    char *str;
    if(strcasecmp(pcmdStr, pParam) == 0)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        paramValue = atoi(str);
        return 0;
    }
    return -1;
}

/*
 *  getParamValueStr(): fetch the parameter value based on the parameter name, 
 *                      and stored in provided place holder as string.
 *                      function explicitly put the null char at last to 
 *                      avoid string with out null in case src string longer.
 *  input:        pcmdStr -- a string pointer to the command string
 *  input:        pParam --  a string pointer to the parameter name
 *  input:        paramValue -- a string pointer to hold the parameter value
 *  input:        paramValLen --  length for the paramValue. 
 *  return        on success 0 else -1
 */
int getParamValueStr(char *pcmdStr, char *pParam, char *paramValue, int paramValLen)
{
    char *str;
    if(strcasecmp(pcmdStr, pParam) == 0)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        strncpy(paramValue, str, (paramValLen - 1));
        paramValue[paramValLen] = 0;
     return 0;
    }
    return -1;
}



/*
 * cmdProcNotDefinedYet(): a dummy function
 */
int cmdProcNotDefinedYet(char *pcmdStr, char *buf, int *len)
{
    printf("The command processing function not defined.\n");

    /* need to send back a response */

    return (WFA_SUCCESS);
}

extern unsigned short wfa_defined_debug;

/*
 *  xcCmdProcGetVersion(): process the command get_version string from TM
 *                         to convert it into a internal format
 *  input:        pcmdStr -- a string pointer to the command string
 */
int xcCmdProcGetVersion(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    DPRINT_INFO(WFA_OUT, "start xcCmdProcGetVersion ...\n");

    if(aBuf == NULL)
        return WFA_FAILURE;

    /* encode the tag without values */
    wfaEncodeTLV(WFA_GET_VERSION_TLV, 0, NULL, aBuf);

    *aLen = 4;

    return WFA_SUCCESS;
}

/*
 *  xcCmdProcAgentConfig(): process the command traffic_agent_config string
 *                          from TM to convert it into a internal format
 *  input:        pcmdStr -- a string pointer to the command string
 */
int xcCmdProcAgentConfig(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    char *str;
    int i = 0, j=0, kwcnt = 0;
    wfaTLV *hdr = (wfaTLV *)aBuf;
    tgProfile_t tgpf = {0, 0, "", -1, "", -1, 0, 0, 0, TG_WMM_AC_BE, 0, 0};
    tgProfile_t *pf = &tgpf;
    int userPrio = 0;

    DPRINT_INFO(WFA_OUT, "start xcCmdProcAgentConfig ...\n");
    DPRINT_INFO(WFA_OUT, "params:  %s\n", pcmdStr);

    if(aBuf == NULL)
        return WFA_FAILURE;

    while((str = strtok_r(NULL, ",", (char **)&pcmdStr)) != NULL)
    {
        for(i = 0; i<sizeof(keywordStr)/sizeof(typeNameStr_t); i++)
        {
            if(strcasecmp(str, keywordStr[i].name) == 0)
            {
                switch(keywordStr[i].type)
                {
                case  KW_PROFILE:
                    str = strtok_r(NULL, ",", (char **)&pcmdStr);
                    if(isString(str) == WFA_FAILURE)
                    {
                        DPRINT_ERR(WFA_ERR, "Incorrect profile keyword format\n");
                        return WFA_FAILURE;
                    }

                    for(j = 0; j < PROF_LAST; j++)
                    {
                        if(strcasecmp(str, profileStr[j].name) == 0)
                        {
                            pf->profile = profileStr[j].type;
                        }
                    }

                    DPRINT_INFO(WFA_OUT, "profile type %i\n", pf->profile);
                    kwcnt++;
                    str = NULL;
                    break;

                case KW_DIRECTION:
                    str = strtok_r(NULL, ",", (char **)&pcmdStr);
                    if(isString(str) == WFA_FAILURE)
                    {
                        DPRINT_ERR(WFA_ERR, "Incorrect direction keyword format\n");
                        return WFA_FAILURE;
                    }

                    if(strcasecmp(str, "send") == 0)
                    {
                        pf->direction = DIRECT_SEND;
                    }
                    else if(strcasecmp(str, "receive") == 0)
                    {
                        pf->direction = DIRECT_RECV;
                    }
                    else
                        printf("Don't know direction\n");

                    DPRINT_INFO(WFA_OUT, "direction %i\n", pf->direction);
                    kwcnt++;
                    str = NULL;
                    break;

                case KW_DIPADDR: /* dest ip address */
                    memcpy(pf->dipaddr, strtok_r(NULL, ",", &pcmdStr), IPV4_ADDRESS_STRING_LEN);
                    if(isIpV4Addr(pf->dipaddr) == WFA_FAILURE)
                    {
                        DPRINT_ERR(WFA_ERR, "Incorrect ipaddr format\n");
                        return WFA_FAILURE;
                    }
                    DPRINT_INFO(WFA_OUT, "dipaddr %s\n", pf->dipaddr);

                    kwcnt++;
                    str = NULL;
                    break;

                case KW_DPORT:
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(isNumber(str) == WFA_FAILURE)
                    {
                        DPRINT_ERR(WFA_ERR, "Incorrect port number format\n");
                        return WFA_FAILURE;
                    }
                    DPRINT_INFO(WFA_OUT, "dport %s\n", str);
                    pf->dport = atoi(str);

                    kwcnt++;
                    str = NULL;
                    break;

                case KW_SIPADDR:
                    memcpy(pf->sipaddr, strtok_r(NULL, ",", &pcmdStr), IPV4_ADDRESS_STRING_LEN);

                    if(isIpV4Addr(pf->sipaddr) == WFA_FAILURE)
                    {
                        DPRINT_ERR(WFA_ERR, "Incorrect ipaddr format\n");
                        return WFA_FAILURE;
                    }
                    DPRINT_INFO(WFA_OUT, "sipaddr %s\n", pf->sipaddr);
                    kwcnt++;
                    str = NULL;
                    break;

                case KW_SPORT:
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(isNumber(str) == WFA_FAILURE)
                    {
                        DPRINT_ERR(WFA_ERR, "Incorrect port number format\n");
                        return WFA_FAILURE;
                    }
                    DPRINT_INFO(WFA_OUT, "sport %s\n", str);
                    pf->sport = atoi(str);

                    kwcnt++;
                    str = NULL;
                    break;

                case KW_FRATE:
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(isNumber(str) == WFA_FAILURE)
                    {
                        DPRINT_ERR(WFA_ERR, "Incorrect frame rate format\n");
                        return WFA_FAILURE;
                    }
                    DPRINT_INFO(WFA_OUT, "framerate %s\n", str);
                    pf->rate = atoi(str);
                    kwcnt++;
                    str = NULL;
                    break;

                case KW_DURATION:
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(isNumber(str) == WFA_FAILURE)
                    {
                        DPRINT_ERR(WFA_ERR, "Incorrect duration format\n");
                        return WFA_FAILURE;
                    }
                    DPRINT_INFO(WFA_OUT, "duration %s\n", str);
                    pf->duration = atoi(str);
                    kwcnt++;
                    str = NULL;
                    break;

                case KW_PLOAD:
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(isNumber(str) == WFA_FAILURE)
                    {
                        DPRINT_ERR(WFA_ERR, "Incorrect payload format\n");
                        return WFA_FAILURE;
                    }
                    DPRINT_INFO(WFA_OUT, "payload %s\n", str);
                    pf->pksize = atoi(str);
                    kwcnt++;
                    str = NULL;
                    break;

                case KW_STARTDELAY:
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(isNumber(str) == WFA_FAILURE)
                    {
                        DPRINT_ERR(WFA_ERR, "Incorrect startDelay format\n");
                        return WFA_FAILURE;
                    }
                    DPRINT_INFO(WFA_OUT, "startDelay %s\n", str);
                    pf->startdelay = atoi(str);
                    kwcnt++;
                    str = NULL;
                    break;

                case KW_MAXCNT:
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(isNumber(str) == WFA_FAILURE)
                    {
                        DPRINT_ERR(WFA_ERR, "Incorrect max count format\n");
                        return WFA_FAILURE;
                    }
                    pf->maxcnt = atoi(str);
                    kwcnt++;
                    str = NULL;
                    break;

                case KW_TCLASS:
                    str = strtok_r(NULL, ",", &pcmdStr);

                    // if user priority is used, tclass is ignored.
                    if(userPrio == 1)
                        break;

                    if(strcasecmp(str, "voice") == 0 )
                    {
                        pf->trafficClass = TG_WMM_AC_VO;
                    }
                    else if(strcasecmp(str, "Video") == 0 )
                    {
                        pf->trafficClass = TG_WMM_AC_VI;
                    }
                    else if(strcasecmp(str, "Background") == 0 )
                    {
                        pf->trafficClass = TG_WMM_AC_BK;
                    }
                    else if(strcasecmp(str, "BestEffort") == 0 )
                    {
                        pf->trafficClass = TG_WMM_AC_BE;
                    }
                    else
                    {
                        pf->trafficClass = TG_WMM_AC_BE;
                    }

                    kwcnt++;
                    str = NULL;
                    break;

                case KW_USERPRIORITY:
                    str = strtok_r(NULL, ",", &pcmdStr);

                    if( strcasecmp(str, "6") == 0 )
                    {
                        pf->trafficClass = TG_WMM_AC_UP6;
                    }
                    else if( strcasecmp(str, "7") == 0 )
                    {
                        pf->trafficClass = TG_WMM_AC_UP7;
                    }
                    else if( strcasecmp(str, "5") == 0 )
                    {
                        pf->trafficClass = TG_WMM_AC_UP5;
                    }
                    else if( strcasecmp(str, "4") == 0 )
                    {
                        pf->trafficClass = TG_WMM_AC_UP4;
                    }
                    else if( strcasecmp(str, "1") == 0 )
                    {
                        pf->trafficClass = TG_WMM_AC_UP1;
                    }
                    else if( strcasecmp(str, "2") == 0 )
                    {
                        pf->trafficClass = TG_WMM_AC_UP2;
                    }
                    else if( strcasecmp(str, "0") == 0 )
                    {
                        pf->trafficClass = TG_WMM_AC_UP0;
                    }
                    else if( strcasecmp(str, "3") == 0)
                    {
                        pf->trafficClass = TG_WMM_AC_UP3;
                    }

                    // if User Priority is used
                    userPrio = 1;

                    kwcnt++;
                    str = NULL;
                    break;

                case KW_STREAMID:
                    kwcnt++;
                    break;

                case KW_NUMFRAME:
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(isNumber(str) == WFA_FAILURE)
                    {
                        DPRINT_ERR(WFA_ERR, "Incorrect numframe format\n");
                        return WFA_FAILURE;
                    }
                    DPRINT_INFO(WFA_OUT, "num frame %s\n", str);
                    kwcnt++;
                    str = NULL;
                    break;

                case KW_USESYNCCLOCK:
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(isNumber(str) == WFA_FAILURE)
                    {
                        DPRINT_ERR(WFA_ERR, "Incorrect sync clock format\n");
                        return WFA_FAILURE;
                    }
                    DPRINT_INFO(WFA_OUT, "sync clock %s\n", str);
                    kwcnt++;
                    str = NULL;
                    break;

                case KW_TAGNAME:
                    str=strtok_r(NULL,",",&pcmdStr);
                    strncpy(pf->WmmpsTagName,str,strlen(str));
                    printf("Got name %s\n",pf->WmmpsTagName);
                    break;

                default:
                    ;
                } /* switch */

                if(str==NULL)
                    break;
            }  /* if */
        } /* for */
    } /* while */

#if 0
    if(kwcnt < 8)
    {
        printf("Incorrect command, missing parameters\n");
        return WFA_FAILURE;
    }
#endif

    printProfile(pf);
    hdr->tag =  WFA_TRAFFIC_AGENT_CONFIG_TLV;
    hdr->len = sizeof(tgProfile_t);

    memcpy(aBuf+4, pf, sizeof(tgpf));

    *aLen = 4+sizeof(tgProfile_t);

    return WFA_SUCCESS;
}

/*
 * xcCmdProcAgentSend(): Process and send the Control command
 *                       "traffic_agent_send"
 * input - pcmdStr  parameter string pointer
 * return - WFA_SUCCESS or WFA_FAILURE;
 */
int xcCmdProcAgentSend(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    wfaTLV *hdr = (wfaTLV *)aBuf;
    char *str, *sid;
    int strid;
    int id_cnt = 0;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, 512);

    DPRINT_INFO(WFA_OUT, "Entering xcCmdProcAgentSend ...\n");
    /* there is only one stream for baseline. Will support
     * multiple streams later.
     */
    str = strtok_r(NULL, ",", &pcmdStr);

    if(str == NULL || str[0] == '\0')
        return WFA_FAILURE;

    /* take the stream ids */
    if(strcasecmp(str, "streamid") != 0)
    {
        DPRINT_ERR(WFA_ERR, "invalid type name\n");
        return WFA_FAILURE;
    }

    /*
     * To handle there are multiple stream ids such as WMM
     */
    while(1)
    {
        sid = strtok_r (NULL, " ", &pcmdStr);
        if(sid == NULL)
            break;

        printf("sid %s\n", sid);
        if(isNumber(sid) == WFA_FAILURE)
            continue;

        strid = atoi(sid);
        printf("id %i\n", strid);
        id_cnt++;

        memcpy(aBuf+4*id_cnt, (char *)&strid, 4);
    }

    hdr->tag =  WFA_TRAFFIC_AGENT_SEND_TLV;
    hdr->len = 4*id_cnt;  /* multiple 4s if more streams */

    *aLen = 4 + 4*id_cnt;

#if 1
    {
        int i;
        for(i = 0; i< *aLen; i++)
            printf("%x ", aBuf[i]);

        printf("\n");
    }
#endif


    return WFA_SUCCESS;
}

/*
 * xcCmdProcAgentReset(): Process and send the Control command
 *                       "traffic_agent_reset"
 * input - pcmdStr  parameter string pointer
 * return - WFA_SUCCESS or WFA_FAILURE;
 */
int xcCmdProcAgentReset(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    wfaTLV *hdr = (wfaTLV *)aBuf;

    DPRINT_INFO(WFA_OUT, "Entering xcCmdProcAgentReset ...\n");

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    hdr->tag =  WFA_TRAFFIC_AGENT_RESET_TLV;
    hdr->len = 0;  /* multiple 4s if more streams */

    *aLen = 4;

    return WFA_SUCCESS;
}

/*
 * xcCmdProcAgentRecvStart(): Process and send the Control command
 *                       "traffic_agent_receive_start"
 * input - pcmdStr  parameter string pointer
 * return - WFA_SUCCESS or WFA_FAILURE;
 */
int xcCmdProcAgentRecvStart(char *pcmdStr, BYTE *aBuf, int *aLen)
{

    wfaTLV *hdr = (wfaTLV *)aBuf;
    char *str, *sid;
    int strid;
    int id_cnt = 0;

    DPRINT_INFO(WFA_OUT, "Entering xcCmdProcAgentRecvStart ...%s\n", pcmdStr);

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    /* there is only one stream for baseline. Will support
     * multiple streams later.
     */
    str = strtok_r(NULL, ",", &pcmdStr);

    if(str == NULL || str[0] == '\0')
    {
        DPRINT_ERR(WFA_ERR, "Null string\n");
        return WFA_FAILURE;
    }


    if(strcasecmp(str, "streamid") != 0)
    {
        DPRINT_ERR(WFA_ERR, "invalid type name\n");
        return WFA_FAILURE;
    }

    while(1)
    {
        sid = strtok_r (NULL, " ", &pcmdStr);
        if(sid == NULL)
            break;

        if(isNumber(sid) == WFA_FAILURE)
            continue;

        strid = atoi(sid);
        id_cnt++;

        memcpy(aBuf+4*id_cnt, (char *)&strid, 4);
    }

    hdr->tag =  WFA_TRAFFIC_AGENT_RECV_START_TLV;
    hdr->len = 4*id_cnt;  /* multiple 4s if more streams */

    *aLen = 4 + 4*id_cnt;

#if 1
    {
        int i;
        for(i = 0; i< *aLen; i++)
            printf("%x ", aBuf[i]);

        printf("\n");
    }
#endif
    return WFA_SUCCESS;
}

/*
 * xcCmdProcAgentRecvStop(): Process and send the Control command
 *                       "traffic_agent_receive_stop"
 * input - pcmdStr  parameter string pointer
 * return - WFA_SUCCESS or WFA_FAILURE;
 */
int xcCmdProcAgentRecvStop(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    wfaTLV *hdr = (wfaTLV *)aBuf;
    char *str, *sid;
    int strid;
    int id_cnt = 0;

    DPRINT_INFO(WFA_OUT, "Entering xcCmdProcAgentRecvStop ...\n");

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    /* there is only one stream for baseline. Will support
     * multiple streams later.
     */
    str = strtok_r(NULL, ",", &pcmdStr);

    if(str == NULL || str[0] == '\0')
        return WFA_FAILURE;

    if(strcasecmp(str, "streamid") != 0)
    {
        DPRINT_ERR(WFA_ERR, "invalid type name\n");
        return WFA_FAILURE;
    }
    while(1)
    {
        sid = strtok_r (NULL, " ", &pcmdStr);
        if(sid == NULL)
            break;

        if(isNumber(sid) == WFA_FAILURE)
            continue;

        strid = atoi(sid);
        id_cnt++;

        memcpy(aBuf+4*id_cnt, (char *)&strid, 4);
    }

    hdr->tag =  WFA_TRAFFIC_AGENT_RECV_STOP_TLV;
    hdr->len = 4*id_cnt;  /* multiple 4s if more streams */

    *aLen = 4 + 4*id_cnt;

    return WFA_SUCCESS;
}

int xcCmdProcAgentSendPing(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    wfaTLV *hdr = (wfaTLV *)aBuf;
    tgPingStart_t *staping = (tgPingStart_t *) (aBuf+sizeof(wfaTLV));
    char *str;
    staping->type = 0;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "destination") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staping->dipaddr, str, 39);
            DPRINT_INFO(WFA_OUT, "destination %s ", staping->dipaddr);
        }
        if(strcasecmp(str, "frameSize") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staping->frameSize=atoi(str);
            DPRINT_INFO(WFA_OUT, "framesize %i ", staping->frameSize);
        }
        if(strcasecmp(str, "frameRate") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staping->frameRate=atof(str);
            DPRINT_INFO(WFA_OUT, "framerate %f ", staping->frameRate);
        }
        if(strcasecmp(str, "duration") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staping->duration=atoi(str);
            DPRINT_INFO(WFA_OUT, "duration %i \n", staping->duration);
        }
        if(strcasecmp(str, "type") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "udp") == 0)
                staping->type = 1;
            else
                staping->type = 0;
        }
        if(strcasecmp(str, "iptype") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staping->iptype=atoi(str);
            DPRINT_INFO(WFA_OUT, "iptype %i \n", staping->iptype);
        }
        if(strcasecmp(str, "dscp") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staping->dscp=atoi(str);
            DPRINT_INFO(WFA_OUT, "dscp %i\n", staping->dscp);
        }
        if(strcasecmp(str, "qos") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "vo") == 0)
            {
                staping->qos = TG_WMM_AC_VO;
            }
            else if(strcasecmp(str, "vi") == 0)
            {
                staping->qos = TG_WMM_AC_VI;
            }
            else if(strcasecmp(str, "be") ==0)
            {
                staping->qos = TG_WMM_AC_BE;
            }
            else if(strcasecmp(str, "bk") == 0)
            {
                staping->qos = TG_WMM_AC_BK;
            }
            else
            {
                // be
                staping->qos = TG_WMM_AC_BE;
            }
        }
    }

    hdr->tag =  WFA_TRAFFIC_SEND_PING_TLV;
    hdr->len = sizeof(tgPingStart_t);

    *aLen = hdr->len + 4;

    return WFA_SUCCESS;
}

int xcCmdProcAgentStopPing(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    wfaTLV *hdr = (wfaTLV *)aBuf;
    char *str;
    int strid;
    str = strtok_r(NULL, ",", &pcmdStr);

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    if(str == NULL || str[0] == '\0')
        return WFA_FAILURE;

    if(strcasecmp(str, "streamid") == 0)
        str = strtok_r(NULL, ",", &pcmdStr);
    else
    {
        DPRINT_ERR(WFA_ERR, "invalid type name\n");
        return WFA_FAILURE;
    }

    if(isNumber(str) == WFA_FAILURE)
        return WFA_FAILURE;

    strid = atoi(str);

    memcpy(aBuf+4, (char *)&strid, 4);

    hdr->tag =  WFA_TRAFFIC_STOP_PING_TLV;
    hdr->len = 4;  /* multiple 4s if more streams */

    *aLen = 8;

    return WFA_SUCCESS;
}

int xcCmdProcStaGetIpConfig(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    int slen;
    char *str = NULL;
    dutCommand_t getipconf;
    memset(&getipconf, 0, sizeof(dutCommand_t));

    DPRINT_INFO(WFA_OUT, "Entering xcCmdProcStaGetIpConfig ...\n");

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    str = strtok_r(NULL, ",", &pcmdStr);
    str = strtok_r(NULL, ",", &pcmdStr);
    if(str == NULL)
        return WFA_FAILURE;


    slen = strlen(str);
    memcpy(getipconf.intf, str, slen);
    wfaEncodeTLV(WFA_STA_GET_IP_CONFIG_TLV, sizeof(dutCommand_t), (BYTE *)&getipconf, aBuf);

    *aLen = 4+sizeof(getipconf);

    return WFA_SUCCESS;
}

int xcCmdProcStaSetIpConfig(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    dutCommand_t staSetIpConfig;
    caStaSetIpConfig_t *setip = (caStaSetIpConfig_t *)&staSetIpConfig.cmdsu.ipconfig;
    caStaSetIpConfig_t defparams = {"", 0, "", "", "", "", ""};
    char *str;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memcpy(setip, &defparams, sizeof(caStaSetIpConfig_t));

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setip->intf, str, 15);
            DPRINT_INFO(WFA_OUT, "interface %s\n", setip->intf);
        }
        else if(strcasecmp(str, "dhcp") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setip->isDhcp = atoi(str);
            DPRINT_INFO(WFA_OUT, "dhcp %i\n", setip->isDhcp);
        }
        else if(strcasecmp(str, "ip") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setip->ipaddr, str, 15);
            DPRINT_INFO(WFA_OUT, "ip %s\n", setip->ipaddr);
        }
        else if(strcasecmp(str, "mask") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setip->mask, str, 15);
            DPRINT_INFO(WFA_OUT, "mask %s\n", setip->mask);
        }
        else if(strcasecmp(str, "defaultGateway") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setip->defGateway, str, 15);
            DPRINT_INFO(WFA_OUT, "gw %s\n", setip->defGateway);
        }
        else if(strcasecmp(str, "primary-dns") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setip->pri_dns, str, 15);
            DPRINT_INFO(WFA_OUT, "dns p %s\n", setip->pri_dns);
        }
        else if(strcasecmp(str, "secondary-dns") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setip->sec_dns, str, 15);
            DPRINT_INFO(WFA_OUT, "dns s %s\n", setip->sec_dns);
        }
        else
        {
            DPRINT_ERR(WFA_ERR, "invalid command %s\n",str);
            return WFA_FAILURE;
        }
    }

    wfaEncodeTLV(WFA_STA_SET_IP_CONFIG_TLV, sizeof(staSetIpConfig), (BYTE *)&staSetIpConfig, aBuf);

    *aLen = 4+sizeof(staSetIpConfig);

    return WFA_SUCCESS;
}

int xcCmdProcStaGetMacAddress(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    int slen;
    char *str = NULL;
    dutCommand_t getmac;

    DPRINT_INFO(WFA_OUT, "Entering xcCmdProcStaGetMacAddress ...\n");

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    memset(&getmac, 0, sizeof(getmac));
    str = strtok_r(NULL, ",", &pcmdStr);
    str = strtok_r(NULL, ",", &pcmdStr);
    if(str == NULL)
        return WFA_FAILURE;

    slen = strlen(str);
    memcpy(getmac.intf, str, slen);
    wfaEncodeTLV(WFA_STA_GET_MAC_ADDRESS_TLV, sizeof(getmac), (BYTE *)&getmac, aBuf);

    *aLen = 4+sizeof(getmac);

    return WFA_SUCCESS;
}

int xcCmdProcStaSetMacAddress(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    char *str = NULL;
    dutCommand_t setmac;

    DPRINT_INFO(WFA_OUT, "Entering xcCmdProcStaSetMacAddress ...\n");

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setmac.intf, str, 15);
            DPRINT_INFO(WFA_OUT, "interface %s\n", setmac.intf);
        }
        else if(strcasecmp(str, "mac") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setmac.cmdsu.macaddr, str, 17);
            DPRINT_INFO(WFA_OUT, "mac %s\n", setmac.cmdsu.macaddr);
        }
    }

    wfaEncodeTLV(WFA_STA_SET_MAC_ADDRESS_TLV, sizeof(setmac), (BYTE *)&setmac, aBuf);

    *aLen = 4+sizeof(setmac);

    return WFA_SUCCESS;
}

int xcCmdProcStaIsConnected(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    int slen;
    char *str = NULL;
    dutCommand_t isconnected;

    DPRINT_INFO(WFA_OUT, "Entering xcCmdProcStaIsConnected\n");

    memset(&isconnected, 0, sizeof(isconnected));

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    str = strtok_r(NULL, ",", &pcmdStr);
    str = strtok_r(NULL, ",", &pcmdStr);
    if(str == NULL)
        return WFA_FAILURE;

    slen = strlen(str);
    memcpy(isconnected.intf, str, slen);
    wfaEncodeTLV(WFA_STA_IS_CONNECTED_TLV, sizeof(isconnected), (BYTE *)&isconnected, aBuf);

    *aLen = 4+sizeof(isconnected);

    return WFA_SUCCESS;
}

int xcCmdProcStaVerifyIpConnection(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    wfaTLV *hdr = (wfaTLV *)aBuf;
    dutCommand_t *verifyip = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    DPRINT_INFO(WFA_OUT, "Entering xcCmdProcStaVerifyIpConnection ...\n");

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(verifyip->intf, str);
            verifyip->intf[15]='\0';
            DPRINT_INFO(WFA_OUT, "interface %s %i\n", verifyip->intf, strlen(verifyip->intf));
        }
        else if(strcasecmp(str, "destination") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(verifyip->cmdsu.verifyIp.dipaddr, str, 15);
            DPRINT_INFO(WFA_OUT, "ip %s\n", verifyip->cmdsu.verifyIp.dipaddr);
        }
        else if(strcasecmp(str, "timeout") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            verifyip->cmdsu.verifyIp.timeout = atoi(str);
            DPRINT_INFO(WFA_OUT, "timeout %i\n", verifyip->cmdsu.verifyIp.timeout);
        }
    }

    wfaEncodeTLV(WFA_STA_VERIFY_IP_CONNECTION_TLV, sizeof(verifyip), (BYTE *)&verifyip, aBuf);

    hdr->tag =  WFA_STA_VERIFY_IP_CONNECTION_TLV;
    hdr->len = sizeof(dutCommand_t);

    *aLen = 4+sizeof(dutCommand_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaGetBSSID(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    char *str = NULL;
    dutCommand_t getbssid;

    DPRINT_INFO(WFA_OUT, "Entering xcCmdProcStaGetBSSID ...\n");

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    memset(&getbssid, 0, sizeof(getbssid));
    str = strtok_r(NULL, ",", &pcmdStr);
    str = strtok_r(NULL, ",", &pcmdStr);
    if(str == NULL)
        return WFA_FAILURE;

    memcpy(getbssid.intf, str, WFA_IF_NAME_LEN-1);
    getbssid.intf[WFA_IF_NAME_LEN-1] = '\0';
    wfaEncodeTLV(WFA_STA_GET_BSSID_TLV, sizeof(getbssid), (BYTE *)&getbssid, aBuf);

    *aLen = 4+sizeof(getbssid);

    return WFA_SUCCESS;
}


int xcCmdProcStaGetStats(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    char *str = NULL;
    dutCommand_t getstats;

    DPRINT_INFO(WFA_OUT, "Entering xcCmdProcStaGetStats ...\n");
    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    memset(&getstats, 0, sizeof(getstats));
    str = strtok_r(NULL, ",", &pcmdStr);
    /* need to check if the parameter name is called interface */
    str = strtok_r(NULL, ",", &pcmdStr);
    if(str == NULL)
        return WFA_FAILURE;

    memcpy(getstats.intf, str, WFA_IF_NAME_LEN-1);
    getstats.intf[WFA_IF_NAME_LEN-1] = '\0';
    wfaEncodeTLV(WFA_STA_GET_STATS_TLV, sizeof(getstats), (BYTE *)&getstats, aBuf);

    *aLen = 4+sizeof(getstats);

    return WFA_SUCCESS;
}


int  xcCmdProcStaSetEncryption(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetEncryption_t *setencryp = (caStaSetEncryption_t *) (aBuf+sizeof(wfaTLV));
    char *str;
    caStaSetEncryption_t defparams = {"", "", 0, {"", "", "", ""}, 0};

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memcpy((void *)setencryp, (void *)&defparams, sizeof(caStaSetEncryption_t));

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setencryp->intf, str, 15);
        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setencryp->ssid, str, 64);
        }
        else if(strcasecmp(str, "encpType") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "wep") == 0)
                setencryp->encpType = ENCRYPT_WEP;
            else
                setencryp->encpType = 0;
        }
        else if(strcasecmp(str, "key1") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy((char *)setencryp->keys[0], str, 26);
            DPRINT_INFO(WFA_OUT, "%s\n", setencryp->keys[0]);
            setencryp->activeKeyIdx = 0;
        }
        else if(strcasecmp(str, "key2") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy((char *)setencryp->keys[1], str, 26);
            DPRINT_INFO(WFA_OUT, "%s\n", setencryp->keys[1]);
        }
        else if(strcasecmp(str, "key3") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy((char *)setencryp->keys[2], str, 26);
            DPRINT_INFO(WFA_OUT, "%s\n", setencryp->keys[2]);
        }
        else if(strcasecmp(str, "key4") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy((char *)setencryp->keys[3], str, 26);
            DPRINT_INFO(WFA_OUT, "%s\n", setencryp->keys[3]);
        }
        else if(strcasecmp(str, "activeKey") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setencryp->activeKeyIdx =  atoi(str);
        }
        else
        {
            DPRINT_INFO(WFA_WNG, "Incorrect Command, check syntax\n");
        }
    }

    wfaEncodeTLV(WFA_STA_SET_ENCRYPTION_TLV, sizeof(caStaSetEncryption_t), (BYTE *)setencryp, aBuf);

    *aLen = 4+sizeof(caStaSetEncryption_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaSetSecurity(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    int ret = WFA_SUCCESS;

#ifndef WFA_PC_CONSOLE
    dutCommand_t *cmd = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
    caStaSetSecurity_t *ssec = &cmd->cmdsu.setsec;
    char *str;
    int secType = 0;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(cmd->intf, str, 15);
        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(ssec->ssid, str, 64);
            DPRINT_INFO(WFA_OUT, "ssid %s\n", ssec->ssid);
        }
        else if(strcasecmp(str, "encpType") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);

            if(strcasecmp(str, "tkip") == 0 || strcasecmp(str, "aes-ccmp") == 0)
                strncpy(ssec->encpType, str, 9);
        }
        else if(strcasecmp(str, "pmf") == 0)
        {
            str = strtok_r (NULL, ",", &pcmdStr);

            if( strcasecmp(str, "optional") == 0)
                ssec->pmf = WFA_OPTIONAL;
            else if(strcasecmp(str, "required") == 0)
                ssec->pmf = WFA_REQUIRED;
            else
                ssec->pmf = WFA_DISABLED;
        }

        else if(strcasecmp(str, "type") == 0)
        {
            /* process the specific type of security */
            str = strtok_r (NULL, ",", &pcmdStr);
            if(strcasecmp(str, "psk") == 0)
            {
                ssec->type = secType = SEC_TYPE_PSK;

                str = strtok_r(NULL, ",", &pcmdStr);
                if(strcasecmp(str, "passphrase") == 0)
                {
                    str = strtok_r(NULL, ",", &pcmdStr);
                    strncpy(ssec->secu.passphrase, str, 64);
                }
            }
            else if(strcasecmp(str, "eaptls") == 0)
            {
                ssec->type = secType = SEC_TYPE_EAPTLS;
            }
            else if(strcasecmp(str, "eapttls") == 0)
            {
                ssec->type = secType = SEC_TYPE_EAPTTLS;
            }
            else if(strcasecmp(str, "eappeap") == 0)
            {
                ssec->type = secType = SEC_TYPE_EAPPEAP;
            }
            else if(strcasecmp(str, "eapsim") == 0)
            {
                ssec->type = secType = SEC_TYPE_EAPSIM;
            }
            else if(strcasecmp(str, "eapfast") == 0)
            {
                ssec->type = secType = SEC_TYPE_EAPFAST;
            }
            else if(strcasecmp(str, "eapaka") == 0)
            {
                ssec->type = secType = SEC_TYPE_EAPAKA;
            }
        }
    }
#endif
    return ret;
}

int xcCmdProcStaSetPSK(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetPSK_t *setencryp = (caStaSetPSK_t *) (aBuf+sizeof(wfaTLV));
#ifndef WFA_PC_CONSOLE
    char *str;
    caStaSetPSK_t defparams = {"", "", "", "", 0, WFA_DISABLED};

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memcpy((void *)setencryp, (void *)&defparams, sizeof(caStaSetPSK_t));

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setencryp->intf, str, 15);
        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setencryp->ssid, str, 64);
            DPRINT_INFO(WFA_OUT, "ssid %s\n", setencryp->ssid);
        }
        else if(strcasecmp(str, "passPhrase") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy((char *)setencryp->passphrase, str, 63);
        }
        else if(strcasecmp(str, "keyMgmtType") == 0)
        {
            str=strtok_r(NULL, ",", &pcmdStr);
            strncpy(setencryp->keyMgmtType, str, 15);
        }
        else if(strcasecmp(str, "encpType") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);

            if(strcasecmp(str, "tkip") == 0)
                setencryp->encpType = ENCRYPT_TKIP;
            else if(strcasecmp(str, "aes-ccmp") == 0)
                setencryp->encpType = ENCRYPT_AESCCMP;
            else if (strcasecmp(str, "aes-ccmp-tkip") == 0)
                setencryp->encpType = ENCRYPT_AESCCMP_TKIP;
        }
        else if(strcasecmp(str, "pmf") == 0)
        {
            str = strtok_r (NULL, ",", &pcmdStr);

            if(strcasecmp(str, "enable") == 0
                    || strcasecmp(str, "optional") == 0)
                setencryp->pmf = WFA_ENABLED;
            else if(strcasecmp(str, "required") == 0)
                setencryp->pmf = WFA_REQUIRED;
            else
                setencryp->pmf = WFA_DISABLED;
        }
        else if (strcasecmp(str, "micAlg") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if (strcasecmp(str, "SHA-1") != 0)
                strncpy(setencryp->micAlg, str, 15);
            else
                strncpy(setencryp->micAlg, "SHA-1", 15);
        }
        else if (strcasecmp(str, "Prog") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setencryp->prog, str, 15);
        }
        else if (strcasecmp(str, "Prefer") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setencryp->prefer = (atoi(str) == 1)?1:0;
        }
    }
#endif
    wfaEncodeTLV(WFA_STA_SET_PSK_TLV, sizeof(caStaSetPSK_t), (BYTE *)setencryp, aBuf);

    *aLen = 4+sizeof(caStaSetPSK_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaSetEapTLS(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetEapTLS_t *setsec = (caStaSetEapTLS_t *) (aBuf+sizeof(wfaTLV));
#ifndef WFA_PC_CONSOLE
    char *str;
    caStaSetEapTLS_t defparams = {"", "", "", "", "", "", "", 0, ""};

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memcpy((void *)setsec, (void *)&defparams, sizeof(caStaSetEapTLS_t));

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->intf, str, 15);
        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->ssid, str, 64);
        }
        else if(strcasecmp(str, "username") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->username, str);
        }
        else if(strcasecmp(str, "keyMgmtType") == 0)
        {
            str=strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->keyMgmtType, str, 8);
        }
        else if(strcasecmp(str, "encpType") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->encrptype, str, 8);
        }
        else if(strcasecmp(str, "trustedRootCA") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->trustedRootCA, str);
        }
        else if(strcasecmp(str, "clientCertificate") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->clientCertificate, str);
        }
        else if(strcasecmp(str, "pmf") == 0)
        {
            str = strtok_r (NULL, ",", &pcmdStr);

            if(strcasecmp(str, "enable") == 0
                    || strcasecmp(str, "optional") == 0)
                setsec->pmf = WFA_ENABLED;
            else if(strcasecmp(str, "required") == 0)
                setsec->pmf = WFA_REQUIRED;
            else if(strcasecmp(str, "forced_required") == 0)
                setsec->pmf = WFA_F_REQUIRED;
            else if(strcasecmp(str, "forced_disabled") == 0)
                setsec->pmf = WFA_F_DISABLED;
            else
                setsec->pmf = WFA_DISABLED;
        }
        else if(strcasecmp(str, "micAlg") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "SHA-1") != 0)
                strncpy(setsec->micAlg, str, 15);
            else
                strncpy(setsec->micAlg, "SHA-1", 15);
        }
    }

#endif
    wfaEncodeTLV(WFA_STA_SET_EAPTLS_TLV, sizeof(caStaSetEapTLS_t), (BYTE *)setsec, aBuf);

    *aLen = 4+sizeof(caStaSetEapTLS_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaSetEapTTLS(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetEapTTLS_t *setsec = (caStaSetEapTTLS_t *) (aBuf+sizeof(wfaTLV));
#ifndef WFA_PC_CONSOLE
    caStaSetEapTTLS_t defparams = {"", "", "", "", "", "", "", ""};
    char *str;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memcpy((void *)setsec, (void *)&defparams, sizeof(caStaSetEapTTLS_t));

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->intf, str, 15);
        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->ssid, str, 64);
        }
        else if(strcasecmp(str, "username") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->username, str);
        }
        else if(strcasecmp(str, "password") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->passwd, str);
        }
        else if(strcasecmp(str, "keyMgmtType") == 0)
        {
            str=strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->keyMgmtType, str, 7);
        }
        else if(strcasecmp(str, "encpType") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->encrptype, str, 8);
        }
        else if(strcasecmp(str, "trustedRootCA") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->trustedRootCA, str);
        }
        else if(strcasecmp(str, "clientCertificate") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->clientCertificate, str);
        }
        else if(strcasecmp(str, "pmf") == 0)
        {
            str = strtok_r (NULL, ",", &pcmdStr);

            if(strcasecmp(str, "enable") == 0
                    || strcasecmp(str, "optional") == 0)
                setsec->pmf = WFA_ENABLED;
            else if(strcasecmp(str, "required") == 0)
                setsec->pmf = WFA_REQUIRED;
            else
                setsec->pmf = WFA_DISABLED;
        }
        else if (strcasecmp(str, "micALg") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if (strcasecmp(str, "SHA-1") != 0)
                strncpy(setsec->micAlg, str, 15);
            else
                strncpy(setsec->micAlg, "SHA-1", 15);
        }
        else if (strcasecmp(str, "Prog") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->prog, str, 15);
        }
        else if (strcasecmp(str, "Prefer") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setsec->prefer = (atoi(str) == 1)?1:0;
        }
    }

#endif
    wfaEncodeTLV(WFA_STA_SET_EAPTTLS_TLV, sizeof(caStaSetEapTTLS_t), (BYTE *)setsec, aBuf);

    *aLen = 4+sizeof(caStaSetEapTTLS_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaSetEapSIM(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetEapSIM_t *setsec = (caStaSetEapSIM_t *) (aBuf+sizeof(wfaTLV));
#ifndef WFA_PC_CONSOLE
    char *str;
    caStaSetEapSIM_t defparams = {"", "", "", "", "", "", 0, {"", "", ""}};

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memcpy((void *)setsec, (void *)&defparams, sizeof(caStaSetEapSIM_t));

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->intf, str, 15);
        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->ssid, str, 64);
        }
        else if(strcasecmp(str, "username") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->username, str);
        }
        else if(strcasecmp(str, "password") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->passwd, str);
        }
        else if(strcasecmp(str, "keyMgmtType") == 0)
        {
            str=strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->keyMgmtType, str, 7);
        }
        else if(strcasecmp(str, "encpType") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->encrptype, str,8);
        }
        else if(strcasecmp(str, "triplet1") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy((char *)setsec->tripletSet[0], str, 63);
            DPRINT_INFO(WFA_OUT, "Triplet1 : %s\n", setsec->tripletSet[0]);
            setsec->tripletCount = 1;
        }
        else if(strcasecmp(str, "triplet2") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy((char *)setsec->tripletSet[1], str, 63);
            DPRINT_INFO(WFA_OUT, "Triplet2 : %s\n", setsec->tripletSet[1]);
            setsec->tripletCount=2;
        }
        else if(strcasecmp(str, "triplet3") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy((char *)setsec->tripletSet[2], str, 63);
            DPRINT_INFO(WFA_OUT, "Triplet1 : %s\n", setsec->tripletSet[2]);
            setsec->tripletCount = 3;
        }
        else if(strcasecmp(str, "pmf") == 0)
        {
            str = strtok_r (NULL, ",", &pcmdStr);

            if(strcasecmp(str, "enable") == 0
                    || strcasecmp(str, "optional") == 0)
                setsec->pmf = WFA_ENABLED;
            else if(strcasecmp(str, "required") == 0)
                setsec->pmf = WFA_REQUIRED;
            else
                setsec->pmf = WFA_DISABLED;
        }
    }

#endif
    wfaEncodeTLV(WFA_STA_SET_EAPSIM_TLV, sizeof(caStaSetEapSIM_t), (BYTE *)setsec, aBuf);

    *aLen = 4+sizeof(caStaSetEapSIM_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaSetPEAP(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetEapPEAP_t *setsec = (caStaSetEapPEAP_t *) (aBuf+sizeof(wfaTLV));
#ifndef WFA_PC_CONSOLE
    char *str;
    caStaSetEapPEAP_t defparams = {"", "", "", "", "", "", "", "", 0};

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memcpy((void *)setsec, (void *)&defparams, sizeof(caStaSetEapPEAP_t));

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->intf, str, 15);
        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->ssid, str, 64);
        }
        else if(strcasecmp(str, "username") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->username, str);
        }
        else if(strcasecmp(str, "password") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->passwd, str);
        }
        else if(strcasecmp(str, "keyMgmtType") == 0)
        {
            str=strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->keyMgmtType, str, 7);
        }
        else if(strcasecmp(str, "encpType") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->encrptype, str, 8);
        }
        else if(strcasecmp(str, "innerEAP") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->innerEAP, str);
        }
        else if(strcasecmp(str, "trustedRootCA") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->trustedRootCA, str,31);
        }
        else if(strcasecmp(str, "peapVersion") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setsec->peapVersion = atoi(str);
        }
        else if(strcasecmp(str, "pmf") == 0)
        {
            str = strtok_r (NULL, ",", &pcmdStr);

            if(strcasecmp(str, "enable") == 0
                    || strcasecmp(str, "optional") == 0)
                setsec->pmf = WFA_ENABLED;
            else if(strcasecmp(str, "required") == 0)
                setsec->pmf = WFA_REQUIRED;
            else
                setsec->pmf = WFA_DISABLED;
        }
    }

#endif
    wfaEncodeTLV(WFA_STA_SET_PEAP_TLV, sizeof(caStaSetEapPEAP_t), (BYTE *)setsec, aBuf);

    *aLen = 4+sizeof(caStaSetEapPEAP_t);

    return WFA_SUCCESS;
}


int xcCmdProcStaSetIBSS(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetIBSS_t *setibss = (caStaSetIBSS_t *) (aBuf+sizeof(wfaTLV));
    char *str;
    int i = 0;
    caStaSetIBSS_t defparams = {"", "", 0, 0, {"", "", "", ""}, 0xFF};

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memcpy((void *)setibss, (void *)&defparams, sizeof(caStaSetIBSS_t));

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setibss->intf, str, 15);
            DPRINT_INFO(WFA_OUT, "interface %s\n", setibss->intf);

        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setibss->ssid, str, 64);
            DPRINT_INFO(WFA_OUT, "ssid %s\n", setibss->ssid);
        }
        else if(strcasecmp(str, "channel") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setibss->channel = atoi(str);
        }
        else if(strcasecmp(str, "encpType") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "wep") == 0)
                setibss->encpType = ENCRYPT_WEP;
            else
                setibss->encpType = 0;
        }
        else if(strncasecmp(str, "key1", 4) == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setibss->keys[i++], str, 26);
            setibss->activeKeyIdx = 0;
        }
        else if(strncasecmp(str, "key2", 4) == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setibss->keys[i++], str, 26);
        }
        else if(strncasecmp(str, "key3", 4) == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setibss->keys[i++], str, 26);
        }
        else if(strncasecmp(str, "key4", 4) == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setibss->keys[i++], str, 26);
        }
        else if(strcasecmp(str, "activeKey") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setibss->activeKeyIdx = atoi(str);
        }
    }

    wfaEncodeTLV(WFA_STA_SET_IBSS_TLV, sizeof(caStaSetIBSS_t), (BYTE *)setibss, aBuf);

    *aLen = 4+sizeof(caStaSetIBSS_t);

    return WFA_SUCCESS;
}

int xcCmdProcDeviceGetInfo(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    dutCommand_t *dutCmd = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
    caDevInfo_t *dinfo = &dutCmd->cmdsu.dev;
    char *str;

    if(aBuf == NULL)
        return WFA_FAILURE;

    printf("entering device get info\n");
    memset(aBuf, 0, *aLen);

    dinfo->fw = 0;
    str = strtok_r(NULL, ",", &pcmdStr);
    if(str != NULL && str[0] != '\0')
    {
        if(strcasecmp(str, "firmware") == 0)
        {
            dinfo->fw = 1;
        }
    }

    wfaEncodeTLV(WFA_DEVICE_GET_INFO_TLV, sizeof(dutCommand_t), (BYTE *)dutCmd, aBuf);

    *aLen = 4 + sizeof(dutCommand_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaGetInfo(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    char *str;
    dutCommand_t *getInfo = (dutCommand_t *) (aBuf+sizeof(wfaTLV));

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    str = strtok_r(NULL, ",", &pcmdStr);
    if(str == NULL || str[0] == '\0')
        return WFA_FAILURE;

    if(strcasecmp(str, "interface") == 0)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        strncpy(getInfo->intf, str, 15);
        DPRINT_INFO(WFA_OUT, "interface %s\n", getInfo->intf);

    }

    wfaEncodeTLV(WFA_STA_GET_INFO_TLV, sizeof(dutCommand_t), (BYTE *)getInfo, aBuf);

    *aLen = 4 + sizeof(dutCommand_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaUpload(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    char *str;
    dutCommand_t *dutCmd = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
    caStaUpload_t *tdp = &dutCmd->cmdsu.upload;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    str = strtok_r(NULL, ",", &pcmdStr);
    if(str == NULL || str[0] == '\0')
        return WFA_FAILURE;

    if(strcasecmp(str, "test") == 0)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(strcasecmp(str, "voice") == 0)
        {
            tdp->type = WFA_UPLOAD_VHSO_RPT;
            DPRINT_INFO(WFA_OUT, "testdata voice %i\n", tdp->type);
            str = strtok_r(NULL, ",", &pcmdStr);
            tdp->next = atoi(str);
        }
    }

    wfaEncodeTLV(WFA_STA_UPLOAD_TLV, sizeof(dutCommand_t), (BYTE *)dutCmd, aBuf);

    *aLen = 4 + sizeof(dutCommand_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaAssociate(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    dutCommand_t *setassoc = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
    char *str;
    caStaAssociate_t *assoc = &setassoc->cmdsu.assoc;
    caStaAssociate_t defparams = {"", "", WFA_DISABLED};

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memcpy(assoc, &defparams, sizeof(caStaAssociate_t));

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setassoc->intf, str, 15);
            DPRINT_INFO(WFA_OUT, "interface %s\n", setassoc->intf);

        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setassoc->cmdsu.assoc.ssid, str, 64);
            DPRINT_INFO(WFA_OUT, "ssid %s\n", setassoc->cmdsu.assoc.ssid);
        }
        else if(strcasecmp(str, "bssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setassoc->cmdsu.assoc.bssid, str, 17);
            DPRINT_INFO(WFA_OUT, "bssid %s\n", setassoc->cmdsu.assoc.bssid);
        }
        else if(strcasecmp(str, "wps") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "enabled") == 0)
                setassoc->cmdsu.assoc.wps = WFA_ENABLED;
        }
    }

    wfaEncodeTLV(WFA_STA_ASSOCIATE_TLV, sizeof(dutCommand_t), (BYTE *)setassoc, aBuf);

    *aLen = 4+sizeof(dutCommand_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaReAssociate(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    dutCommand_t *setassoc = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
    char *str;
    caStaAssociate_t *assoc = &setassoc->cmdsu.assoc;
    caStaAssociate_t defparams = {"", "", WFA_DISABLED};

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memcpy(assoc, &defparams, sizeof(caStaAssociate_t));

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setassoc->intf, str, 15);
            DPRINT_INFO(WFA_OUT, "interface %s\n", setassoc->intf);

        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setassoc->cmdsu.assoc.ssid, str, 64);
            DPRINT_INFO(WFA_OUT, "ssid %s\n", setassoc->cmdsu.assoc.ssid);
        }
        else if(strcasecmp(str, "bssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setassoc->cmdsu.assoc.bssid, str, 17);
            DPRINT_INFO(WFA_OUT, "bssid %s\n", setassoc->cmdsu.assoc.bssid);
        }
    }

    wfaEncodeTLV(WFA_STA_REASSOCIATE_TLV, sizeof(dutCommand_t), (BYTE *)setassoc, aBuf);

    *aLen = 4+sizeof(dutCommand_t);

    return WFA_SUCCESS;
}

int xcCmdProcDeviceListIF(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    dutCommand_t *getdevlist = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    str = strtok_r(NULL, ",", &pcmdStr);
    if(str == NULL || str[0] == '\0')
        return WFA_FAILURE;

    if(strcasecmp(str, "interfaceType") == 0)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(strcmp(str, "802.11") == 0)
            getdevlist->cmdsu.iftype = IF_80211;

        DPRINT_INFO(WFA_OUT, "interface type %i\n", getdevlist->cmdsu.iftype);
    }

    wfaEncodeTLV(WFA_DEVICE_LIST_IF_TLV, sizeof(dutCommand_t), (BYTE *)getdevlist, aBuf);

    *aLen = 4 + sizeof(dutCommand_t);

#if DEBUG
    for(i = 0; i< len; i++)
        printf("%x ", buf[i]);

    printf("\n");
#endif

    return WFA_SUCCESS;
}

int xcCmdProcStaSetUAPSD(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetUAPSD_t *setuapsd = (caStaSetUAPSD_t *) (aBuf+sizeof(wfaTLV));
    char *str;
    wfaTLV *hdr = (wfaTLV *)aBuf;
    caStaSetUAPSD_t defparams = {"", "", 0, 0, 0, 0, 0};

    DPRINT_INFO(WFA_OUT, "start xcCmdProcAgentConfig ...\n");
    DPRINT_INFO(WFA_OUT, "params:  %s\n", pcmdStr);
    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memcpy((void *)setuapsd, (void *)&defparams, sizeof(caStaSetUAPSD_t));
    setuapsd->acBE = 0;
    setuapsd->acBK = 0;
    setuapsd->acVI = 0;
    setuapsd->acVO = 0;

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setuapsd->intf, str, 15);
        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setuapsd->ssid, str, 64);
        }
        else if(strcasecmp(str, "maxSP") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setuapsd->maxSPLength = atoi(str);
        }
        else if(strcasecmp(str, "acBE") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setuapsd->acBE = atoi(str);

        }
        else if(strcasecmp(str, "acBK") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setuapsd->acBK = atoi(str);

        }
        else if(strcasecmp(str, "acVI") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setuapsd->acVI = atoi(str);
        }
        else if(strcasecmp(str, "acVO") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setuapsd->acVO = atoi(str);
        }
        else if(strcasecmp(str, "type") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setuapsd->type = atoi(str);
        }
        else if(strcasecmp(str, "peer") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setuapsd->peer, str, 17);
        }
    }
    hdr->tag =  WFA_STA_SET_UAPSD_TLV;
    hdr->len = sizeof(caStaSetUAPSD_t);

    memcpy(aBuf+4, setuapsd, sizeof(caStaSetUAPSD_t));

    *aLen = 4+sizeof(caStaSetUAPSD_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaDebugSet(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    dutCommand_t *debugSet = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "level") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(atoi(str) == WFA_DEBUG_INFO || WFA_DEBUG_WARNING)
            {
                debugSet->cmdsu.dbg.level = atoi(str);
                DPRINT_INFO(WFA_OUT, "dbg level %i\n", debugSet->cmdsu.dbg.level);
            }
            else
                return WFA_FAILURE;  /* not support */

        }
        else if(strcasecmp(str, "enable") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            printf("enable %i\n", atoi(str));
            switch(atoi(str)) /* enable */
            {
            case 1:
                debugSet->cmdsu.dbg.state = 1;
                printf("enable\n");
                break;
            case 0:
                debugSet->cmdsu.dbg.state = 0;
                printf("disable\n");
                break;
            default:
                printf("wrong\n");
                return WFA_FAILURE;  /* command invalid */
            }
        }
    }

    wfaEncodeTLV(WFA_STA_DEBUG_SET_TLV, sizeof(dutCommand_t), (BYTE *)debugSet, aBuf);

    *aLen = 4 + sizeof(dutCommand_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaSetMode(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetMode_t *setmode = (caStaSetMode_t *) (aBuf+sizeof(wfaTLV));
    char *str;
    caStaSetMode_t defparams = {"", "", 0, 0, 0, {"", "", "", ""}, 0xFF};

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memcpy((void *)setmode, (void *)&defparams, sizeof(caStaSetMode_t));

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setmode->intf, str, 15);
        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setmode->ssid, str, 64);
        }
        else if(strcasecmp(str, "encpType") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "wep") == 0)
                setmode->encpType = ENCRYPT_WEP;
            else
                setmode->encpType = 0;
        }
        else if(strcasecmp(str, "key1") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy((char *)setmode->keys[0], str, 26);
            DPRINT_INFO(WFA_OUT, "%s\n", setmode->keys[0]);
            setmode->activeKeyIdx = 0;
        }
        else if(strcasecmp(str, "key2") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy((char *)setmode->keys[1], str, 26);
            DPRINT_INFO(WFA_OUT, "%s\n", setmode->keys[1]);
        }
        else if(strcasecmp(str, "key3") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy((char *)setmode->keys[2], str, 26);
            DPRINT_INFO(WFA_OUT, "%s\n", setmode->keys[2]);
        }
        else if(strcasecmp(str, "key4") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy((char *)setmode->keys[3], str, 26);
            DPRINT_INFO(WFA_OUT, "%s\n", setmode->keys[3]);
        }
        else if(strcasecmp(str, "activeKey") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setmode->activeKeyIdx =  atoi(str);
        }
        else if(strcasecmp(str, "mode") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            printf("\r\n mode is %s\n",str);
            if(strcasecmp(str, "adhoc") == 0)
                setmode->mode = 1;
            else
                setmode->mode = 0;
        }
        else if(strcasecmp(str, "channel") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setmode->channel = atoi(str);
        }
        else
        {
            DPRINT_INFO(WFA_WNG, "Incorrect Command, check syntax\n");
            printf("\r\n mode is %s\n",str);
        }
    }

    wfaEncodeTLV(WFA_STA_SET_MODE_TLV, sizeof(caStaSetMode_t), (BYTE *)setmode, aBuf);
    *aLen = 4+sizeof(caStaSetMode_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaGetP2pDevAddress(char * pcmdStr,BYTE * aBuf,int * aLen)
{
    dutCommand_t *getP2pDevAdd= (dutCommand_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(getP2pDevAdd->intf, str, WFA_IF_NAME_LEN-1 );
            getP2pDevAdd->intf[WFA_IF_NAME_LEN-1] = '\0';
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_GET_DEV_ADDRESS_TLV, sizeof(dutCommand_t), (BYTE *)getP2pDevAdd, aBuf);

    *aLen = 4+sizeof(dutCommand_t);

    return TRUE;
}

int xcCmdProcStaSetP2p(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetP2p_t *staSetP2p= (caStaSetP2p_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSetP2p->intf, str, WFA_IF_NAME_LEN-1);
            staSetP2p->intf[WFA_IF_NAME_LEN-1]='\0';

        }
        else if(strcasecmp(str, "listen_chn") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->listen_chn= atoi(str);
            staSetP2p->listen_chn_flag =1;
        }
        else if(strcasecmp(str, "p2p_mode") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSetP2p->p2p_mode, str, 15);
            staSetP2p->p2p_mode_flag = 1;
        }
        else if(strcasecmp(str, "persistent") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->presistent= atoi(str);
            staSetP2p->presistent_flag = 1;
        }
        else if(strcasecmp(str, "intra_bss") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->intra_bss= atoi(str);
            staSetP2p->intra_bss_flag = 1;
        }
        else if(strcasecmp(str, "noa_duration") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->noa_duration= atoi(str);
            staSetP2p->noa_duration_flag = 1;
        }
        else if(strcasecmp(str, "noa_interval") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->noa_interval= atoi(str);
            staSetP2p->noa_interval_flag = 1;
        }
        else if(strcasecmp(str, "noa_count") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->noa_count= atoi(str);
            staSetP2p->noa_count_flag = 1;
        }
        else if(strcasecmp(str, "concurrency") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->concurrency= atoi(str);
            staSetP2p->concurrency_flag= 1;
        }
        else if(strcasecmp(str, "p2pinvitation") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->p2p_invitation= atoi(str);
            staSetP2p->p2p_invitation_flag= 1;
        }
        else if(strcasecmp(str, "bcn_int") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->bcn_int= atoi(str);
            staSetP2p->bcn_int_flag= 1;
        }
        else if(strcasecmp(str, "ext_listen_time_interval") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->ext_listen_time_int= atoi(str);
            staSetP2p->ext_listen_time_int_flag= 1;
        }
        else if(strcasecmp(str, "ext_listen_time_period") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->ext_listen_time_period= atoi(str);
            staSetP2p->ext_listen_time_period_flag= 1;
        }
        else if(strcasecmp(str, "discoverability") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->discoverability= atoi(str);
            staSetP2p->discoverability_flag= 1;
        }
        else if(strcasecmp(str, "service_discovery") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->service_discovery= atoi(str);
            staSetP2p->service_discovry_flag= 1;
        }
        else if(strcasecmp(str, "crossconnection") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->crossconnection= atoi(str);
            staSetP2p->crossconnection_flag= 1;
        }
        else if(strcasecmp(str, "p2pmanaged") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->p2pmanaged= atoi(str);
            staSetP2p->p2pmanaged_flag= 1;
        }
        else if(strcasecmp(str, "go_apsd") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetP2p->go_apsd= atoi(str);
            staSetP2p->go_apsd_flag= 1;
        }
        else if(strcasecmp(str, "DiscoverType") == 0)
        {
            staSetP2p->discover_type_flag= 1;

            str = strtok_r(NULL, ",", &pcmdStr);
            printf("DiscoverType is %s\n", str);
            if(strcasecmp(str, "WFD") == 0)
                staSetP2p->discoverType= 1;
            else if (strcasecmp(str, "P2P") == 0)
                staSetP2p->discoverType = 2;
            else if (strcasecmp(str, "TDLS") == 0)
                staSetP2p->discoverType = 3;
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_SETP2P_TLV, sizeof(caStaSetP2p_t), (BYTE *)staSetP2p, aBuf);

    *aLen = 4+sizeof(caStaSetP2p_t);

    return TRUE;
}


int xcCmdProcStaP2pConnect(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaP2pConnect_t *staP2pConnect= (caStaP2pConnect_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staP2pConnect->intf, str,WFA_IF_NAME_LEN-1);
            staP2pConnect->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "groupid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staP2pConnect->grpid, str, WFA_P2P_GRP_ID_LEN-1);
            staP2pConnect->grpid[WFA_P2P_GRP_ID_LEN-1]='\0';
        }
        else if(strcasecmp(str, "p2pdevid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staP2pConnect->devId, str, WFA_P2P_DEVID_LEN-1);
            staP2pConnect->devId[WFA_P2P_DEVID_LEN-1]='\0';
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_CONNECT_TLV, sizeof(caStaP2pConnect_t), (BYTE *)staP2pConnect, aBuf);

    *aLen = 4+sizeof(caStaP2pConnect_t);

    return TRUE;
}



int xcCmdProcStaP2pStartGroupFormation(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaP2pStartGrpForm_t *staP2pStartGrpForm= (caStaP2pStartGrpForm_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staP2pStartGrpForm->intf, str, WFA_IF_NAME_LEN-1);
            staP2pStartGrpForm->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "p2pdevid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staP2pStartGrpForm->devId, str, WFA_P2P_DEVID_LEN-1);
            staP2pStartGrpForm->devId[WFA_P2P_DEVID_LEN-1]='\0';
        }
        else if(strcasecmp(str, "intent_val") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staP2pStartGrpForm->intent_val= atoi(str);
        }
        else if(strcasecmp(str, "init_go_neg") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staP2pStartGrpForm->init_go_neg= atoi(str);
        }
        else if(strcasecmp(str, "oper_chn") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staP2pStartGrpForm->oper_chn= atoi(str);
            staP2pStartGrpForm->oper_chn_flag=1;
        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staP2pStartGrpForm->ssid, str, WFA_SSID_NAME_LEN-1);
            staP2pStartGrpForm->ssid[WFA_SSID_NAME_LEN-1]='\0';
            staP2pStartGrpForm->ssid_flag =1;
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_START_GRP_FORMATION_TLV, sizeof(caStaP2pStartGrpForm_t), (BYTE *)staP2pStartGrpForm, aBuf);

    *aLen = 4+sizeof(caStaP2pStartGrpForm_t);

    return TRUE;
}

int xcCmdProcStaP2pDissolve(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaP2pDissolve_t *staP2pDissolve= (caStaP2pDissolve_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staP2pDissolve->intf, str, WFA_IF_NAME_LEN-1);
            staP2pDissolve->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "groupid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staP2pDissolve->grpId, str, WFA_P2P_GRP_ID_LEN-1);
            staP2pDissolve->grpId[WFA_P2P_GRP_ID_LEN-1]='\0';
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_DISSOLVE_TLV, sizeof(caStaP2pDissolve_t), (BYTE *)staP2pDissolve, aBuf);

    *aLen = 4+sizeof(caStaP2pDissolve_t);

    return TRUE;
}

int xcCmdProcStaSendP2pInvReq(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSendP2pInvReq_t *staSendP2pInvReq= (caStaSendP2pInvReq_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSendP2pInvReq->intf, str, WFA_IF_NAME_LEN-1);
            staSendP2pInvReq->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "groupid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSendP2pInvReq->grpId, str, WFA_P2P_GRP_ID_LEN-1);
            staSendP2pInvReq->grpId[WFA_P2P_GRP_ID_LEN-1]='\0';
        }
        else if(strcasecmp(str, "p2pdevid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSendP2pInvReq->devId, str, WFA_P2P_DEVID_LEN-1);
            staSendP2pInvReq->devId[WFA_P2P_DEVID_LEN-1]='\0';
        }
        else if(strcasecmp(str, "reinvoke") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSendP2pInvReq->reinvoke= atoi(str);
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_SEND_INV_REQ_TLV, sizeof(caStaSendP2pInvReq_t), (BYTE *)staSendP2pInvReq, aBuf);

    *aLen = 4+sizeof(caStaSendP2pInvReq_t);

    return TRUE;
}

int xcCmdProcStaAcceptP2pInvReq(char *pcmdStr, BYTE *aBuf, int *aLen)
{

    caStaAcceptP2pInvReq_t *staAccceptP2pInvReq= (caStaAcceptP2pInvReq_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staAccceptP2pInvReq->intf, str, WFA_IF_NAME_LEN-1);
            staAccceptP2pInvReq->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "groupid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staAccceptP2pInvReq->grpId, str, WFA_P2P_GRP_ID_LEN-1);
            staAccceptP2pInvReq->grpId[WFA_P2P_GRP_ID_LEN-1]='\0';
        }
        else if(strcasecmp(str, "p2pdevid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staAccceptP2pInvReq->devId, str, WFA_P2P_DEVID_LEN-1);
            staAccceptP2pInvReq->devId[WFA_P2P_DEVID_LEN-1]='\0';
        }
        else if(strcasecmp(str, "reinvoke") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staAccceptP2pInvReq->reinvoke= atoi(str);
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_ACCEPT_INV_REQ_TLV, sizeof(caStaAcceptP2pInvReq_t), (BYTE *)staAccceptP2pInvReq, aBuf);

    *aLen = 4+sizeof(caStaAcceptP2pInvReq_t);

    return TRUE;
}



int xcCmdProcStaSendP2pProvDisReq(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSendP2pProvDisReq_t *staSendP2pProvDisReq= (caStaSendP2pProvDisReq_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSendP2pProvDisReq->intf, str, WFA_IF_NAME_LEN-1);
            staSendP2pProvDisReq->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "configmethod") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSendP2pProvDisReq->confMethod, str, 15);
        }
        else if(strcasecmp(str, "p2pdevid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSendP2pProvDisReq->devId, str, WFA_P2P_DEVID_LEN-1);
            staSendP2pProvDisReq->devId[WFA_P2P_DEVID_LEN-1]='\0';
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_SEND_PROV_DIS_REQ_TLV, sizeof(caStaSendP2pProvDisReq_t), (BYTE *)staSendP2pProvDisReq, aBuf);

    *aLen = 4+sizeof(caStaSendP2pProvDisReq_t);

    return TRUE;
}

int xcCmdProcStaSetWpsPbc(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetWpsPbc_t *staSetWpsPbc= (caStaSetWpsPbc_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSetWpsPbc->intf, str, WFA_IF_NAME_LEN-1);
            staSetWpsPbc->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "groupid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSetWpsPbc->grpId, str, WFA_P2P_GRP_ID_LEN-1);
            staSetWpsPbc->grpId[WFA_P2P_GRP_ID_LEN-1]='\0';
            staSetWpsPbc->grpid_flag=1;
        }
    }

    wfaEncodeTLV(WFA_STA_WPS_SETWPS_PBC_TLV, sizeof(caStaSetWpsPbc_t), (BYTE *)staSetWpsPbc, aBuf);

    *aLen = 4+sizeof(caStaSetWpsPbc_t);

    return TRUE;
}

int xcCmdProcStaWpsReadPin(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaWpsReadPin_t *staWpsReadPin= (caStaWpsReadPin_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staWpsReadPin->intf, str, WFA_IF_NAME_LEN-1);
            staWpsReadPin->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "groupid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staWpsReadPin->grpId, str, WFA_P2P_GRP_ID_LEN-1);
            staWpsReadPin->grpId[WFA_P2P_GRP_ID_LEN-1]='\0';
            staWpsReadPin->grpid_flag=1;
        }
    }

    wfaEncodeTLV(WFA_STA_WPS_READ_PIN_TLV, sizeof(caStaWpsReadPin_t), (BYTE *)staWpsReadPin, aBuf);

    *aLen = 4+sizeof(caStaWpsReadPin_t);

    return TRUE;
}


int xcCmdProcStaWpsReadLabel(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaWpsReadLabel_t *staWpsReadLabel= (caStaWpsReadLabel_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staWpsReadLabel->intf, str, WFA_IF_NAME_LEN-1);
            staWpsReadLabel->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "groupid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staWpsReadLabel->grpId, str, WFA_P2P_GRP_ID_LEN-1);
            staWpsReadLabel->grpId[WFA_P2P_GRP_ID_LEN-1]='\0';
            staWpsReadLabel->grpid_flag=1;
        }
    }

    wfaEncodeTLV(WFA_STA_WPS_READ_LABEL_TLV, sizeof(caStaWpsReadLabel_t), (BYTE *)staWpsReadLabel, aBuf);

    *aLen = 4+sizeof(caStaWpsReadLabel_t);

    return TRUE;
}


int xcCmdProcStaWpsEnterPin(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaWpsEnterPin_t *staWpsEnterPin= (caStaWpsEnterPin_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staWpsEnterPin->intf, str, WFA_IF_NAME_LEN-1);
            staWpsEnterPin->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "pin") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staWpsEnterPin->wpsPin, str, WFA_WPS_PIN_LEN-1);
            staWpsEnterPin->wpsPin[WFA_WPS_PIN_LEN-1]='\0';
        }
        else if(strcasecmp(str, "groupid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staWpsEnterPin->grpId, str, WFA_P2P_GRP_ID_LEN-1);
            staWpsEnterPin->grpId[WFA_P2P_GRP_ID_LEN-1]='\0';
            staWpsEnterPin->grpid_flag=1;
        }
    }

    wfaEncodeTLV(WFA_STA_WPS_ENTER_PIN_TLV, sizeof(caStaWpsEnterPin_t), (BYTE *)staWpsEnterPin, aBuf);

    *aLen = 4+sizeof(caStaWpsEnterPin_t);

    return TRUE;
}

int xcCmdProcStaGetPsk(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaGetPsk_t *staGetPsk= (caStaGetPsk_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staGetPsk->intf, str, WFA_IF_NAME_LEN-1);
            staGetPsk->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "groupid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staGetPsk->grpId, str, WFA_P2P_GRP_ID_LEN-1);
            staGetPsk->grpId[WFA_P2P_GRP_ID_LEN-1]='\0';
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_GET_PSK_TLV, sizeof(caStaGetPsk_t), (BYTE *)staGetPsk, aBuf);

    *aLen = 4+sizeof(caStaGetPsk_t);

    return TRUE;
}

int xcCmdProcStaP2pStartAutoGo(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaStartAutoGo_t *staP2pStartAutoGo= (caStaStartAutoGo_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staP2pStartAutoGo->intf, str,WFA_IF_NAME_LEN-1);
            staP2pStartAutoGo->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "oper_chn") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staP2pStartAutoGo->oper_chn= atoi(str);
        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staP2pStartAutoGo->ssid, str, WFA_SSID_NAME_LEN-1);
            staP2pStartAutoGo->ssid[WFA_SSID_NAME_LEN-1]='\0';
            staP2pStartAutoGo->ssid_flag =1;
        }
        else if(strcasecmp(str, "RTSP") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staP2pStartAutoGo->rtsp_flag= 1;
            staP2pStartAutoGo->rtsp= atoi(str);
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_START_AUTO_GO_TLV, sizeof(caStaStartAutoGo_t), (BYTE *)staP2pStartAutoGo, aBuf);

    *aLen = 4+sizeof(caStaStartAutoGo_t);

    return TRUE;
}


int xcCmdProcStaP2pReset(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    dutCommand_t *staP2pReset= (dutCommand_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staP2pReset->intf, str, WFA_IF_NAME_LEN-1);
            staP2pReset->intf[WFA_IF_NAME_LEN-1]='\0';
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_RESET_TLV, sizeof(dutCommand_t), (BYTE *)staP2pReset, aBuf);

    *aLen = 4+sizeof(dutCommand_t);

    return TRUE;
}



int xcCmdProcStaGetP2pIpConfig(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaGetP2pIpConfig_t *staGetP2pIpConfig = (caStaGetP2pIpConfig_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staGetP2pIpConfig->intf, str,WFA_IF_NAME_LEN-1);
            staGetP2pIpConfig->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "groupid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staGetP2pIpConfig->grpId, str, WFA_P2P_GRP_ID_LEN-1);
            staGetP2pIpConfig->grpId[WFA_P2P_GRP_ID_LEN-1]='\0';
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_GET_IP_CONFIG_TLV, sizeof(caStaGetP2pIpConfig_t), (BYTE *)staGetP2pIpConfig, aBuf);

    *aLen = 4+sizeof(caStaGetP2pIpConfig_t);

    return TRUE;
}

int xcCmdProcStaSendServiceDiscoveryReq(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSendServiceDiscoveryReq_t *staSendServiceDiscoveryReq = (caStaSendServiceDiscoveryReq_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSendServiceDiscoveryReq->intf, str,WFA_IF_NAME_LEN-1);
            staSendServiceDiscoveryReq->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "p2pdevid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSendServiceDiscoveryReq->devId, str, WFA_P2P_DEVID_LEN-1);
            staSendServiceDiscoveryReq->devId[WFA_P2P_DEVID_LEN-1]='\0';
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_SEND_SERVICE_DISCOVERY_REQ_TLV, sizeof(caStaSendServiceDiscoveryReq_t), (BYTE *)staSendServiceDiscoveryReq, aBuf);

    *aLen = 4+sizeof(caStaSendServiceDiscoveryReq_t);

    return TRUE;
}

int xcCmdProcStaSendP2pPresenceReq(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSendP2pPresenceReq_t *staSendP2pPresenceReq = (caStaSendP2pPresenceReq_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSendP2pPresenceReq->intf, str,WFA_IF_NAME_LEN-1);
            staSendP2pPresenceReq->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "interval") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSendP2pPresenceReq->interval= atoll(str);
        }
        else if(strcasecmp(str, "duration") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSendP2pPresenceReq->duration= atoll(str);
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_SEND_PRESENCE_REQ_TLV, sizeof(caStaSendP2pPresenceReq_t), (BYTE *)staSendP2pPresenceReq, aBuf);

    *aLen = 4+sizeof(caStaSendP2pPresenceReq_t);

    return TRUE;
}


int xcCmdProcStaSetSleepReq(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetSleep_t *staSetSleep = (caStaSetSleep_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSetSleep->intf, str,WFA_IF_NAME_LEN-1);
            staSetSleep->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "groupid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSetSleep->grpId, str, WFA_P2P_GRP_ID_LEN-1);
            staSetSleep->grpId[WFA_P2P_GRP_ID_LEN-1]='\0';
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_SET_SLEEP_TLV, sizeof(caStaSetSleep_t), (BYTE *)staSetSleep, aBuf);

    *aLen = 4+sizeof(caStaSetSleep_t);

    return TRUE;
}


int xcCmdProcStaSetOpportunistcPsReq(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetOpprPs_t *staSetOpprPs = (caStaSetOpprPs_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSetOpprPs->intf, str,WFA_IF_NAME_LEN-1);
            staSetOpprPs->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "ctwindow") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetOpprPs->ctwindow= atoi(str);
        }
        else if(strcasecmp(str, "groupid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSetOpprPs->grpId, str, WFA_P2P_GRP_ID_LEN-1);
            staSetOpprPs->grpId[WFA_P2P_GRP_ID_LEN-1]='\0';
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_SET_OPPORTUNISTIC_PS_TLV, sizeof(caStaSetOpprPs_t), (BYTE *)staSetOpprPs, aBuf);

    *aLen = 4+sizeof(caStaSetOpprPs_t);

    return TRUE;
}
int xcCmdProcStaAddARPTableEntry(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaAddARPTableEntry_t *staAddARPTableEntry = (caStaAddARPTableEntry_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staAddARPTableEntry->intf, str,WFA_IF_NAME_LEN-1);
            staAddARPTableEntry->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "macaddress") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(staAddARPTableEntry->macaddress, str);
        }
        else if(strcasecmp(str, "ipaddress") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(staAddARPTableEntry->ipaddress, str);
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_ADD_ARP_TABLE_ENTRY_TLV, sizeof(caStaAddARPTableEntry_t), (BYTE *)staAddARPTableEntry, aBuf);

    *aLen = 4+sizeof(caStaAddARPTableEntry_t);

    return TRUE;
}
int xcCmdProcStaBlockICMPResponse(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaBlockICMPResponse_t *staBlockICMPResponse = (caStaBlockICMPResponse_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staBlockICMPResponse->intf, str,WFA_IF_NAME_LEN-1);
            staBlockICMPResponse->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "groupid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(staBlockICMPResponse->grpId, str);
        }
        else if(strcasecmp(str, "ipaddress") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(staBlockICMPResponse->ipaddress, str);
        }
    }

    wfaEncodeTLV(WFA_STA_P2P_BLOCK_ICMP_RESPONSE_TLV, sizeof(caStaBlockICMPResponse_t), (BYTE *)staBlockICMPResponse, aBuf);

    *aLen = 4+sizeof(caStaBlockICMPResponse_t);

    return TRUE;
}

int xcCmdProcStaSetPwrSave(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetPwrSave_t *setps = (caStaSetPwrSave_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setps->intf, str, 15);
        }
        else if(strcasecmp(str, "mode") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setps->mode, str, 64);
        }
    }

    wfaEncodeTLV(WFA_STA_SET_PWRSAVE_TLV, sizeof(caStaSetPwrSave_t), (BYTE *)setps, aBuf);
    *aLen = 4+sizeof(caStaSetPwrSave_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaSetWMM(char *pcmdStr, BYTE *aBuf, int *aLen)
{
#ifdef WFA_WMM_AC
    caStaSetWMM_t *setwmm = (caStaSetWMM_t *) (aBuf+sizeof(wfaTLV));
    char *str;
    wfaTLV *hdr = (wfaTLV *)aBuf;

    DPRINT_INFO(WFA_OUT, "start xcCmdProcStaSetWMM ...\n");
    DPRINT_INFO(WFA_OUT, "params:  %s\n", pcmdStr);
    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    /* Some default values, in case they are not specified*/
    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setwmm->intf, str, 15);
        }
        else if(strcasecmp(str, "GROUP") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str,"WMMAC") == 0)
                setwmm->group = GROUP_WMMAC;
            else if(strcasecmp(str,"WMM-CONFIG") == 0)
            {
                setwmm->group = GROUP_WMMCONF;
                setwmm->actions.config.frag_thr = 2346;
                setwmm->actions.config.rts_thr = 2346;
                setwmm->actions.config.wmm = 1;
            }
        }
        else if(strcasecmp(str, "ACTION") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str,"addts") == 0)
            {
                //Put default values for the tspec element
                setwmm->action = WMMAC_ADDTS;
                setwmm->actions.addts.accesscat = WMMAC_AC_BE;
                setwmm->actions.addts.tspec.tsinfo.dummy1 = 1;
                setwmm->actions.addts.tspec.tsinfo.dummy2 = 0;
            }
            else if(strcasecmp(str,"delts") == 0)
                setwmm->action = WMMAC_DELTS;
            DPRINT_INFO(WFA_OUT,"action is %d\n",setwmm->action);
        }
        else if(strcasecmp(str, "RTS_thr") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.config.rts_thr = atoi(str);
        }
        else if(strcasecmp(str, "wmm") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(atoi(str) != 0)
                setwmm->actions.config.wmm = 1;
            else
                setwmm->actions.config.wmm = 0;
        }
        else if(strcasecmp(str, "Frag_thr") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.config.frag_thr = atoi(str);
        }
        else if(strcasecmp(str, "DIALOG_TOKEN") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.dialog_token = atoi(str);
        }
        else if(strcasecmp(str, "TID") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(setwmm->action == WMMAC_ADDTS)
                setwmm->actions.addts.tspec.tsinfo.TID  = atoi(str);
            else
                setwmm->actions.delts = atoi(str);
        }
        else if(strcasecmp(str, "SENDTRIG") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str,"true") == 0)
                setwmm->send_trig=1;
            else
                setwmm->send_trig=0;
        }
        else if(strcasecmp(str, "DEST") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setwmm->dipaddr, str, 15);
        }
        else if(strcasecmp(str, "trigac") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str,"VO") == 0)
                setwmm->trig_ac= WMMAC_AC_VO;
            else if(strcasecmp(str,"VI") == 0)
                setwmm->trig_ac= WMMAC_AC_VI;
            else if(strcasecmp(str,"BE") == 0)
                setwmm->trig_ac= WMMAC_AC_BE;
            else if(strcasecmp(str,"BK") == 0)
                setwmm->trig_ac= WMMAC_AC_BK;
        }
        else if(strcasecmp(str, "DIRECTION") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str,"UP") == 0)
                setwmm->actions.addts.tspec.tsinfo.direction = WMMAC_UPLINK;
            else if(strcasecmp(str,"DOWN") == 0)
                setwmm->actions.addts.tspec.tsinfo.direction = WMMAC_DOWNLINK;
            else if(strcasecmp(str,"BIDI") == 0)
                setwmm->actions.addts.tspec.tsinfo.direction = WMMAC_BIDIR;
        }
        else if(strcasecmp(str, "PSB") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str,"UAPSD") == 0)
                setwmm->actions.addts.tspec.tsinfo.PSB = 1;
            else
                setwmm->actions.addts.tspec.tsinfo.PSB = 0;
        }
        else if(strcasecmp(str, "UP") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.tsinfo.UP = atoi(str);
        }
        else if(strcasecmp(str, "Fixed") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "true") == 0)
                setwmm->actions.addts.tspec.Fixed = 1;
            else
                setwmm->actions.addts.tspec.Fixed = 0;
        }
        else if(strcasecmp(str, "SIZE") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.size = atoi(str);
        }
        else if(strcasecmp(str, "MAXSIZE") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.maxsize = atoi(str);
        }
        else if(strcasecmp(str, "MIN_SRVC_INTRVL") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.min_srvc = atoi(str);
        }
        else if(strcasecmp(str, "MAX_SRVC_INTRVL") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.max_srvc = atoi(str);
        }
        else if(strcasecmp(str, "INACTIVITY") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.inactivity = atoi(str);
        }
        else if(strcasecmp(str, "SUSPENSION") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.suspension = atoi(str);
        }
        else if(strcasecmp(str, "SRVCSTARTTIME") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.srvc_strt_tim = atoi(str);
        }
        else if(strcasecmp(str, "MINDATARATE") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.mindatarate = atoi(str);
        }
        else if(strcasecmp(str, "MEANDATARATE") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.meandatarate = atoi(str);
        }
        else if(strcasecmp(str, "PEAKDATARATE") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.peakdatarate = atoi(str);
        }
        else if(strcasecmp(str, "BURSTSIZE") == 0
                || strcasecmp(str, "MSDUAGGR") == 0)
        {
            // which is used is depending on BurstSizeDef
            // additional checking will be needed.
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.burstsize = atoi(str);
        }
        else if(strcasecmp(str, "DELAYBOUND") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.delaybound = atoi(str);
        }
        else if(strcasecmp(str, "PHYRATE") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.PHYrate = atoi(str);
        }
        else if(strcasecmp(str, "SBA") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.sba = atof(str);
        }
        else if(strcasecmp(str, "MEDIUM_TIME") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            setwmm->actions.addts.tspec.medium_time = atoi(str);
        }
        else if(strcasecmp(str, "ACCESSCAT") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str,"VO") == 0)
                setwmm->actions.addts.accesscat = WMMAC_AC_VO;
            else if(strcasecmp(str,"VI") == 0)
                setwmm->actions.addts.accesscat = WMMAC_AC_VI;
            else if(strcasecmp(str,"BE") == 0)
                setwmm->actions.addts.accesscat = WMMAC_AC_BE;
            else if(strcasecmp(str,"BK") == 0)
                setwmm->actions.addts.accesscat = WMMAC_AC_BK;
        }
        else if(strcasecmp(str, "infoAck") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str,"HT") == 0)
            {
                setwmm->actions.addts.tspec.tsinfo.infoAck = 1;
            }
            else // normal
            {
                setwmm->actions.addts.tspec.tsinfo.infoAck = 0;
            }
        }
        else if(strcasecmp(str, "BurstSizeDef") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str,"SET") == 0)
            {
                setwmm->actions.addts.tspec.tsinfo.bstSzDef = 1;
            }
            else // CLEAR
            {
                setwmm->actions.addts.tspec.tsinfo.bstSzDef = 0;
            }
        }
    }
    if(setwmm->action == WMMAC_ADDTS)
        printf("ADDTS AC PARAMS: dialog id: %d, TID: %d, DIRECTION: %d, PSB: %d, UP: %d, INFOACK: %d BURST SIZE DEFN: %d\
 Fixed %d, MSDU Size: %d, Max MSDU Size %d, MIN SERVICE INTERVAL: %d, MAX SERVICE INTERVAL: %d\
        ,INACTIVITY: %d,SUSPENSION %d,SERVICE START TIME: %d,MIN DATARATE: %d,MEAN DATA RATE: %d\
        , PEAK DATA RATE: %d,BURSTSIZE or MSDU Aggreg: %d,DELAY BOUND: %d,PHYRATE: %d, SPLUSBW: %f,MEDIUM TIME: %d, ACCESSCAT: %d\n"\
               ,setwmm->actions.addts.dialog_token,setwmm->actions.addts.tspec.tsinfo.TID\
               ,setwmm->actions.addts.tspec.tsinfo.direction,setwmm->actions.addts.tspec.tsinfo.PSB,setwmm->actions.addts.tspec.tsinfo.UP\
               ,setwmm->actions.addts.tspec.tsinfo.infoAck,setwmm->actions.addts.tspec.tsinfo.bstSzDef\
               ,setwmm->actions.addts.tspec.Fixed,setwmm->actions.addts.tspec.size, setwmm->actions.addts.tspec.maxsize,\
               setwmm->actions.addts.tspec.min_srvc,\
               setwmm->actions.addts.tspec.max_srvc,setwmm->actions.addts.tspec.inactivity,setwmm->actions.addts.tspec.suspension,\
               setwmm->actions.addts.tspec.srvc_strt_tim,setwmm->actions.addts.tspec.mindatarate,setwmm->actions.addts.tspec.meandatarate\
               ,setwmm->actions.addts.tspec.peakdatarate,setwmm->actions.addts.tspec.burstsize,\
               setwmm->actions.addts.tspec.delaybound,setwmm->actions.addts.tspec.PHYrate,setwmm->actions.addts.tspec.sba,\
               setwmm->actions.addts.tspec.medium_time,setwmm->actions.addts.accesscat);
    else
        printf("DELTS AC PARAMS: TID:  %d\n", setwmm->actions.delts);

    hdr->tag =  WFA_STA_SET_WMM_TLV;
    hdr->len = sizeof(caStaSetWMM_t);

    memcpy(aBuf+4, setwmm, sizeof(caStaSetWMM_t));

    *aLen = 4+sizeof(caStaSetWMM_t);
#endif
    return WFA_SUCCESS;
}

int xcCmdProcStaSetEapFAST(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetEapFAST_t *setsec = (caStaSetEapFAST_t *) (aBuf+sizeof(wfaTLV));
#ifndef WFA_PC_CONSOLE
    caStaSetEapFAST_t defparams = {"", "", "", "", "", "", "", "", 0, ""};
    char *str;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memcpy((void *)setsec, (void *)&defparams, sizeof(caStaSetEapFAST_t));

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->intf, str, 15);
        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->ssid, str, 64);
        }
        else if(strcasecmp(str, "username") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->username, str);
        }
        else if(strcasecmp(str, "password") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->passwd, str);
        }
        else if(strcasecmp(str, "keyMgmtType") == 0)
        {
            str=strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->keyMgmtType, str, 7);
        }
        else if(strcasecmp(str, "encpType") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->encrptype, str, 8);
        }
        else if(strcasecmp(str, "trustedRootCA") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->trustedRootCA, str,31);
        }
        else if(strcasecmp(str, "innerEAP") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->innerEAP, str);
        }
        else if(strcasecmp(str, "validateServer") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "yes") == 0)
            {
                setsec->validateServer=1;
            }
            else if(strcasecmp(str, "no") == 0)
            {
                setsec->validateServer=0;
            }
        }
        else if(strcasecmp(str, "pacFile") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->pacFileName, str);
        }
        else if(strcasecmp(str, "pmf") == 0)
        {
            str = strtok_r (NULL, ",", &pcmdStr);

            if(strcasecmp(str, "enable") == 0
                    || strcasecmp(str, "optional") == 0)
                setsec->pmf = WFA_ENABLED;
            else if(strcasecmp(str, "required") == 0)
                setsec->pmf = WFA_REQUIRED;
            else
                setsec->pmf = WFA_DISABLED;
        }
    }

#endif
    wfaEncodeTLV(WFA_STA_SET_EAPFAST_TLV, sizeof(caStaSetEapFAST_t), (BYTE *)setsec, aBuf);

    *aLen = 4+sizeof(caStaSetEapFAST_t);

    return WFA_SUCCESS;
}


int xcCmdProcStaSetEapAKA(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetEapAKA_t *setsec = (caStaSetEapAKA_t *) (aBuf+sizeof(wfaTLV));
#ifndef WFA_PC_CONSOLE
    char *str;
    caStaSetEapAKA_t defparams = {"", "", "", "", "", "", 0, {"", "", ""}};

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memcpy((void *)setsec, (void *)&defparams, sizeof(caStaSetEapAKA_t));

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->intf, str, 15);
        }
        else if(strcasecmp(str, "ssid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->ssid, str, 64);
        }
        else if(strcasecmp(str, "username") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->username, str);
        }
        else if(strcasecmp(str, "password") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strcpy(setsec->passwd, str);
        }
        else if(strcasecmp(str, "keyMgmtType") == 0)
        {
            str=strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->keyMgmtType, str, 7);
        }
        else if(strcasecmp(str, "encpType") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(setsec->encrptype, str, 8);
        }
        else if(strcasecmp(str, "triplet1") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy((char *)setsec->tripletSet[0], str, 63);
            DPRINT_INFO(WFA_OUT, "Triplet1 : %s\n", setsec->tripletSet[0]);
            setsec->tripletCount = 1;
        }
        else if(strcasecmp(str, "triplet2") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy((char *)setsec->tripletSet[1], str, 63);
            DPRINT_INFO(WFA_OUT, "Triplet2 : %s\n", setsec->tripletSet[1]);
            setsec->tripletCount=2;
        }
        else if(strcasecmp(str, "triplet3") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy((char *)setsec->tripletSet[2], str, 63);
            DPRINT_INFO(WFA_OUT, "Triplet1 : %s\n", setsec->tripletSet[2]);
            setsec->tripletCount = 3;
        }
        else if(strcasecmp(str, "pmf") == 0)
        {
            str = strtok_r (NULL, ",", &pcmdStr);

            if(strcasecmp(str, "enable") == 0
                    || strcasecmp(str, "optional") == 0)
                setsec->pmf = WFA_ENABLED;
            else if(strcasecmp(str, "required") == 0)
                setsec->pmf = WFA_REQUIRED;
            else
                setsec->pmf = WFA_DISABLED;
        }
    }

#endif
    wfaEncodeTLV(WFA_STA_SET_EAPAKA_TLV, sizeof(caStaSetEapAKA_t), (BYTE *)setsec, aBuf);

    *aLen = 4+sizeof(caStaSetEapAKA_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaSetSystime(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetSystime_t *systime = (caStaSetSystime_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "month") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            systime->month=atoi(str);
            DPRINT_INFO(WFA_OUT, "\n month %i \n", systime->month);
        }
        else if(strcasecmp(str, "date") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            systime->date=atoi(str);
            DPRINT_INFO(WFA_OUT, "\n date %i \n", systime->date);
        }
        else if(strcasecmp(str, "year") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            systime->year=atoi(str);
            DPRINT_INFO(WFA_OUT, "\n year %i \n", systime->year);
        }
        else if(strcasecmp(str, "hours") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            systime->hours=atoi(str);
            DPRINT_INFO(WFA_OUT, "\n hours %i \n", systime->hours);
        }
        else if(strcasecmp(str, "minutes") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            systime->minutes=atoi(str);
            DPRINT_INFO(WFA_OUT, "\n minutes %i \n", systime->minutes);
        }
        else if(strcasecmp(str, "seconds") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            systime->seconds=atoi(str);
            DPRINT_INFO(WFA_OUT, "\n seconds %i \n", systime->seconds);
        }
    }

    wfaEncodeTLV(WFA_STA_SET_SYSTIME_TLV, sizeof(caStaSetSystime_t), (BYTE *)systime, aBuf);

    *aLen = 4+sizeof(caStaSetSystime_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaDisconnect(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    dutCommand_t *disc = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(disc->intf, str,WFA_IF_NAME_LEN-1);
            disc->intf[WFA_IF_NAME_LEN-1]='\0';
        }
    }

    wfaEncodeTLV(WFA_STA_DISCONNECT_TLV, sizeof(dutCommand_t), (BYTE *)disc, aBuf);

    *aLen = 4+sizeof(dutCommand_t);
    return WFA_SUCCESS;

}

//#ifdef WFA_STA_TB
/* Check for enable/disable and return WFA_ENABLE/WFA_DISABLE. WFA_INVALID_BOOL if invalid */
int wfaStandardBoolParsing (char *str)
{
    int rc;

    if(strcasecmp(str, "enable") == 0)
        rc=WFA_ENABLED;
    else if(strcasecmp(str, "disable") == 0)
        rc=WFA_DISABLED;
    else
        rc=WFA_INVALID_BOOL;

    return rc;
}
//#endif

int xcCmdProcStaSendNeigReq(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    char *str;
    dutCommand_t *getInfo = (dutCommand_t *) (aBuf+sizeof(wfaTLV));

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    str = strtok_r(NULL, ",", &pcmdStr);
    if(str == NULL || str[0] == '\0')
        return FALSE;

    if(strcasecmp(str, "interface") == 0)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        strncpy(getInfo->intf, str, 15);
        DPRINT_INFO(WFA_OUT, "interface %s\n", getInfo->intf);
    }

    wfaEncodeTLV(WFA_STA_SEND_NEIGREQ_TLV, sizeof(dutCommand_t), (BYTE *)getInfo, aBuf);

    *aLen = 4 + sizeof(dutCommand_t);

    return TRUE;
}
int xcCmdProcStaDevSendFrame(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    char *str;
    dutCommand_t *cmd = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
    caStaDevSendFrame_t *sf = &cmd->cmdsu.sf;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(cmd->intf, str, 15);
            DPRINT_INFO(WFA_OUT, "interface %s\n", cmd->intf);
        }
        else if (strcasecmp(str, "program") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if (strcasecmp(str, "PMF") == 0)
            {
                pmfFrame_t *pmf = &sf->frameType.pmf;

                sf->program= PROG_TYPE_PMF;

                for(;;)
                {
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(str == NULL || str[0] == '\0')
                        break;

                    if (strcasecmp(str, "framename") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "disassoc") == 0)
                        {
                            pmf->eFrameName= PMF_TYPE_DISASSOC;
                        }
                        else if (strcasecmp(str, "saquery") == 0)
                        {
                            pmf->eFrameName = PMF_TYPE_SAQUERY;
                        }
                        else if (strcasecmp(str, "assocreq") == 0)
                        {
                            pmf->eFrameName = PMF_TYPE_ASSOCREQ;
                        }
                        else if (strcasecmp(str, "reassocreq") == 0)
                        {
                            pmf->eFrameName = PMF_TYPE_REASSOCREQ;
                        }
                        else if (strcasecmp(str, "deauth") == 0)
                        {
                            pmf->eFrameName = PMF_TYPE_DEAUTH;
                        }
                    }
                    else if (strcasecmp(str, "protected") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "correctKey") == 0)
                        {
                            pmf->eProtected= PMF_PROT_CORRECTKEY;
                        }
                        else if (strcasecmp(str, "incorrectKey") == 0)
                        {
                            pmf->eProtected = PMF_PROT_INCORRECTKEY;
                        }
                        else if (strcasecmp(str, "unprotected") == 0)
                        {
                            pmf->eProtected = PMF_PROT_UNPROTECTED;
                        }
                    }
                    else if (strcasecmp(str, "stationid") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        strncpy(pmf->staid, str, WFA_MAC_ADDR_STR_LEN-1);
                        pmf->staid[WFA_MAC_ADDR_STR_LEN-1]='\0';
                    }
                    else if (strcasecmp(str, "sender") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        strncpy(pmf->sender, str, 7);
                        pmf->sender[7]='\0';
                        pmf->sender_flag =1;
                    }
                    else if (strcasecmp(str, "bssid") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        strncpy(pmf->bssid, str, WFA_MAC_ADDR_STR_LEN-1);
                        pmf->bssid[WFA_MAC_ADDR_STR_LEN-1]='\0';
                        pmf->bssid_flag=1;
                    }
                } /* for */
            } /* if PMF */
            else if (strcasecmp(str, "TDLS") == 0)
            {
                tdlsFrame_t *tdls = &sf->frameType.tdls;

                sf->program= PROG_TYPE_TDLS;
                for(;;)
                {
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(str == NULL || str[0] == '\0')
                        break;

                    if (strcasecmp(str, "type") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "discovery") == 0)
                        {
                            tdls->eFrameName= TDLS_TYPE_DISCOVERY;
                        }
                        else if (strcasecmp(str, "setup") == 0)
                        {
                            tdls->eFrameName = TDLS_TYPE_SETUP;
                        }
                        else if (strcasecmp(str, "teardown") == 0)
                        {
                            tdls->eFrameName = TDLS_TYPE_TEARDOWN;
                        }
#if 0 /* TTG to decide whether to have this needed */
                        else if (strcasecmp(str, "channelswitch") == 0)
                        {
                            tdls->eFrameName = TDLS_TYPE_CHANNELSWITCH;
                        }
#endif
                        else if (strcasecmp(str, "psnull") == 0)
                        {
                            tdls->eFrameName = TDLS_TYPE_NULLFRAME;
                        }
                    }
                    else if (strcasecmp(str, "peer") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        strncpy(tdls->peer, str, WFA_MAC_ADDR_STR_LEN-1);
                        tdls->peer[WFA_MAC_ADDR_STR_LEN-1]='\0';
                    }
                    else if (strcasecmp(str, "timeout") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        tdls->timeout = atoi(str);
                        if(tdls->timeout <301)
                            return WFA_FAILURE;
                        tdls->timeout_flag=1;
                    }
                    else if(strcasecmp(str, "bssid") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        strncpy(tdls->bssid, str, WFA_MAC_ADDR_STR_LEN-1);
                        tdls->bssid[WFA_MAC_ADDR_STR_LEN-1]='\0';
                        tdls->bssid_flag=1;
                    }
                    else if(strcasecmp(str, "switchtime") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        tdls->switchtime = atoi(str);
                        tdls->switchtime_flag=1;
                    }
                    else if(strcasecmp(str, "channel") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        tdls->channel = atoi(str);
                        tdls->channel_flag=1;
                    }
                    else if(strcasecmp(str, "channelOffset") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        strncpy(tdls->offset, str, 4);
                        tdls->offset[3]=1;
                        tdls->offset_flag=1;
                    }
                    else if(strcasecmp(str, "status") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        tdls->status = atoi(str);
                        if(tdls->status != 0 && tdls->status != 37)
                            return WFA_FAILURE;
                        tdls->status_flag=1;
                    }
                    else if(strcasecmp(str, "reason") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        tdls->reason = atoi(str);
                        tdls->reason_flag=1;
                    }
                } /* for */
            } /* TDLS */
            else if (strcasecmp(str, "VENT") == 0)
            {
                ventFrame_t *vent = &sf->frameType.vent;

                sf->program= PROG_TYPE_VENT;
                for(;;)
                {
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(str == NULL || str[0] == '\0')
                        break;

                    if (strcasecmp(str, "framename") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "neigreq") == 0)
                        {
                            vent->type = VENT_TYPE_NEIGREQ;
                        }
                        if (strcasecmp(str, "transmgmt") == 0)
                        {
                            vent->type = VENT_TYPE_TRANSMGMT;
                            str = strtok_r(NULL, ",", &pcmdStr);
                            strncpy(vent->ssid, str, WFA_SSID_NAME_LEN);
                        }
                    }
                }
            }
            else if (strcasecmp(str, "WFD") == 0)
            {
                wfdFrame_t *wfd = &sf->frameType.wfd;

                sf->program= PROG_TYPE_WFD;
                for(;;)
                {
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(str == NULL || str[0] == '\0')
                        break;
                    if (strcasecmp(str, "framename") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "wfd_probereq") == 0)
                        {
                            wfd->eframe= WFD_FRAME_PRBREQ;
                        }
                        if (strcasecmp(str, "rtsp") == 0)
                        {
                            wfd->eframe= WFD_FRAME_RTSP;
                        }
                        if (strcasecmp(str, "WFD_ServDiscReq") == 0)
                        {
                            wfd->eframe= WFD_FRAME_SERVDISC_REQ;
                        }
                        if (strcasecmp(str, "WFD_ProbeReqTdls") == 0)
                        {
                            wfd->eframe= WFD_FRAME_PRBREQ_TDLS_REQ;
                        }
                        if (strcasecmp(str, "11v_TimingMsrReq") == 0)
                        {
                            wfd->eframe= WFD_FRAME_11V_TIMING_MSR_REQ;
                        }


                    }
                    else if(strcasecmp(str, "source") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        strncpy(wfd->sa, str, WFA_MAC_ADDR_STR_LEN-1);
                        wfd->sa[WFA_MAC_ADDR_STR_LEN-1]='\0';
                    }
                    else if(strcasecmp(str, "destination") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        strncpy(wfd->da, str, WFA_MAC_ADDR_STR_LEN-1);
                        wfd->da[WFA_MAC_ADDR_STR_LEN-1]='\0';
                    }
                    else if(strcasecmp(str, "devtype") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "source") == 0)
                        {
                            wfd->eDevType= WFD_DEV_TYPE_SOURCE;
                        }
                        if (strcasecmp(str, "p-sink") == 0)
                        {
                            wfd->eDevType= WFD_DEV_TYPE_PSINK;
                        }
                        if (strcasecmp(str, "s-sink") == 0)
                        {
                            wfd->eDevType= WFD_DEV_TYPE_SSINK;
                        }
                        wfd->devtype_flag=1;
                    }
                    else if(strcasecmp(str, "rtspmsgtype") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "pause") == 0)
                        {
                            wfd->eRtspMsgType= WFD_RTSP_PAUSE;
                        }
                        if (strcasecmp(str, "play") == 0)
                        {
                            wfd->eRtspMsgType= WFD_RTSP_PLAY;
                        }
                        if (strcasecmp(str, "teardown") == 0)
                        {
                            wfd->eRtspMsgType= WFD_RTSP_TEARDOWN;
                        }
                        if (strcasecmp(str, "trigger-pause") == 0)
                        {
                            wfd->eRtspMsgType= WFD_RTSP_TRIG_PAUSE;
                        }
                        if (strcasecmp(str, "trigger-play") == 0)
                        {
                            wfd->eRtspMsgType= WFD_RTSP_TRIG_PLAY;
                        }
                        if (strcasecmp(str, "trigger-teardown") == 0)
                        {
                            wfd->eRtspMsgType= WFD_RTSP_TRIG_TEARDOWN;
                        }
                        if (strcasecmp(str, "set_parameter") == 0)
                        {
                            wfd->eRtspMsgType= WFD_RTSP_SET_PARAMETER;
                        }
                        wfd->rtspmsg_flag=1;
                    }
                    else if(strcasecmp(str, "wfdsession") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        strncpy(wfd->wfdSessionID, str, WFA_WFD_SESSION_ID_LEN-1);
                        wfd->wfdSessionID[WFA_WFD_SESSION_ID_LEN-1]='\0';
                        wfd->wfdsessionid_flag=1;
                    }
                    else if(strcasecmp(str, "setparameter") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "capUibcKeyBoard") == 0)
                        {
                            wfd->eSetParams= WFD_CAP_UIBC_KEYBOARD;
                        }
                        if (strcasecmp(str, "CapUibcMouse") == 0)
                        {
                            wfd->eSetParams= WFD_CAP_UIBC_MOUSE;
                        }
                        if (strcasecmp(str, "capReNego") == 0)
                        {
                            wfd->eSetParams= WFD_CAP_RE_NEGO;
                        }
                        if (strcasecmp(str, "standBy") == 0)
                        {
                            wfd->eSetParams= WFD_STANDBY;
                        }
                        if (strcasecmp(str, "UibcSettingEnable") == 0)
                        {
                            wfd->eSetParams= WFD_UIBC_SETTINGS_ENABLE;
                        }
                        if (strcasecmp(str, "UibcSettingDisable") == 0)
                        {
                            wfd->eSetParams= WFD_UIBC_SETTINGS_DISABLE;
                        }
                        if (strcasecmp(str, "route_audio") == 0)
                        {
                            wfd->eSetParams= WFD_ROUTE_AUDIO;
                        }
                        if (strcasecmp(str, "3dVideoParam") == 0)
                        {
                            wfd->eSetParams= WFD_3D_VIDEOPARAM;
                        }
                        if (strcasecmp(str, "2dVideoParam") == 0)
                        {
                            wfd->eSetParams= WFD_2D_VIDEOPARAM;
                        }
                        wfd->setparm_flag=1;
                    }
                    else if(strcasecmp(str, "audioDest") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "p-sink") == 0)
                        {
                            wfd->eAudioDest= WFD_DEV_TYPE_PSINK;
                        }
                        if (strcasecmp(str, "s-sink") == 0)
                        {
                            wfd->eAudioDest= WFD_DEV_TYPE_SSINK;
                        }
                        wfd->audioDest_flag=1;
                    }
                    else if(strcasecmp(str, "bssid") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        strncpy(wfd->bssid, str, WFA_MAC_ADDR_STR_LEN-1);
                        wfd->bssid[WFA_MAC_ADDR_STR_LEN-1]='\0';
                        wfd->bssid_flag=1;
                    }
                    else if(strcasecmp(str, "MsrReqAction") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "enable") == 0)
                        {
                            wfd->eMsrAction= eEnable;
                        }
                        else
                        {
                            wfd->eMsrAction= eDisable;
                        }
                        wfd->msrReqAction_flag=1;
                    }
                    else if(strcasecmp(str, "CapReNegotiateParam") == 0)
                    {
                        int temp1;
                        char *tstr1,*tstr2;
                        wfd->capReNego_flag= 1;
                        str = strtok_r(NULL, ",", &pcmdStr);
                        printf("\n The Video format is : %s",str);
                        tstr1 = strtok_r(str, "-", &str);
                        tstr2 = strtok_r(str, "-", &str);
                        temp1 = atoi(tstr2);
                        printf("\n The Video format is : %s****%d*****",tstr1,temp1);

                        if(strcasecmp(tstr1, "cea") == 0)
                        {
                            wfd->ecapReNego = eCEA+1+temp1;
                        }
                        else if(strcasecmp(tstr1, "vesa") == 0)
                        {
                            wfd->ecapReNego = eVesa+1+temp1;
                        }
                        else
                        {
                            wfd->ecapReNego = eHH+1+temp1;
                        }
                    }
                }
            }
            else if (strcasecmp(str, "HS2-R2") == 0)
            {
                hs2Frame_t *hs2 = &sf->frameType.hs2_r2;

                sf->program= PROG_TYPE_HS2_R2;
                for(;;)
                {
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(str == NULL || str[0] == '\0')
                        break;
                    if (strcasecmp(str, "framename") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "ANQPQuery") == 0)
                        {
                            hs2->eframe= HS2_FRAME_ANQPQuery;
                        }
                        if (strcasecmp(str, "DLSRequest") == 0)
                        {
                            hs2->eframe= HS2_FRAME_DLSRequest;
                        }
                        if (strcasecmp(str, "GARPReq") == 0)
                        {
                            hs2->eframe= HS2_FRAME_GARPReq;
                        }
                        if (strcasecmp(str, "GARPRes") == 0)
                        {
                            hs2->eframe= HS2_FRAME_GARPRes;
                        }
                        if (strcasecmp(str, "NeighAdv") == 0)
                        {
                            hs2->eframe= HS2_FRAME_NeighAdv;
                        }
                        if (strcasecmp(str, "ARPProbe") == 0)
                        {
                            hs2->eframe= HS2_FRAME_ARPProbe;
                        }
                        if (strcasecmp(str, "ARPAnnounce") == 0)
                        {
                            hs2->eframe= HS2_FRAME_ARPAnnounce;
                        }
                        if (strcasecmp(str, "NeighSolicitReq") == 0)
                        {
                            hs2->eframe= HS2_FRAME_NeighSolicitReq;
                        }
                        if (strcasecmp(str, "ARPReply") == 0)
                        {
                            hs2->eframe= HS2_FRAME_ARPReply;
                        }

                    }/* if (strcasecmp(str, "framename") == 0)  */

                    else if(strcasecmp(str, "dest") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        strncpy(hs2->sDestMac, str, WFA_MAC_ADDR_STR_LEN-1);
                        hs2->sDestMac[WFA_MAC_ADDR_STR_LEN-1]='\0';
                    }
                    /* boolean or 1/0 binary values  */
                    else if(strcasecmp(str, "ANQP_CAP_LIST") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->bAnqpCapList = atoi(str);
                    }
                    else if(strcasecmp(str, "NAI_REALM_LIST") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->bNaiRealmList= atoi(str);
                    }
                    else if(strcasecmp(str, "3GPP_INFO") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->b3gppInfo= atoi(str);
                    }
                    else if(strcasecmp(str, "DOMAIN_LIST") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->bDomainList = atoi( str);
                    }
                    else if(strcasecmp(str, "HS_CAP_LIST") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->bHsCapList = atoi(str);
                    }
                    else if(strcasecmp(str, "OPER_NAME") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->bOperName= atoi(str);
                    }
                    else if(strcasecmp(str, "NAI_HOME_REALM_LIST") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->bNaiHomeRealmList= atoi(str);
                    }
                    else if(strcasecmp(str, "VENUE_NAME") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->bVenueName = atoi( str);
                    }

                    else if(strcasecmp(str, "ROAMING_CONS") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->bRoamingCons = atoi(str);
                    }
                    else if(strcasecmp(str, "ESS_DISASSOC_IMM") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->bEssDisassocImm= atoi(str);
                    }
                    else if(strcasecmp(str, "WAN_MAT") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->bWanMat= atoi(str);
                    }
                    else if(strcasecmp(str, "OP_CLASS") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->bOpClass = atoi( str);
                    }
                    else if(strcasecmp(str, "OSU_PROVIDER_LIST") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->bOsuProviderList = atoi(str);
                    }
                    else if(strcasecmp(str, "NET_AUTH_TYPE") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->bNetAuthType= atoi(str);
                    }
                    /*  integer values  */
                    else if(strcasecmp(str, "DISASSOC_TIMER") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->nDisassocTimer = atoi(str);
                    }
                    else if(strcasecmp(str, "NFrames") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        hs2->nFrames= atoi(str);
                    }
                    /*  string vars  */
                    else if(strcasecmp(str, "SESS_INFO_URL") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        memset( hs2->sSessInfoUrl, 0, WFA_URL_STRING_LEN);
                        strncpy(hs2->sSessInfoUrl, str, WFA_URL_STRING_LEN-1);
                        hs2->sSessInfoUrl[WFA_URL_STRING_LEN-1]='\0';
                    }
                    else if(strcasecmp(str, "Name ") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        memset( hs2->sDevName, 0, WFA_SSID_NAME_LEN);
                        strncpy(hs2->sDevName, str, WFA_SSID_NAME_LEN-1);
                        hs2->sDevName[WFA_SSID_NAME_LEN-1]='\0';
                    }
                    else if(strcasecmp(str, "ICON_REQUEST") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        memset( hs2->sIconRequest, 0, WFA_SSID_NAME_LEN);
                        strncpy(hs2->sIconRequest, str, WFA_SSID_NAME_LEN-1);
                        hs2->sIconRequest[WFA_SSID_NAME_LEN-1]='\0';
                    }
                    else if(strcasecmp(str, "SenderMAC") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        memset( hs2->sSenderMac, 0, WFA_MAC_ADDR_STR_LEN);
                        strncpy(hs2->sSenderMac, str, WFA_MAC_ADDR_STR_LEN-1);
                        hs2->sSenderMac[WFA_MAC_ADDR_STR_LEN-1]='\0';
                    }
                    else if(strcasecmp(str, "DestIP") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        memset( hs2->sDestIp, 0, IPV6_ADDRESS_STRING_LEN);
                        strncpy(hs2->sDestIp, str, IPV6_ADDRESS_STRING_LEN-1);
                        hs2->sDestIp[IPV6_ADDRESS_STRING_LEN-1]='\0';
                    }
                    else if(strcasecmp(str, "SenderIP") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        memset( hs2->sSenderIp, 0, IPV6_ADDRESS_STRING_LEN);
                        strncpy(hs2->sSenderIp, str, IPV6_ADDRESS_STRING_LEN-1);
                        hs2->sSenderIp[IPV6_ADDRESS_STRING_LEN-1]='\0';
                    }
                } /* for loop: hs2_r2 each prometers */
            } /* HS2-R2 */
			else if (strcasecmp(str, "LOC") == 0)
            {
                locFrame_t *loc = &sf->frameType.loc;

                sf->program= PROG_TYPE_LOC;
                for(;;)
                {
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(str == NULL || str[0] == '\0')
                        break;
                    if (strcasecmp(str, "framename") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "ANQPQUERY") == 0)
                        {
                            loc->eframe= LOC_TYPE_ANQPQUERY;
                        }
                        if (strcasecmp(str, "NeighReportReq") == 0)
                        {
                            loc->eframe= LOC_TYPE_NeighReportReq;
                        }
                        if (strcasecmp(str, "RadioMsntReq") == 0)
                        {
                            loc->eframe= LOC_TYPE_RadioMsntReq;
                        }
                    }

                    else if(strcasecmp(str, "destmac") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        strncpy(loc->sDestMac, str, WFA_MAC_ADDR_STR_LEN-1);
                        loc->sDestMac[WFA_MAC_ADDR_STR_LEN-1]='\0';
                    }
                    else if(strcasecmp(str, "askForLocCivic") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        loc->baskForLocCivic = atoi(str);
                    }
                    else if(strcasecmp(str, "askForLCI") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        loc->baskForLCI= atoi(str);
                    }
                    else if(strcasecmp(str, "address3") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        loc->baddress3= atoi(str);
                    }
                    else if(strcasecmp(str, "MsntType") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        loc->bmsntType = atoi( str);
                    }
                    else if(strcasecmp(str, "MaxAgeSubelem") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        loc->bmaxAgeSubelem = atoi(str);
                    }
                    else if(strcasecmp(str, "RandInterval") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        loc->brandInterval= atoi(str);
                    }
                    else if(strcasecmp(str, "MinAPcount") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        loc->bminApcount= atoi(str);
                    }
                    else if(strcasecmp(str, "AskForPublicIdentifierURI-FQDN") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        loc->baskForPublicIdentifierURI_FQDN = atoi( str);
                    }
                }
            } /* LOC */
        }/* program  */
    } /* for loop : each programs*/

    wfaEncodeTLV(WFA_STA_DEV_SEND_FRAME_TLV, sizeof(dutCommand_t), (BYTE *)cmd, aBuf);

    *aLen = 4 + sizeof(dutCommand_t);

    return WFA_SUCCESS;
}


int xcCmdProcStaTestBedCmd(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    dutCommand_t *info = (dutCommand_t *) (aBuf+sizeof(wfaTLV));

    DPRINT_INFO(WFA_OUT, "This is a TestBed Station Command ONLY\n");

    wfaEncodeTLV(WFA_STA_SEND_NEIGREQ_TLV, sizeof(dutCommand_t), (BYTE *)info, aBuf);

    *aLen = 4 + sizeof(dutCommand_t);

    return WFA_SUCCESS;

}

//#ifdef WFA_STA_TB
int xcCmdProcStaPresetTestParameters(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaPresetParameters_t *presetTestParams = (caStaPresetParameters_t *) (aBuf+sizeof(wfaTLV));
    char *str;
    char *tstr1,*tstr2;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memset(presetTestParams, 0, sizeof(caStaPresetParameters_t));

    for(;;)
    {
        str = strtok_r(pcmdStr, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(presetTestParams->intf, str, 15);
        }
        else if(strcasecmp(str, "mode") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            printf("modeis %s\n", str);

            if(strcasecmp(str, "11b") == 0 || strcasecmp(str, "b") == 0)
                presetTestParams->wirelessMode = eModeB;
            else if(strcasecmp(str, "11g") == 0 || strcasecmp(str, "g") == 0 || strcasecmp(str, "bg") ==0 )
                presetTestParams->wirelessMode = eModeBG;
            else if(strcasecmp(str, "11a") == 0 || strcasecmp(str, "a") == 0)
                presetTestParams->wirelessMode = eModeA;
            else if(strcasecmp(str, "11abg") == 0 || strcasecmp(str, "abg") == 0)
                presetTestParams->wirelessMode = eModeABG;
            else if(strcasecmp(str, "11na") == 0)
                presetTestParams->wirelessMode = eModeAN;
            else if(strcasecmp(str, "11ng") == 0)
                presetTestParams->wirelessMode = eModeGN;
            else if(strcasecmp(str, "11nl") == 0)
                presetTestParams->wirelessMode = eModeNL;   // n+abg
            else if(strcasecmp(str, "11ac") == 0)
                presetTestParams->wirelessMode = eModeAC;

            presetTestParams->modeFlag = 1;
            printf("\nSetting Mode as %d\n", presetTestParams->wirelessMode);
        }
        else if(strcasecmp(str, "powersave") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            printf("powersave is %s\n", str);
            if(strcasecmp(str, "on") == 0 || strcasecmp(str, "pspoll")==0)
                presetTestParams->legacyPowerSave = 1;
            else if (strcasecmp(str, "fast") == 0)
                presetTestParams->legacyPowerSave = 2;
            else if (strcasecmp(str, "psnonpoll") == 0)
                presetTestParams->legacyPowerSave = 3;
            else
                presetTestParams->legacyPowerSave = 0;

            presetTestParams->psFlag = 1;
            printf("\nSetting legacyPowerSave as %d\n", presetTestParams->legacyPowerSave);
        }
        else if(strcasecmp(str, "wmm") == 0)
        {
            presetTestParams->wmmFlag = 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            printf("wmm is %s\n", str);

            if(strcasecmp(str, "on") == 0)
                presetTestParams->wmmState = 1;
            else if(strcasecmp(str, "off") == 0)
                presetTestParams->wmmState = 0;
        }
        else if(strcasecmp(str, "noack") == 0)
        {
            /* uncomment and use it char *ackpol; */
            char *setvalues =strtok_r(NULL, ",", &pcmdStr);
            if(setvalues != NULL)
            {
                /* BE */
                /* str=strtok_r(NULL, ":", &setvalues);
                if(str != NULL)
                {
                    if(strcasecmp(str, "enable") == 0)
                       presetTestParams->noack_be = 2;
                    else if(strcasecmp(str, "disable") == 0)
                       presetTestParams->noack_be = 1;
                 }*/
                /* BK */
                /* str=strtok_r(NULL, ":", &setvalues);
                   if(str != NULL)
                   {
                      if(strcasecmp(str, "enable") == 0)
                         presetTestParams->noack_bk = 2;
                      else if(strcasecmp(str, "disable") == 0)
                         presetTestParams->noack_bk = 1;
                    }*/
                /* VI */
                /*str=strtok_r(NULL, ":", &setvalues);
                if(str != NULL)
                {
                    if(strcasecmp(str, "enable") == 0)
                        presetTestParams->noack_vi = 2;
                    else if(strcasecmp(str, "disable") == 0)
                        presetTestParams->noack_vi = 1;
                }*/
                /* VO */
                /*  str=strtok_r(NULL, ":", &setvalues);
                if(str != NULL)
                {
                    if(strcasecmp(str, "enable") == 0)
                       presetTestParams->noack_vo = 2;
                    else if(strcasecmp(str, "disable") == 0)
                       presetTestParams->noack_vo = 1;
                }*/
            }
        }
        else if(strcasecmp(str, "ht") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "on") == 0)
            {
                presetTestParams->ht = 1;
            }
            else
            {
                presetTestParams->ht = 0;
            }
        }
        else if(strcasecmp(str, "reset") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "11n") == 0)
            {
                presetTestParams->reset = eResetProg11n;
                printf("reset to %s\n", str);
            }
        }
        else if(strcasecmp(str, "ft_oa") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->ftoa = eEnable;
                printf("ft_oa enabled\n");
            }
            else
            {
                presetTestParams->ftoa = eDisable;
            }
        }
        else if(strcasecmp(str, "ft_ds") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->ftds = eEnable;
                printf("ft_ds enabled\n");
            }
            else
            {
                presetTestParams->ftds = eDisable;
            }
        }
        else if(strcasecmp(str, "active_scan") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->activescan = eEnable;
                printf("active scan enabled\n");
            }
            else
            {
                presetTestParams->activescan = eDisable;
            }
        }
#if 0
        else if(strcasecmp(str, "ignoreChswitchProhibit") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enabled") == 0)
            {
                presetTestParams->ignChSwitchProh = eEnable;
            }
            else
            {
                presetTestParams->ignChSwitchProh = eDisable;
            }
        }
#endif
        else if(strcasecmp(str, "tdls") == 0)
        {
            presetTestParams->tdlsFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enabled") == 0)
            {
                presetTestParams->tdls = eEnable;
            }
            else
            {
                presetTestParams->tdls = eDisable;
            }
        }
        else if(strcasecmp(str, "tdlsmode") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Default") == 0)
            {
                presetTestParams->tdlsMode = eDef;
            }
            else if(strcasecmp(str, "HiLoMac") == 0)
            {
                presetTestParams->tdlsMode = eHiLoMac;
            }
            else if(strcasecmp(str, "ExistLink") == 0)
            {
                presetTestParams->tdlsMode = eExistLink;
            }
            else if(strcasecmp(str, "APProhibit") == 0)
            {
                presetTestParams->tdlsMode = eAPProhibit;
            }
            else if(strcasecmp(str, "WeakSecurity") == 0)
            {
                presetTestParams->tdlsMode = eWeakSec;
            }
            else if(strcasecmp(str, "IgnoreChswitchProhibit") == 0)
            {
                presetTestParams->tdlsMode = eIgnChnlSWProh;
            }
        }
        else if(strcasecmp(str, "wfddevtype") == 0)
        {
            presetTestParams->wfdDevTypeFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "source") == 0)
            {
                presetTestParams->wfdDevType= eSource;
            }
            else if(strcasecmp(str, "p-sink") == 0)
            {
                presetTestParams->wfdDevType= ePSink;
            }
            else if(strcasecmp(str, "s-sink") == 0)
            {
                presetTestParams->wfdDevType= eSSink;
            }
            else if(strcasecmp(str, "dual") == 0)
            {
                presetTestParams->wfdDevType= eDual;
            }
        }
        else if(strcasecmp(str, "uibc_gen") == 0)
        {
            presetTestParams->wfdUibcGenFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdUibcGen= eEnable;
            }
            else
            {
                presetTestParams->wfdUibcGen= eDisable;
            }
        }
        else if(strcasecmp(str, "uibc_hid") == 0)
        {
            presetTestParams->wfdUibcHidFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdUibcHid= eEnable;
            }
            else
            {
                presetTestParams->wfdUibcHid= eDisable;
            }
        }
        else if(strcasecmp(str, "ui_input") == 0)
        {
            char *uilist;
            presetTestParams->wfdUiInputFlag= 1;

            uilist = strtok_r(NULL, ",", &pcmdStr);
            presetTestParams->wfdUiInputs=0;
            for(;;)
            {
                str = strtok_r(uilist, " ", &uilist);
                if(str == NULL || str[0] == '\0')
                    break;
			   printf("\n The UI input is : %s",str);

                if(strcasecmp(str, "keyboard") == 0)
                {
                    presetTestParams->wfdUiInput[presetTestParams->wfdUiInputs]= eKeyBoard;
                }
                else if(strcasecmp(str, "mouse") == 0)
                {
                    presetTestParams->wfdUiInput[presetTestParams->wfdUiInputs]= eMouse;
                }
                presetTestParams->wfdUiInputs++;
            }
        }
        else if(strcasecmp(str, "hdcp") == 0)
        {
            presetTestParams->wfdHdcpFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdHdcp= eEnable;
            }
            else
            {
                presetTestParams->wfdHdcp= eDisable;
            }
        }
        else if(strcasecmp(str, "frameskip") == 0)
        {
            presetTestParams->wfdFrameSkipFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdFrameSkip= eEnable;
            }
            else
            {
                presetTestParams->wfdFrameSkip= eDisable;
            }
        }
        else if(strcasecmp(str, "avchange") == 0)
        {
            presetTestParams->wfdAvChangeFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdAvChange= eEnable;
            }
            else
            {
                presetTestParams->wfdAvChange= eDisable;
            }
        }
        else if(strcasecmp(str, "standby") == 0)
        {
            presetTestParams->wfdStandByFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdStandBy= eEnable;
            }
            else
            {
                presetTestParams->wfdStandBy= eDisable;
            }
        }
        else if(strcasecmp(str, "inputcontent") == 0)
        {
            presetTestParams->wfdInVideoFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Protected") == 0)
            {
                presetTestParams->wfdInVideo= eProtected;
            }
            else if(strcasecmp(str, "Unprotected") == 0)
            {
                presetTestParams->wfdInVideo= eUnprotected;
            }
            else if(strcasecmp(str, "ProtectedVideoOnly") == 0)
            {
                presetTestParams->wfdInVideo= eProtectedVideoOnly;
            }
        }
        else if(strcasecmp(str, "videoformat") == 0)
        {
            int temp1;
            char *videolist;
            presetTestParams->wfdVideoFmatFlag= 1;

            videolist = strtok_r(NULL, ",", &pcmdStr);
            presetTestParams->wfdInputVideoFmats=0;

            for(;;)
            {
                str = strtok_r(videolist, " ", &videolist);
                if(str == NULL || str[0] == '\0')
                    break;
			   printf("\n The Video format is : %s",str);

                tstr1 = strtok_r(str, "-", &str);
                tstr2 = strtok_r(str, "-", &str);

                temp1 = atoi(tstr2);
                printf("\n The Video format is : %s****%d*****",tstr1,temp1);


                if(strcasecmp(tstr1, "cea") == 0)
                {
                    presetTestParams->wfdVideoFmt[presetTestParams->wfdInputVideoFmats]= eCEA+1+temp1;
                }
                else if(strcasecmp(tstr1, "vesa") == 0)
                {
                    presetTestParams->wfdVideoFmt[presetTestParams->wfdInputVideoFmats]=  eVesa+1+temp1;
                }
                else
                {
                    presetTestParams->wfdVideoFmt[presetTestParams->wfdInputVideoFmats]=  eHH+1+temp1;
                }
                presetTestParams->wfdInputVideoFmats++;
            }
        }
        else if(strcasecmp(str, "AudioFormat") == 0)
        {
            presetTestParams->wfdAudioFmatFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Mandatory") == 0)
            {
                presetTestParams->wfdAudioFmt= eMandatoryAudioMode;
            }
            else
            {
                presetTestParams->wfdAudioFmt= eDefaultAudioMode;
            }
        }

        else if(strcasecmp(str, "i2c") == 0)
        {
            presetTestParams->wfdI2cFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdI2c= eEnable;
            }
            else
            {
                presetTestParams->wfdI2c= eDisable;
            }
        }
        else if(strcasecmp(str, "videorecovery") == 0)
        {
            presetTestParams->wfdVideoRecoveryFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdVideoRecovery= eEnable;
            }
            else
            {
                presetTestParams->wfdVideoRecovery= eDisable;
            }
        }
        else if(strcasecmp(str, "PrefDisplay") == 0)
        {
            presetTestParams->wfdPrefDisplayFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdPrefDisplay= eEnable;
            }
            else
            {
                presetTestParams->wfdPrefDisplay= eDisable;
            }
        }
        else if(strcasecmp(str, "ServiceDiscovery") == 0)
        {
            presetTestParams->wfdServiceDiscoveryFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdServiceDiscovery= eEnable;
            }
            else
            {
                presetTestParams->wfdServiceDiscovery= eDisable;
            }
        }
        else if(strcasecmp(str, "3dVideo") == 0)
        {
            presetTestParams->wfd3dVideoFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfd3dVideo= eEnable;
            }
            else
            {
                presetTestParams->wfd3dVideo= eDisable;
            }
        }
        else if(strcasecmp(str, "MultiTxStream") == 0)
        {
            presetTestParams->wfdMultiTxStreamFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdMultiTxStream= eEnable;
            }
            else
            {
                presetTestParams->wfdMultiTxStream= eDisable;
            }
        }
        else if(strcasecmp(str, "TimeSync") == 0)
        {
            presetTestParams->wfdTimeSyncFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdTimeSync= eEnable;
            }
            else
            {
                presetTestParams->wfdTimeSync= eDisable;
            }
        }
        else if(strcasecmp(str, "EDID") == 0)
        {
            presetTestParams->wfdEDIDFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdEDID= eEnable;
            }
            else
            {
                presetTestParams->wfdEDID= eDisable;
            }
        }
        else if(strcasecmp(str, "UIBC_Prepare") == 0)
        {
            presetTestParams->wfdUIBCPrepareFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdUIBCPrepare= eEnable;
            }
            else
            {
                presetTestParams->wfdUIBCPrepare= eDisable;
            }
        }
        else if(strcasecmp(str, "OptionalFeature") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "DisableAll") == 0)
            {
                presetTestParams->wfdOptionalFeatureFlag= eEnable;
            }
            else
            {
                presetTestParams->wfdOptionalFeatureFlag= eDisable;
            }
        }
        else if(strcasecmp(str, "SessionAvailability") == 0)
        {
            presetTestParams->wfdSessionAvailFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdSessionAvail= eEnable;
            }
            else
            {
                presetTestParams->wfdSessionAvail= eDisable;
            }
        }
        else if(strcasecmp(str, "DeviceDiscoverability") == 0)
        {
            presetTestParams->wfdDeviceDiscoverabilityFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdDeviceDiscoverability= eEnable;
            }
            else
            {
                presetTestParams->wfdDeviceDiscoverability= eDisable;
            }
        }
		else if(strcasecmp(str, "oper_chn") == 0)
        {
             str = strtok_r(NULL, ",", &pcmdStr);
             presetTestParams->oper_chn= atoi(str); 
        }
        else if (strcasecmp(str, "program") == 0)
        {
            presetTestParams->programFlag= 1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if (strcasecmp(str, "PMF") == 0)
            {
                presetTestParams->program=PROG_TYPE_PMF;
            }
			else if (strcasecmp(str, "General") == 0)
			{
				presetTestParams->program=PROG_TYPE_GEN;
			}
			else if (strcasecmp(str, "TDLS") == 0)
			{
				presetTestParams->program=PROG_TYPE_TDLS;
			}
			else if (strcasecmp(str, "VOE") == 0)
			{
				presetTestParams->program=PROG_TYPE_VENT;
			}
			else if (strcasecmp(str, "WFD") == 0)
			{
				presetTestParams->program=PROG_TYPE_WFD;
			}
			else if (strcasecmp(str, "WFDS") == 0)
			{
				presetTestParams->program=PROG_TYPE_WFDS;
			}
			
        }
        else if(strcasecmp(str, "CoupledCap") == 0)
        {
            presetTestParams->wfdCoupledCapFlag=1;
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "Enable") == 0)
            {
                presetTestParams->wfdCoupledCap= eEnable;
            }
            else
            {
                presetTestParams->wfdCoupledCap= eDisable;
            }
        }
        else if (strcasecmp(str, "supplicant") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if (strcasecmp(str, "Default") == 0 || strcasecmp(str, "WPA_Supplicant") == 0)
            {
                presetTestParams->supplicant = eWpaSupplicant;
            }
        }
		else if(strcasecmp(str, "type") == 0)
		{
		   str = strtok_r(NULL, ",", &pcmdStr);
		   if(strcasecmp(str, "AcceptPD") == 0)
		   {
			  presetTestParams->wfdsType= eAcceptPD;
		   }
		   else if (strcasecmp(str, "RejectPD") == 0)
		   {
			  presetTestParams->wfdsType= eRejectPD;
		   }
		   else if (strcasecmp(str, "IgnorePD") == 0)
		   {
			  presetTestParams->wfdsType= eIgnorePD;
		   }

		}
		else if(strcasecmp(str, "connectionCapabilityInfo") == 0)
		{
		   presetTestParams->wfdsConnectionCapabilityFlag=1;	
		   str = strtok_r(NULL, ",", &pcmdStr);
		   if(strcasecmp(str, "GO") == 0)
		   {
			  presetTestParams->wfdsConnectionCapability= eWfdsGO;
		   }
		   else if (strcasecmp(str, "CLI") == 0)
		   {
			  presetTestParams->wfdsConnectionCapability= eWfdsCLI;
		   }
		   else if (strcasecmp(str, "NewGO") == 0)
		   {
			  presetTestParams->wfdsConnectionCapability= eWfdsNewGO;
		   }
		   else if (strcasecmp(str, "New") == 0)
		   {
			  presetTestParams->wfdsConnectionCapability= eWfdsNew;
		   }
		   else if (strcasecmp(str, "CliGO") == 0)
		   {
			  presetTestParams->wfdsConnectionCapability= eWfdsCliGO;
		   }
		}
    }

    wfaEncodeTLV(WFA_STA_PRESET_PARAMETERS_TLV, sizeof(caStaPresetParameters_t), (BYTE *)presetTestParams, aBuf);

    *aLen = 4 + sizeof(caStaPresetParameters_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaResetDefault(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaResetDefault_t *reset = (caStaResetDefault_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(reset->intf, str, 15);
        }
        else if(strcasecmp(str, "prog") == 0) // VHT, 11n, VOE; HS2; HS2-R2, NAN, LOC etc
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(reset->prog, str, sizeof(reset->prog));
        }
        else if(strcasecmp(str, "type") == 0) // dut or sta
        {
           str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(reset->type, str, sizeof(reset->type));
        }
    }

    wfaEncodeTLV(WFA_STA_RESET_DEFAULT_TLV, sizeof(caStaResetDefault_t), (BYTE *)reset, aBuf);
    *aLen = 4+sizeof(caStaResetDefault_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaSetRadio(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    char *str;
    dutCommand_t *cmd = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
    caStaSetRadio_t *sr = &cmd->cmdsu.sr;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(cmd->intf, str, 15);
            DPRINT_INFO(WFA_OUT, "interface %s\n", cmd->intf);
        }
        else if (strcasecmp(str, "mode") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if (strcasecmp(str, "off") == 0)
            {
                sr->mode = WFA_OFF;
            }
            else
            {
                sr->mode = WFA_ON;
            }
        }
    }

    wfaEncodeTLV(WFA_STA_SET_RADIO_TLV, sizeof(dutCommand_t), (BYTE *)cmd, aBuf);
    *aLen = 4+sizeof(dutCommand_t);
    return WFA_SUCCESS;
}

/* If you decide to use CLI, the function is to be disabled */

int xcCmdProcStaSetWireless(char *pcmdStr, BYTE *aBuf, int *aLen)
{

    caStaSetWireless_t *staWirelessParams = (caStaSetWireless_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    DPRINT_INFO(WFA_OUT,"xcCmdProcStaSetWireless Starts...");

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staWirelessParams->intf, str, 15);
        }
        else if(strcasecmp(str, "band") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staWirelessParams->band,str,7);
            DPRINT_INFO(WFA_OUT, "\n Band -%s- \n", staWirelessParams->band);
        }
        else if(strcasecmp(str, "noack") == 0)
        {
            char *ackpol;
            int ackpolcnt = 0;
            char *setvalues =strtok_r(NULL, ",", &pcmdStr);

            if(setvalues != NULL)
            {
                while((ackpol = strtok_r(NULL, ":", &setvalues)) != NULL && ackpolcnt < 4)
                {
                    if(strcasecmp(str, "enable") == 0)
                        staWirelessParams->noAck[ackpolcnt] = 1;
                    else if(strcasecmp(str, "disable") == 0)
                        staWirelessParams->noAck[ackpolcnt] = 0;

                    ackpolcnt++;
                }
            }
        }
        else if(strcasecmp(str, "program") == 0) // VHT or 11n or Voice
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staWirelessParams->program, str, WFA_PROGNAME_LEN);

            if(strcasecmp(staWirelessParams->program, "VHT") == 0)
            {
                // process addba_reject, ampdu, amsdu, stbc_rx, width, smps, txsp_stream, rxsp_stream, band, DYN_BW_SGNL
                // SGI80, TXBF, LDPC, Opt_md_notif_ie, nss_mcs_cap, tx_lgi_rate, zero_crc, vht_tkip, vht_wep, bw_sgnl

            }
        }
    }

    wfaEncodeTLV(WFA_STA_SET_WIRELESS_TLV, sizeof(caStaSetWireless_t), (BYTE *)staWirelessParams, aBuf);
    *aLen = 4+sizeof(caStaSetWireless_t);
    return WFA_SUCCESS;
}

int xcCmdProcStaSendADDBA(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetSendADDBA_t *staSendADDBA = (caStaSetSendADDBA_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    DPRINT_INFO(WFA_OUT,"xcCmdProcStaSendADDBA Starts...");

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSendADDBA->intf, str, 15);
        }
        else if(strcasecmp(str, "tid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSendADDBA->tid = atoi(str);
            DPRINT_INFO(WFA_OUT, "\n TID -%i- \n", staSendADDBA->tid);
        }
    }

    wfaEncodeTLV(WFA_STA_SEND_ADDBA_TLV, sizeof(caStaSetSendADDBA_t), (BYTE *)staSendADDBA, aBuf);
    *aLen = 4+sizeof(caStaSetSendADDBA_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaSetRIFS(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSetRIFS_t *staSetRIFS = (caStaSetRIFS_t *)(aBuf+sizeof(wfaTLV));
    char *str;

    DPRINT_INFO(WFA_OUT, "xcCmdProcSetRIFS starts ...\n");

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSetRIFS->intf, str, 15);
        }
        else if(strcasecmp(str, "action") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staSetRIFS->action = wfaStandardBoolParsing(str);
            DPRINT_INFO(WFA_OUT, "\n TID -%i- \n", staSetRIFS->action);
        }
    }

    wfaEncodeTLV(WFA_STA_SET_RIFS_TEST_TLV, sizeof(caStaSetRIFS_t), (BYTE *)staSetRIFS, aBuf);
    *aLen = 4+sizeof(caStaSetRIFS_t);

    return WFA_SUCCESS;

}


int xcCmdProcStaSendCoExistMGMT(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaSendCoExistMGMT_t *staSendMGMT = (caStaSendCoExistMGMT_t *)(aBuf+sizeof(wfaTLV));
    char *str;

    DPRINT_INFO(WFA_OUT, "xcCmdProcSendCoExistMGMT starts ...\n");

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSendMGMT->intf, str, 15);
        }
        else if(strcasecmp(str, "type") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSendMGMT->type, str, 15);
        }
        else if(strcasecmp(str, "value") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staSendMGMT->value, str, 15);
        }
    }

    wfaEncodeTLV(WFA_STA_SEND_COEXIST_MGMT_TLV, sizeof(caStaSendCoExistMGMT_t), (BYTE *)staSendMGMT, aBuf);
    *aLen = 4+sizeof(caStaSendCoExistMGMT_t);

    return WFA_SUCCESS;
}


int xcCmdProcStaSet11n(char *pcmdStr, BYTE *aBuf, int *aLen)
{


    caSta11n_t *v11nParams = (caSta11n_t *) (aBuf+sizeof(wfaTLV));
    char *str;
    caSta11n_t init11nParams = {"wifi0", 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,0xFFFF, 0xFFFF, "", "", 0xFF, 0xFF, 0xFF, 0xFF};

    DPRINT_INFO(WFA_OUT,"xcCmdProcStaSet11n Starts...");

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);
    memcpy(v11nParams, &init11nParams, sizeof(caSta11n_t));

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;
        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(v11nParams->intf, str,WFA_IF_NAME_LEN-1);
            v11nParams->intf[WFA_IF_NAME_LEN-1]='\0';
        }

        if(strcasecmp(str, "ampdu") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            v11nParams->ampdu=wfaStandardBoolParsing(str);
            if (v11nParams->ampdu<0)
            {
                DPRINT_INFO(WFA_OUT, "Invalid AMPDU Value %s\n",str);
                return WFA_FAILURE;
            }
            DPRINT_INFO(WFA_OUT, "\n AMPDU -%i- \n", v11nParams->ampdu);
        }
        else if(strcasecmp(str, "40_intolerant") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            v11nParams->_40_intolerant=wfaStandardBoolParsing(str);
            if (v11nParams->_40_intolerant<0)
            {
                DPRINT_INFO(WFA_OUT, "Invalid _40_intolerant Value %s\n",str);
                return WFA_FAILURE;
            }
            DPRINT_INFO(WFA_OUT, "\n _40_intolerant -%i- \n", v11nParams->_40_intolerant);
        }
        else if(strcasecmp(str, "sgi20") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            v11nParams->sgi20=wfaStandardBoolParsing(str);
            if (v11nParams->sgi20<0)
            {
                DPRINT_INFO(WFA_OUT, "Invalid sgi20 Value %s\n",str);
                return WFA_FAILURE;
            }
            DPRINT_INFO(WFA_OUT, "\n sgi20 -%i- \n", v11nParams->sgi20);
        }
        else if(strcasecmp(str, "amsdu") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            v11nParams->amsdu=wfaStandardBoolParsing(str);
            if (v11nParams->amsdu<0)
            {
                DPRINT_INFO(WFA_OUT, "Invalid amsdu Value %s\n",str);
                return WFA_FAILURE;
            }
            DPRINT_INFO(WFA_OUT, "\n amsdu -%i- \n", v11nParams->amsdu);
        }
        else if(strcasecmp(str, "addba_reject") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            v11nParams->addba_reject=wfaStandardBoolParsing(str);
            if (v11nParams->addba_reject<0)
            {
                DPRINT_INFO(WFA_OUT, "Invalid addba_reject Value %s\n",str);
                return WFA_FAILURE;
            }
            DPRINT_INFO(WFA_OUT, "\n addba_reject -%i- \n", v11nParams->addba_reject);
        }
        else if(strcasecmp(str, "greenfield") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            v11nParams->greenfield=wfaStandardBoolParsing(str);
            if (v11nParams->greenfield<0)
            {
                DPRINT_INFO(WFA_OUT, "Invalid greenfield Value %s\n",str);
                return WFA_FAILURE;
            }
            DPRINT_INFO(WFA_OUT, "\n greenfield -%i- \n", v11nParams->greenfield);
        }
        else if(strcasecmp(str, "mcs32") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            v11nParams->mcs32=wfaStandardBoolParsing(str);
            if (v11nParams->mcs32<0)
            {
                DPRINT_INFO(WFA_OUT, "Invalid mcs32 Value %s\n",str);
                return WFA_FAILURE;
            }
            DPRINT_INFO(WFA_OUT, "\n mcs32 -%i- \n", v11nParams->mcs32);
        }
        else if(strcasecmp(str, "rifs_test") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            v11nParams->rifs_test=wfaStandardBoolParsing(str);
            if (v11nParams->rifs_test<0)
            {
                DPRINT_INFO(WFA_OUT, "Invalid rifs_test Value %s\n",str);
                return WFA_FAILURE;
            }
            DPRINT_INFO(WFA_OUT, "\n rifs_test -%i- \n", v11nParams->rifs_test);
        }
        else if(strcasecmp(str, "width") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(v11nParams->width,str,7);
            DPRINT_INFO(WFA_OUT, "\n width -%s- \n", v11nParams->width);
        }
        else if(strcasecmp(str, "mcs_fixedrate") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(v11nParams->mcs_fixedrate,str,4);
            DPRINT_INFO(WFA_OUT, "\n mcs fixedrate -%s- \n", v11nParams->mcs_fixedrate);
        }
        else if(strcasecmp(str, "stbc_rx") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            v11nParams->stbc_rx = atoi(str);
            DPRINT_INFO(WFA_OUT, "\n stbc rx -%d- \n", v11nParams->stbc_rx);
        }
        else if(strcasecmp(str, "smps") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "dynamic") == 0)
            {
                v11nParams->smps = 0;
            }
            else if(strcasecmp(str, "static")==0)
            {
                v11nParams->smps = 1;
            }
            else if(strcasecmp(str, "nolimit") == 0)
            {
                v11nParams->smps = 2;
            }
            DPRINT_INFO(WFA_OUT, "\n smps  -%d- \n", v11nParams->smps);
        }
        else if(strcasecmp(str, "txsp_stream") == 0 )
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            v11nParams->txsp_stream = atoi(str);
            DPRINT_INFO(WFA_OUT, "\n txsp_stream -%d- \n", v11nParams->txsp_stream);
        }
        else if(strcasecmp(str, "rxsp_stream") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            v11nParams->rxsp_stream = atoi(str);
            DPRINT_INFO(WFA_OUT, "\n rxsp_stream -%d- \n", v11nParams->rxsp_stream);
        }
    }

    wfaEncodeTLV(WFA_STA_SET_11N_TLV, sizeof(caSta11n_t), (BYTE *)v11nParams, aBuf);
    *aLen = 4+sizeof(caSta11n_t);
    return WFA_SUCCESS;
}

//#endif


int xcCmdProcStaSetRFeature(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    dutCommand_t *dutcmd = (dutCommand_t *) (aBuf+sizeof(wfaTLV));
    caStaRFeat_t *rfeat = (caStaRFeat_t *) &dutcmd->cmdsu.rfeat;
    char *str;

    if(aBuf == NULL)
        return WFA_FAILURE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(dutcmd->intf, str, 15);
        }
        else if(strcasecmp(str, "prog") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(rfeat->prog, str, 8);
        }
        else if(strcasecmp(str, "uapsd") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "enable") == 0)
                rfeat->uapsd = eEnable;
            else
                rfeat->uapsd = eDisable;
        }
        else if(strcasecmp(str, "peer") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(rfeat->peer, str, 17);
        }
        else if(strcasecmp(str, "tpktimer") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if(strcasecmp(str, "enable") == 0)
                rfeat->tpktimer= eEnable;
            else
                rfeat->tpktimer = eDisable;
        }
        else if(strcasecmp(str, "ChSwitchMode") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(rfeat->chswitchmode, str, sizeof(rfeat->chswitchmode));
        }
        else if(strcasecmp(str, "OffChNum") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            rfeat->offchnum = atoi(str);
        }
        else if(strcasecmp(str, "SecChOffset") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(rfeat->secchoffset, str, sizeof(rfeat->secchoffset));
        }
    }

    wfaEncodeTLV(WFA_STA_SET_RFEATURE_TLV, sizeof(caStaRFeat_t), (BYTE *)rfeat, aBuf);
    *aLen = 4+sizeof(caStaRFeat_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaStartWfdConnection(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaStartWfdConn_t *staStartWfdConn = (caStaStartWfdConn_t *) (aBuf+sizeof(wfaTLV));
    char *str;
    BYTE tmp_cnt;
    char *tmp_str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(pcmdStr, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staStartWfdConn->intf, str,WFA_IF_NAME_LEN-1);
            staStartWfdConn->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "peeraddress") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);

            for(tmp_cnt=0;; tmp_cnt++)
            {
                tmp_str = strtok_r(str, " ", &str);
                if(str == NULL || str[0] == '\0')
                    break;

                strncpy(staStartWfdConn->peer[tmp_cnt], tmp_str, WFA_MAC_ADDR_STR_LEN-1);
                staStartWfdConn->peer[tmp_cnt][WFA_MAC_ADDR_STR_LEN-1]='\0';
            }
        }
        else if(strcasecmp(str, "init_wfd") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staStartWfdConn->init_wfd= atoi(str);
            staStartWfdConn->init_wfd_flag=1;
        }
        else if(strcasecmp(str, "intent_val") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staStartWfdConn->intent_val= atoi(str);
            staStartWfdConn->intent_val_flag=1;
        }
        else if(strcasecmp(str, "oper_chn") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staStartWfdConn->oper_chn= atoi(str);
            staStartWfdConn->oper_chn_flag=1;
        }
        else if(strcasecmp(str, "coupledSession") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            staStartWfdConn->coupledSession= atoi(str);
            staStartWfdConn->coupledSession_flag=1;
        }
    }

    wfaEncodeTLV(WFA_STA_START_WFD_CONNECTION_TLV,sizeof(caStaStartWfdConn_t), (BYTE *)staStartWfdConn, aBuf);

    *aLen = 4+sizeof(caStaStartWfdConn_t);
    return WFA_SUCCESS;
}

int xcCmdProcStaCliCommand(char *pcmdStr, BYTE *aBuf, int *aLen)
{

    printf("\n The CA CLI command to DUT is : %s",pcmdStr);
    printf("\n The CA CLI command to DUT Length : %d",strlen(pcmdStr));
    wfaEncodeTLV(WFA_STA_CLI_CMD_TLV, strlen(pcmdStr), (BYTE *)pcmdStr, aBuf);

    *aLen = 4+strlen(pcmdStr);
    return TRUE;

}

int xcCmdProcStaConnectGoStartWfd(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaConnectGoStartWfd_t *staConnectGoStartWfd= (caStaConnectGoStartWfd_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staConnectGoStartWfd->intf, str,WFA_IF_NAME_LEN-1);
            staConnectGoStartWfd->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "groupid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staConnectGoStartWfd->grpid, str, WFA_P2P_GRP_ID_LEN-1);
            staConnectGoStartWfd->grpid[WFA_P2P_GRP_ID_LEN-1]='\0';
        }
        else if(strcasecmp(str, "p2pdevid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staConnectGoStartWfd->devId, str, WFA_P2P_DEVID_LEN-1);
            staConnectGoStartWfd->devId[WFA_P2P_DEVID_LEN-1]='\0';
        }
    }

    wfaEncodeTLV(WFA_STA_CONNECT_GO_START_WFD_TLV, sizeof(caStaConnectGoStartWfd_t), (BYTE *)staConnectGoStartWfd, aBuf);

    *aLen = 4+sizeof(caStaConnectGoStartWfd_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaGenerateEvent(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaGenEvent_t *staGenEvent= (caStaGenEvent_t *) (aBuf+sizeof(wfaTLV));
    char *str;
    caWfdStaGenEvent_t *pWfdEvent;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, '\0', *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staGenEvent->intf, str,WFA_IF_NAME_LEN-1);
            staGenEvent->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "program") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);

            if (strcasecmp(str, "WFD") == 0)
            {
                staGenEvent->program= PROG_TYPE_WFD;
                pWfdEvent = (caWfdStaGenEvent_t *) &staGenEvent->wfdEvent;

                for(;;)
                {
                    str = strtok_r(NULL, ",", &pcmdStr);
                    if(str == NULL || str[0] == '\0')
                        break;
                    if (strcasecmp(str, "type") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "uibc_gen") == 0)
                        {
                            pWfdEvent->type= eUibcGen;
                        }
                        if (strcasecmp(str, "uibc_hid") == 0)
                        {
                            pWfdEvent->type= eUibcHid;
                        }
                        if (strcasecmp(str, "frameskip") == 0)
                        {
                            pWfdEvent->type= eFrameSkip;
                        }
                        if (strcasecmp(str, "inputContent") == 0)
                        {
                            pWfdEvent->type= eInputContent;
                        }
                        if (strcasecmp(str, "i2cread") == 0)
                        {
                            pWfdEvent->type= eI2cRead;
                        }
                        if (strcasecmp(str, "i2cwrite") == 0)
                        {
                            pWfdEvent->type= eI2cWrite;
                        }
                        if (strcasecmp(str, "idrReq") == 0)
                        {
                            pWfdEvent->type= eIdrReq;
                        }
                    }
                    else if(strcasecmp(str, "sessionid") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        strncpy(pWfdEvent->wfdSessionID, str, WFA_WFD_SESSION_ID_LEN-1);
                        pWfdEvent->wfdSessionID[WFA_WFD_SESSION_ID_LEN-1]='\0';
                        pWfdEvent->wfdSessionIdflag=1;
                    }
                    else if(strcasecmp(str, "uibceventtype") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "KeyBoard") == 0)
                        {
                            pWfdEvent->wfdUibcEventType= eKeyBoardEvent;
                        }
                        if (strcasecmp(str, "Mouse") == 0)
                        {
                            pWfdEvent->wfdUibcEventType= eMouseEvent;
                        }
                        pWfdEvent->wfdUibcEventTypeflag=1;
                    }
                    else if(strcasecmp(str, "uibc_prepare") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "KeyBoard") == 0)
                        {
                            pWfdEvent->wfdUibcEventPrepare= eKeyBoardEvent;
                        }
                        if (strcasecmp(str, "Mouse") == 0)
                        {
                            pWfdEvent->wfdUibcEventPrepare= eMouseEvent;
                        }
                        pWfdEvent->wfdUibcEventPrepareflag=1;
                    }
                    else if(strcasecmp(str, "frameSkip") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "Start") == 0)
                        {
                            pWfdEvent->wfdFrameSkipRateflag=1;
                        }
                        else
                        {
                            pWfdEvent->wfdFrameSkipRateflag=0;
                        }
                    }
                    else if(strcasecmp(str, "InputContentType") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        if (strcasecmp(str, "Protected") == 0)
                        {
                            pWfdEvent->wfdInputContentType= eProtected;
                        }
                        if (strcasecmp(str, "Unprotected") == 0)
                        {
                            pWfdEvent->wfdInputContentType= eUnprotected;
                        }
                        if (strcasecmp(str, "ProtectedVideoOnly") == 0)
                        {
                            pWfdEvent->wfdInputContentType= eProtectedVideoOnly;
                        }
                        pWfdEvent->wfdInputContentTypeflag=1;
                    }
                    else if(strcasecmp(str, "I2c_Struct") == 0)
                    {
                        str = strtok_r(NULL, ",", &pcmdStr);
                        strncpy(pWfdEvent->wfdI2cData, str, strlen(str));
                        pWfdEvent->wfdI2cData[31]='\0';
                        pWfdEvent->wfdI2cDataflag=1;
                    }
                }
            }
        }
    }

    wfaEncodeTLV(WFA_STA_GENERATE_EVENT_TLV, sizeof(caStaGenEvent_t), (BYTE *)staGenEvent, aBuf);

    *aLen = 4+sizeof(caStaGenEvent_t);

    return WFA_SUCCESS;
}


int xcCmdProcStaReinvokeWfdSession(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaReinvokeWfdSession_t *staReinvokeWfdSession= (caStaReinvokeWfdSession_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staReinvokeWfdSession->intf, str,WFA_IF_NAME_LEN-1);
            staReinvokeWfdSession->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "groupid") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staReinvokeWfdSession->grpid, str, WFA_P2P_GRP_ID_LEN-1);
            staReinvokeWfdSession->grpid[WFA_P2P_GRP_ID_LEN-1]='\0';
            staReinvokeWfdSession->grpid_flag=1;
        }
        else if(strcasecmp(str, "PeerAddress") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staReinvokeWfdSession->peer, str, WFA_MAC_ADDR_STR_LEN-1);
            staReinvokeWfdSession->peer[WFA_MAC_ADDR_STR_LEN-1]='\0';
        }
        else if(strcasecmp(str, "invitationaction") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            if (strcasecmp(str, "Send") == 0)
            {
                staReinvokeWfdSession->wfdInvitationAction= eInvitationSend;
            }
            else if (strcasecmp(str, "Accept") == 0)
            {
                staReinvokeWfdSession->wfdInvitationAction= eInvitationAccept;
            }
        }
    }

    wfaEncodeTLV(WFA_STA_REINVOKE_WFD_SESSION_TLV, sizeof(caStaReinvokeWfdSession_t), (BYTE *)staReinvokeWfdSession, aBuf);
    *aLen = 4+sizeof(caStaReinvokeWfdSession_t);

    return WFA_SUCCESS;
}

int xcCmdProcStaGetParameter(char *pcmdStr, BYTE *aBuf, int *aLen)
{
    caStaGetParameter_t *staGetParameter = (caStaGetParameter_t *) (aBuf+sizeof(wfaTLV));
    char *str;

    if(aBuf == NULL)
        return FALSE;

    memset(aBuf, 0, *aLen);

    for(;;)
    {
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;

        if(strcasecmp(str, "interface") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);
            strncpy(staGetParameter->intf, str,WFA_IF_NAME_LEN-1);
            staGetParameter->intf[WFA_IF_NAME_LEN-1]='\0';
        }
        else if(strcasecmp(str, "program") == 0)
        {
            str = strtok_r(NULL, ",", &pcmdStr);

		  if (strcasecmp(str, "WFD") == 0)
		  {
	  		  staGetParameter->program= PROG_TYPE_WFD;
		  }
		  else if (strcasecmp(str, "WFDS") == 0)
		  {
	  		  staGetParameter->program= PROG_TYPE_WFDS;
		  }
			  
		  str = strtok_r(NULL, ",", &pcmdStr);
		  if(strcasecmp(str, "Parameter") == 0)
		  {
			 str = strtok_r(NULL, ",", &pcmdStr);  
			 if (strcasecmp(str, "DiscoveredDevList") == 0)
			 {
				 staGetParameter->getParamValue= eDiscoveredDevList;
			 }
			 else if (strcasecmp(str, "OpenPorts") == 0)
			 {
				 staGetParameter->getParamValue= eOpenPorts;				 
			 }
			 else if (strcasecmp(str, "NAN") == 0)
		  	 {
			 	staGetParameter->program= PROG_TYPE_NAN;
			 	str = strtok_r(NULL, ",", &pcmdStr);
             	if(strcasecmp(str, "Parameter") == 0)
             	{
					staGetParameter->getParamValue= eMasterPref;
			 	}
		  	 }			 
		  }
		  
	   	}
	   
	}
	
	wfaEncodeTLV(WFA_STA_GET_PARAMETER_TLV, sizeof(caStaGetParameter_t), (BYTE *)staGetParameter, aBuf);
	
	*aLen = 4+sizeof(caStaGetParameter_t);
	 
	return WFA_SUCCESS;
}


int xcCmdProcStaNfcAction(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaNfcAction_t *staNfcAction = (caStaNfcAction_t *) (aBuf+sizeof(wfaTLV));
	char *str;
	
	if(aBuf == NULL)
	   return FALSE;
	
	memset(aBuf, 0, *aLen);
	
	for(;;)
	{
	   str = strtok_r(NULL, ",", &pcmdStr);
	   if(str == NULL || str[0] == '\0')
		  break;
	
	   if(strcasecmp(str, "interface") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);	
		  strncpy(staNfcAction->intf, str,WFA_IF_NAME_LEN-1);
		  staNfcAction->intf[WFA_IF_NAME_LEN-1]='\0';
	   }
	   else if(strcasecmp(str, "operation") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);	
		  if (strcasecmp(str, "write_select") == 0)
		  {
			  staNfcAction->nfcOperation= eNfcWriteSelect;
		  }
		  else if(strcasecmp(str, "read_tag") == 0)
		  {
			  staNfcAction->nfcOperation= eNfcReadTag;
		  }
		  else if(strcasecmp(str, "conn_hndovr") == 0)
		  {
			  staNfcAction->nfcOperation= eNfcHandOver;
		  }
		  else if(strcasecmp(str, "write_config") == 0)
		  {
			  staNfcAction->nfcOperation= eNfcWriteConfig;
		  }
		  else if(strcasecmp(str, "write_passwd") == 0)
		  {
			  staNfcAction->nfcOperation= eNfcWritePasswd;
		  }
		  else if(strcasecmp(str, "wps_conn_hndovr") == 0)
		  {
			  staNfcAction->nfcOperation= eNfcWpsHandOver;
		  }		  
	   }	   
	   else if(strcasecmp(str, "intent_val") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);
		  staNfcAction->intent_val= atoi(str);
		  staNfcAction->intent_val_flag=1;		  
	   }
	   else if(strcasecmp(str, "oper_chn") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);
		  staNfcAction->oper_chn= atoi(str);
		  staNfcAction->oper_chn_flag=1;
	   }
	   else if(strcasecmp(str, "ssid") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);	
		  strncpy(staNfcAction->ssid, str, WFA_SSID_NAME_LEN-1);
		  staNfcAction->ssid[WFA_SSID_NAME_LEN-1]='\0';
		  staNfcAction->ssid_flag =1;
	   }   
	   else if(strcasecmp(str, "init") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);
		  staNfcAction->nfc_init= atoi(str);
		  staNfcAction->nfc_init_flag=1;
	   }	   
	}
	
	wfaEncodeTLV(WFA_STA_NFC_ACTION_TLV, sizeof(caStaNfcAction_t), (BYTE *)staNfcAction, aBuf);
	
	*aLen = 4+sizeof(caStaNfcAction_t);
	 
	return WFA_SUCCESS;
}

int xcCmdProcStaExecAction(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaExecAction_t *staExecAction = (caStaExecAction_t *) (aBuf+sizeof(wfaTLV));
	char *str;
	
	if(aBuf == NULL)
	   return FALSE;
	
	memset(aBuf, 0, *aLen);
	
	for(;;)
	{
	   str = strtok_r(NULL, ",", &pcmdStr);
	   if(str == NULL || str[0] == '\0')
		  break;
	
	   if(strcasecmp(str, "interface") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);	
		  strncpy(staExecAction->intf, str,WFA_IF_NAME_LEN-1);
		  staExecAction->intf[WFA_IF_NAME_LEN-1]='\0';
	   }
	   else if(strcasecmp(str, "prog") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);

		  if (strcasecmp(str, "NAN") == 0)
		  {
	  		  staExecAction->prog= PROG_TYPE_NAN;
		  }
	   }
	   else if(strcasecmp(str, "nanOp") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->nanOp, str, 8);
	   }
	   else if(strcasecmp(str, "masterPref") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->masterPref, str, 8);
	   }
	   else if(strcasecmp(str, "randFactor") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->randFactor, str, 8);
	   }
	   else if(strcasecmp(str, "hopCount") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->hopCount, str, 8);
	   }
	   else if(strcasecmp(str, "highTsf") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->highTsf, str, 8);
	   }
	   else if(strcasecmp(str, "methodType") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->methodType, str, 16);
	   }
	   else if(strcasecmp(str, "furtherAvailInd") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->furtherAvailInd, str, 8);
	   }
	   else if(strcasecmp(str, "mac") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->mac, str, WFA_MAC_ADDR_STR_LEN-1);
         staExecAction->mac[WFA_MAC_ADDR_STR_LEN-1]='\0';
	   }
	   else if(strcasecmp(str, "band") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->band, str, 8);
	   }
	   else if(strcasecmp(str, "fiveGHzOnly") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
		 staExecAction->fiveGHzOnly= atoi(str);
	   }
	   else if(strcasecmp(str, "publishType") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->publishType, str, 16);
	   }
	   else if(strcasecmp(str, "subscribeType") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->subscribeType, str, 16);
	   }
	   else if(strcasecmp(str, "serviceName") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->serviceName, str, 64);
	   }
	   else if(strcasecmp(str, "sdfTxDw") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         staExecAction->sdfTxDw= atoi(str);
	   }
	   else if(strcasecmp(str, "sdfDelay") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         staExecAction->sdfDelay= atoi(str);
	   }
	   else if(strcasecmp(str, "rxMatchFilter") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->rxMatchFilter, str, 64);
	   }
	   else if(strcasecmp(str, "txMatchFilter") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->txMatchFilter, str, 64);
	   }
	   else if(strcasecmp(str, "discRangeLtd") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         staExecAction->discRangeLtd= atoi(str);
	   }
	   else if(strcasecmp(str, "discRangeIgnore") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         staExecAction->discRangeIgnore= atoi(str);
	   }
	   else if(strcasecmp(str, "includeBit") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         staExecAction->includeBit= atoi(str);
	   }
	   else if(strcasecmp(str, "srfType") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         staExecAction->srfType= atoi(str);
	   }
	   else if(strcasecmp(str, "remoteInstanceID") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         staExecAction->remoteInstanceID= atoi(str);
	   }
	   else if(strcasecmp(str, "localInstanceID") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         staExecAction->localInstanceID= atoi(str);
	   }
	   else if(strcasecmp(str, "destMac") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->destMac, str, WFA_MAC_ADDR_STR_LEN-1);
         staExecAction->destMac[WFA_MAC_ADDR_STR_LEN-1]='\0';
	   }
	   else if(strcasecmp(str, "trigger") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         strncpy(staExecAction->trigger, str, 16);
	   }
	   else if(strcasecmp(str, "askForLocCivic") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         staExecAction->askForLocCivic= atoi(str);
	   }
	   else if(strcasecmp(str, "askForLCI") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         staExecAction->askForLCI= atoi(str);
	   }
	   else if(strcasecmp(str, "burstsExponent") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         staExecAction->burstsExponent= atoi(str);
	   }
	   else if(strcasecmp(str, "asap") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         staExecAction->asap= atoi(str);
	   }
	   else if(strcasecmp(str, "formatBwFTM") == 0)
	   {
		 str = strtok_r(NULL, ",", &pcmdStr);
         staExecAction->formatBwFTM= atoi(str);
	   }
	}
	
	wfaEncodeTLV(WFA_STA_EXEC_ACTION_TLV, sizeof(caStaExecAction_t), (BYTE *)staExecAction, aBuf);
	
	*aLen = 4+sizeof(caStaExecAction_t);
	 
	return WFA_SUCCESS;
}

int xcCmdProcStaInvokeCommand(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaInvokeCmd_t *staInvokeCmd = (caStaInvokeCmd_t *) (aBuf+sizeof(wfaTLV));
	char *str;
	
	if(aBuf == NULL)
	   return FALSE;
	
	memset(aBuf, 0, *aLen);
	
	for(;;)
	{
	   str = strtok_r(NULL, ",", &pcmdStr);
	   if(str == NULL || str[0] == '\0')
		  break;
	
	   if(strcasecmp(str, "interface") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);	
		  strncpy(staInvokeCmd->intf, str,WFA_IF_NAME_LEN-1);
		  staInvokeCmd->intf[WFA_IF_NAME_LEN-1]='\0';
	   }
	   else if(strcasecmp(str, "prog") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);	
		  if (strcasecmp(str, "WFDS") == 0)
		  {
			  staInvokeCmd->program= PROG_TYPE_WFDS;
		  }
	   }	   
	   else if(strcasecmp(str, "command_type") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);
		  if (strcasecmp(str, "Primitive") == 0)
		  {
			  staInvokeCmd->cmdType= ePrimitiveCmdType;
		  }
		  else if(strcasecmp(str, "Message") == 0)
		  {
			  staInvokeCmd->cmdType= eMessageCmdType;
		  }
	   }

	   else if(strcasecmp(str, "primitive_type") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);
		  if (strcasecmp(str, "Advertise") == 0)
		  {
			  staInvokeCmd->InvokeCmds.primtiveType.AdvPrim.PrimType= eCmdPrimTypeAdvt;

			  for(;;)
			  {
				 str = strtok_r(NULL, ",", &pcmdStr);
				 if(str == NULL || str[0] == '\0')
					break;

				  if (strcasecmp(str, "service_name") == 0)
				  {
					  str = strtok_r(NULL, ",", &pcmdStr);
					  if (strcasecmp(str, "OOB") == 0)
					  {
						  strcpy(staInvokeCmd->InvokeCmds.primtiveType.AdvPrim.serviceName, "OOB");						  
					  }
					  else
					  {
						  strncpy(staInvokeCmd->InvokeCmds.primtiveType.AdvPrim.serviceName, str,63);						 
						  staInvokeCmd->InvokeCmds.primtiveType.AdvPrim.serviceName[63]='\0';						  
					  }

					  staInvokeCmd->InvokeCmds.primtiveType.AdvPrim.serviceName_flag= 1;
				  }
				  else if (strcasecmp(str, "auto_accept") == 0)
				  {
					  staInvokeCmd->InvokeCmds.primtiveType.AdvPrim.autoAccept_flag= 1;				  		
					  str = strtok_r(NULL, ",", &pcmdStr);

					  staInvokeCmd->InvokeCmds.primtiveType.AdvPrim.autoAccpet= atoi(str);

				  }
				  else if (strcasecmp(str, "service_info") == 0)
				  {
					  staInvokeCmd->InvokeCmds.primtiveType.AdvPrim.serviceInfo_flag= 1;				  		
					  str = strtok_r(NULL, ",", &pcmdStr);
					  strncpy(staInvokeCmd->InvokeCmds.primtiveType.AdvPrim.serviceInfo, str,63);
					  staInvokeCmd->InvokeCmds.primtiveType.AdvPrim.serviceInfo[63]='\0';

				  }
				  else if (strcasecmp(str, "service_status") == 0)
				  {
					  staInvokeCmd->InvokeCmds.primtiveType.AdvPrim.serviceStatus_flag= 1;				  		
					  str = strtok_r(NULL, ",", &pcmdStr);
  					  staInvokeCmd->InvokeCmds.primtiveType.AdvPrim.serviceStaus= atoi(str);
				  }
				  
			  }
				  
				  
		  }

		  else if (strcasecmp(str, "Seek") == 0)
		  {
			  staInvokeCmd->InvokeCmds.primtiveType.SeekPrim.PrimType= eCmdPrimTypeSeek;

			  for(;;)
			  {
				 str = strtok_r(NULL, ",", &pcmdStr);
				 if(str == NULL || str[0] == '\0')
					break;

				  if (strcasecmp(str, "service_name") == 0)
				  {
					  str = strtok_r(NULL, ",", &pcmdStr);
					  if (strcasecmp(str, "OOB") == 0)
					  {
						  strcpy(staInvokeCmd->InvokeCmds.primtiveType.SeekPrim.serviceName, "OOB");						  
					  }
					  else
					  {
						  strncpy(staInvokeCmd->InvokeCmds.primtiveType.SeekPrim.serviceName, str,63);						  
						  staInvokeCmd->InvokeCmds.primtiveType.SeekPrim.serviceName[63]='\0';
						  
					  }
					  staInvokeCmd->InvokeCmds.primtiveType.SeekPrim.serviceName_flag= 1;
				  }
				  else if (strcasecmp(str, "exact_search") == 0)
				  {
					  staInvokeCmd->InvokeCmds.primtiveType.SeekPrim.exactSearch_flag= 1;				  		
					  str = strtok_r(NULL, ",", &pcmdStr);
  					  staInvokeCmd->InvokeCmds.primtiveType.SeekPrim.exactSearch= atoi(str);
				  }
				  else if (strcasecmp(str, "mac_address") == 0)
				  {
					  staInvokeCmd->InvokeCmds.primtiveType.SeekPrim.macAddress_flag= 1;				  		
					  str = strtok_r(NULL, ",", &pcmdStr);
					  strncpy(staInvokeCmd->InvokeCmds.primtiveType.SeekPrim.macaddress,str,WFA_MAC_ADDR_STR_LEN-1);
		  			  staInvokeCmd->InvokeCmds.primtiveType.SeekPrim.macaddress[WFA_MAC_ADDR_STR_LEN-1]='\0';
				  }
				  else if (strcasecmp(str, "service_info") == 0)
				  {
					  staInvokeCmd->InvokeCmds.primtiveType.SeekPrim.serviceInfo_flag= 1;				  		
					  str = strtok_r(NULL, ",", &pcmdStr);
					  strncpy(staInvokeCmd->InvokeCmds.primtiveType.SeekPrim.serviceInfo, str,63);
					  staInvokeCmd->InvokeCmds.primtiveType.SeekPrim.serviceInfo[63]='\0';
				  }				  
				  
			  	}
		  }
		  else if(strcasecmp(str, "Cancel") == 0)
		  {
			  staInvokeCmd->InvokeCmds.primtiveType.CancelPrim.PrimType= eCmdPrimTypeCancel;
			  for(;;)
			  {
				 str = strtok_r(NULL, ",", &pcmdStr);
				 if(str == NULL || str[0] == '\0')
					break;
				 if (strcasecmp(str, "cancelMethod") == 0)
				 {		  
				 	str = strtok_r(NULL, ",", &pcmdStr);
					staInvokeCmd->InvokeCmds.primtiveType.CancelPrim.cancelMethod_flag= 1;
					if (strcasecmp(str, "Seek") == 0)
					  	staInvokeCmd->InvokeCmds.primtiveType.CancelPrim.cancelMethod= eCmdPrimTypeSeek;
					else if (strcasecmp(str, "Advertise") == 0) // not  defined in CAPI, just for information
					  	staInvokeCmd->InvokeCmds.primtiveType.CancelPrim.cancelMethod= eCmdPrimTypeAdvt;				

				 }
				 
			  }
		  }
		  else if(strcasecmp(str, "ConnectSession") == 0)
		  {
			  staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.PrimType= eCmdPrimTypeConnSession;
			  for(;;)
			  {
				 str = strtok_r(NULL, ",", &pcmdStr);
				 if(str == NULL || str[0] == '\0')
					break;
				 if (strcasecmp(str, "service_mac") == 0)
				 {
				 	 str = strtok_r(NULL, ",", &pcmdStr);
					 staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.serviceMac_flag= 1;				 
					 strncpy(staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.serviceMac,str,WFA_MAC_ADDR_STR_LEN-1);
					 staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.serviceMac[WFA_MAC_ADDR_STR_LEN-1]='\0';
				 }
				 else if (strcasecmp(str, "AdvID") == 0)
				 {
				 	 str = strtok_r(NULL, ",", &pcmdStr);
					 staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.advId_flag= 1;				 
					 staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.advID = strtol(str,NULL,16);
				 }			
				 else if (strcasecmp(str, "session_info") == 0)
				 {
				 	str = strtok_r(NULL, ",", &pcmdStr);
					staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.sessionInfo_flag= 1;				  		
					strncpy(staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.sessionInfo,str,63);
					staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.sessionInfo[63]='\0';
				 }				 				 
				 else if (strcasecmp(str, "network_role") == 0)
				 {
					staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.networkRole_flag= 1;				  		
					str = strtok_r(NULL, ",", &pcmdStr);
					staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.networkRole= atoi(str);
				 }				 				 
				 else if (strcasecmp(str, "connectionCapabilityInfo") == 0)
				 {
					staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.connCapInfo_flag= 1;				  		
					str = strtok_r(NULL, ",", &pcmdStr);
					if(strcasecmp(str, "GO") == 0)
					{
					staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.connCapInfo= eWfdsGO;
					}
					else if (strcasecmp(str, "CLI") == 0)
					{
					staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.connCapInfo= eWfdsCLI;
					}
					else if (strcasecmp(str, "NewGO") == 0)
					{
					staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.connCapInfo= eWfdsNewGO;
					}
					else if (strcasecmp(str, "New") == 0)
					{
					staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.connCapInfo= eWfdsNew;
					}
					else if (strcasecmp(str, "CliGO") == 0)
					{
					staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.connCapInfo= eWfdsCliGO;
					}
				 }				 				 		 
				 else if (strcasecmp(str, "ssid") == 0)
				 {
					staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.ssid_flag= 1;				  		
					str = strtok_r(NULL, ",", &pcmdStr);
					strncpy(staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.ssid,str,63);
					staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.ssid[63]='\0';					
				 }				 				 
				 else if (strcasecmp(str, "Oper_chn") == 0)
				 {
					staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.operChn_flag= 1;				  		
					str = strtok_r(NULL, ",", &pcmdStr);
					staInvokeCmd->InvokeCmds.primtiveType.ConnSessPrim.operChn= atoi(str);
				 }				 				 
			  }
			  
		  }		  

		   else if(strcasecmp(str, "ConfirmSession") == 0)
		   {
			   staInvokeCmd->InvokeCmds.primtiveType.ConfSessPrim.PrimType= eCmdPrimTypeConfirmSession;
			   for(;;)
			   {
				  str = strtok_r(NULL, ",", &pcmdStr);
				  if(str == NULL || str[0] == '\0')
					 break;
				  if (strcasecmp(str, "session_mac") == 0)
				  {
					  str = strtok_r(NULL, ",", &pcmdStr);
					  staInvokeCmd->InvokeCmds.primtiveType.ConfSessPrim.sessionMac_flag= 1; 			  
					  strncpy(staInvokeCmd->InvokeCmds.primtiveType.ConfSessPrim.sessionMac,str,WFA_MAC_ADDR_STR_LEN-1);
					  staInvokeCmd->InvokeCmds.primtiveType.ConfSessPrim.sessionMac[WFA_MAC_ADDR_STR_LEN-1]='\0';
				  }
				  else if (strcasecmp(str, "session_id") == 0)
				  {
					  str = strtok_r(NULL, ",", &pcmdStr);
					  staInvokeCmd->InvokeCmds.primtiveType.ConfSessPrim.sessionID_flag= 1;				  
					  staInvokeCmd->InvokeCmds.primtiveType.ConfSessPrim.sessionID = strtol(str,NULL,16);
				  } 		 
				  else if (strcasecmp(str, "confirmed") == 0)
				  {
					 staInvokeCmd->InvokeCmds.primtiveType.ConfSessPrim.confirmed_flag= 1; 					 
					 str = strtok_r(NULL, ",", &pcmdStr);
					 staInvokeCmd->InvokeCmds.primtiveType.ConfSessPrim.confirmed= atoi(str);
				  } 							  
			   	}
		   }	   
		   else if(strcasecmp(str, "SetSessionReady") == 0)
		   {
			   staInvokeCmd->InvokeCmds.primtiveType.SetSessRdyPrim.PrimType= eCmdPrimTypeSetSessionReady;
			   for(;;)
			   {
				  str = strtok_r(NULL, ",", &pcmdStr);
				  if(str == NULL || str[0] == '\0')
					 break;
				  if (strcasecmp(str, "session_mac") == 0)
				  {
					  str = strtok_r(NULL, ",", &pcmdStr);
					  staInvokeCmd->InvokeCmds.primtiveType.SetSessRdyPrim.sessionMac_flag= 1; 			  
					  strncpy(staInvokeCmd->InvokeCmds.primtiveType.SetSessRdyPrim.sessionMac,str,WFA_MAC_ADDR_STR_LEN-1);
					  staInvokeCmd->InvokeCmds.primtiveType.SetSessRdyPrim.sessionMac[WFA_MAC_ADDR_STR_LEN-1]='\0';
				  }
				  else if (strcasecmp(str, "session_id") == 0)
				  {
					  str = strtok_r(NULL, ",", &pcmdStr);
					  staInvokeCmd->InvokeCmds.primtiveType.SetSessRdyPrim.sessionID_flag= 1;				  
					  staInvokeCmd->InvokeCmds.primtiveType.SetSessRdyPrim.sessionID = strtol(str,NULL,16);
				  } 		 
			   	}
		   }	   
		   else if(strcasecmp(str, "BoundPort") == 0)
		   {
			   staInvokeCmd->InvokeCmds.primtiveType.BoundPortPrim.PrimType= eCmdPrimTypeBoundPort;
			   for(;;)
			   {
				  str = strtok_r(NULL, ",", &pcmdStr);
				  if(str == NULL || str[0] == '\0')
					 break;
				  if (strcasecmp(str, "session_mac") == 0)
				  {
					  str = strtok_r(NULL, ",", &pcmdStr);
					  staInvokeCmd->InvokeCmds.primtiveType.BoundPortPrim.sessionMac_flag= 1; 			  
					  strncpy(staInvokeCmd->InvokeCmds.primtiveType.BoundPortPrim.sessionMac,str,WFA_MAC_ADDR_STR_LEN-1);
					  staInvokeCmd->InvokeCmds.primtiveType.BoundPortPrim.sessionMac[WFA_MAC_ADDR_STR_LEN-1]='\0';
				  }
				  else if (strcasecmp(str, "session_id") == 0)
				  {
					  str = strtok_r(NULL, ",", &pcmdStr);
					  staInvokeCmd->InvokeCmds.primtiveType.BoundPortPrim.sessionID_flag= 1;				  
					  staInvokeCmd->InvokeCmds.primtiveType.BoundPortPrim.sessionID = strtol(str,NULL,16);
				  }
				  else if (strcasecmp(str, "port") == 0)
				  {
					 staInvokeCmd->InvokeCmds.primtiveType.BoundPortPrim.port_flag= 1; 					 
					 str = strtok_r(NULL, ",", &pcmdStr);
					 staInvokeCmd->InvokeCmds.primtiveType.BoundPortPrim.port= atoi(str);
				  } 							  
				  
			   	}
			   
		   }	   
		   else if(strcasecmp(str, "ServiceStatusChange") == 0)
		   {
			   staInvokeCmd->InvokeCmds.primtiveType.ServStatusChngPrim.PrimType= eCmdPrimTypeServiceStatusChange;
			   for(;;)
			   {
				  str = strtok_r(NULL, ",", &pcmdStr);
				  if(str == NULL || str[0] == '\0')
					 break;
				  if (strcasecmp(str, "advId") == 0)
				  {
					  str = strtok_r(NULL, ",", &pcmdStr);
					  staInvokeCmd->InvokeCmds.primtiveType.ServStatusChngPrim.advId_flag= 1;				  
					  staInvokeCmd->InvokeCmds.primtiveType.ServStatusChngPrim.advID = strtol(str,NULL,16);
				  }
				  else if (strcasecmp(str, "ServiceStatus") == 0)
				  {
					 staInvokeCmd->InvokeCmds.primtiveType.ServStatusChngPrim.serviceStatus_flag= 1; 					 
					 str = strtok_r(NULL, ",", &pcmdStr);
					 staInvokeCmd->InvokeCmds.primtiveType.ServStatusChngPrim.serviceStatus= atoi(str);
				  } 							  
				  
			   	}
			   
		   }	   
		   else if(strcasecmp(str, "CloseSession") == 0)
		   {
			   staInvokeCmd->InvokeCmds.primtiveType.CloseSessPrim.PrimType= eCmdPrimTypeCloseSession;
			   for(;;)
			   {
				  str = strtok_r(NULL, ",", &pcmdStr);
				  if(str == NULL || str[0] == '\0')
					 break;
				  if (strcasecmp(str, "session_mac") == 0)
				  {
					  str = strtok_r(NULL, ",", &pcmdStr);
					  staInvokeCmd->InvokeCmds.primtiveType.CloseSessPrim.sessionMac_flag= 1; 			  
					  strncpy(staInvokeCmd->InvokeCmds.primtiveType.CloseSessPrim.sessionMac,str,WFA_MAC_ADDR_STR_LEN-1);
					  staInvokeCmd->InvokeCmds.primtiveType.CloseSessPrim.sessionMac[WFA_MAC_ADDR_STR_LEN-1]='\0';
				  }
				  else if (strcasecmp(str, "session_id") == 0)
				  {
					  str = strtok_r(NULL, ",", &pcmdStr);
					  staInvokeCmd->InvokeCmds.primtiveType.CloseSessPrim.sessionID_flag= 1;				  
					  staInvokeCmd->InvokeCmds.primtiveType.CloseSessPrim.sessionID = strtol(str,NULL,16);
				  }
			   	}
			   
		   }	   
		  
	   }

	   else if(strcasecmp(str, "opcode") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);
		  if (strcasecmp(str, "REQUEST_SESSION") == 0)
		  	staInvokeCmd->InvokeCmds.Msg.opcode= eMsgReqSession;
		  else if (strcasecmp(str, "REMOVE_SESSION") == 0)
		  	staInvokeCmd->InvokeCmds.Msg.opcode= eMsgRmvSession;
		  else if (strcasecmp(str, "REJECT_SESSION") == 0)
		  	staInvokeCmd->InvokeCmds.Msg.opcode= eMsgRejSession;
		  else if (strcasecmp(str, "ADDED_SESSION") == 0)
		  	staInvokeCmd->InvokeCmds.Msg.opcode= eMsgAddedSession;		  

			for(;;)
			{
			  str = strtok_r(NULL, ",", &pcmdStr);
			  if(str == NULL || str[0] == '\0')
				 break;
			  if (strcasecmp(str, "session_id") == 0)
			  { 	   
				 str = strtok_r(NULL, ",", &pcmdStr);
				 staInvokeCmd->InvokeCmds.Msg.sessionId_flag= 1;
				 staInvokeCmd->InvokeCmds.Msg.sessionID = strtol(str,NULL,16);				 
			  }
			  else if (strcasecmp(str, "session_mac") == 0) // not  defined in CAPI, just for information
			  {
				  str = strtok_r(NULL, ",", &pcmdStr);
				  staInvokeCmd->InvokeCmds.Msg.sessionMac_flag= 1;
				  strncpy(staInvokeCmd->InvokeCmds.Msg.sessionMac,str,WFA_MAC_ADDR_STR_LEN-1);
				  staInvokeCmd->InvokeCmds.Msg.sessionMac[WFA_MAC_ADDR_STR_LEN-1]='\0';
			  }

			}

		  
	   }
	}
	
	wfaEncodeTLV(WFA_STA_INVOKE_COMMAND_TLV, sizeof(caStaInvokeCmd_t), (BYTE *)staInvokeCmd, aBuf);
	
	*aLen = 4+sizeof(caStaInvokeCmd_t);
	 
	return WFA_SUCCESS;
}

int xcCmdProcStaManageService(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaMngServ_t *staManageServCmd = (caStaMngServ_t *) (aBuf+sizeof(wfaTLV));
	char *str;
	
	if(aBuf == NULL)
	   return FALSE;
	
	memset(aBuf, 0, *aLen);
	
	for(;;)
	{
	   str = strtok_r(NULL, ",", &pcmdStr);
	   if(str == NULL || str[0] == '\0')
		  break;
	
	   if(strcasecmp(str, "interface") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);	
		  strncpy(staManageServCmd->intf, str,WFA_IF_NAME_LEN-1);
		  staManageServCmd->intf[WFA_IF_NAME_LEN-1]='\0';
	   }
	   else if(strcasecmp(str, "prog") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);	
		  if (strcasecmp(str, "WFDS") == 0)
		  {
			  staManageServCmd->program= PROG_TYPE_WFDS;
			  for(;;)
			  {
				 str = strtok_r(NULL, ",", &pcmdStr);
				 if(str == NULL || str[0] == '\0')
					break;
				 if (strcasecmp(str, "service_name") == 0)
				 {
					 str = strtok_r(NULL, ",", &pcmdStr);
					 if(strcasecmp(str, "Send") == 0)
					 {
						 staManageServCmd->MngCmds.MgtServ.serviceName= eServiceNameSend;
					 }
					 else if(strcasecmp(str, "Display") == 0)
					 {
						 staManageServCmd->MngCmds.MgtServ.serviceName= eServiceNameDisplay;
					 }
					 else if(strcasecmp(str, "Play") == 0)
					 {
						 staManageServCmd->MngCmds.MgtServ.serviceName= eServiceNamePlay;
					 }
					 else if(strcasecmp(str, "Print") == 0)
					 {
						 staManageServCmd->MngCmds.MgtServ.serviceName= eServiceNamePrint;
					 }

				 }
				 else if (strcasecmp(str, "service_role") == 0)
				 {
					 str = strtok_r(NULL, ",", &pcmdStr);
					 if (strcasecmp(str, "Tx") == 0)
					   staManageServCmd->MngCmds.MgtServ.serviceRole= eServiceRoleTx;
					 else
					   staManageServCmd->MngCmds.MgtServ.serviceRole= eServiceRoleRx;
				 }

				 
				 if (strcasecmp(str, "service_mac") == 0)
				 {
				 	 str = strtok_r(NULL, ",", &pcmdStr);				 
					 staManageServCmd->MngCmds.MgtServ.serviceMac_flag= 1;				 
					 strncpy(staManageServCmd->MngCmds.MgtServ.serviceMac,str,WFA_MAC_ADDR_STR_LEN-1);
					 staManageServCmd->MngCmds.MgtServ.serviceMac[WFA_MAC_ADDR_STR_LEN-1]='\0';
				 }
				 else if (strcasecmp(str, "AdvID") == 0)
				 {
				 	 str = strtok_r(NULL, ",", &pcmdStr);				 
					 staManageServCmd->MngCmds.MgtServ.advId_flag= 1;				 
					 staManageServCmd->MngCmds.MgtServ.advID=strtol(str,NULL,16);;
				 }			
				 else if (strcasecmp(str, "session_info") == 0)
				 {
				 	str = strtok_r(NULL, ",", &pcmdStr);				 
					staManageServCmd->MngCmds.MgtServ.sessionInfo_flag= 1;				  		
					strncpy(staManageServCmd->MngCmds.MgtServ.sessionInfo,str,63);
					staManageServCmd->MngCmds.MgtServ.sessionInfo[63]='\0';
				 }				 				 
				 else if (strcasecmp(str, "network_role") == 0)
				 {
					staManageServCmd->MngCmds.MgtServ.networkRole_flag= 1;				  		
					str = strtok_r(NULL, ",", &pcmdStr);
					staManageServCmd->MngCmds.MgtServ.networkRole= atoi(str);
				 }				 				 
				 else if (strcasecmp(str, "connectionCapabilityInfo") == 0)
				 {
					staManageServCmd->MngCmds.MgtServ.connCapInfo_flag= 1;				  		
					str = strtok_r(NULL, ",", &pcmdStr);
					if(strcasecmp(str, "GO") == 0)
					{
					staManageServCmd->MngCmds.MgtServ.connCapInfo= eWfdsGO;
					}
					else if (strcasecmp(str, "CLI") == 0)
					{
					staManageServCmd->MngCmds.MgtServ.connCapInfo= eWfdsCLI;
					}
					else if (strcasecmp(str, "NewGO") == 0)
					{
					staManageServCmd->MngCmds.MgtServ.connCapInfo= eWfdsNewGO;
					}
					else if (strcasecmp(str, "New") == 0)
					{
					staManageServCmd->MngCmds.MgtServ.connCapInfo= eWfdsNew;
					}
					else if (strcasecmp(str, "CliGO") == 0)
					{
					staManageServCmd->MngCmds.MgtServ.connCapInfo= eWfdsCliGO;
					}
				 }		
				 else if (strcasecmp(str, "manage_actions") == 0)
				 {
					staManageServCmd->MngCmds.MgtServ.mngActions_flag= 1;				  		
					str = strtok_r(NULL, ",", &pcmdStr);

					char * str2, *subtoken, *saveptr2;
					int index=0;
					for (str2 = str; ; str2 = NULL) 
					{
						 subtoken = strtok_r(str2," ",&saveptr2);
						 if (subtoken == NULL)
							 break;
						 if(strcasecmp(str, "transfer") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsTransfer;
						 }
						 else if (strcasecmp(str, "pause") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsPause;
						 }
						 else if (strcasecmp(str, "resume") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsResume;
						 }
						 else if (strcasecmp(str, "modify") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsModify;
						 }
						 else if (strcasecmp(str, "cancel") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsCancel;
						 }
						 else if (strcasecmp(str, "amidClose") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsAmidClose;
						 }
						 else if (strcasecmp(str, "close") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsClose;
						 }
						 else if (strcasecmp(str, "receive") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsReceive;
						 }
						 else if (strcasecmp(str, "play") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsPlay;
						 }
						 else if (strcasecmp(str, "display") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsDisplay;
						 }
						 else if (strcasecmp(str, "GetPrinterAttr") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsGetPrintAttr;
						 }
						 else if (strcasecmp(str, "PrintJobOperation") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsPrintJobOper;
						 }
						 else if (strcasecmp(str, "GetJobAttr") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsGetJobAttr;
						 }
						 else if (strcasecmp(str, "CreateJobOper") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsCreateJobOper;
						 }
						 else if (strcasecmp(str, "SendPrintDoc") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsSendPrintDoc;
						 }
						 else if (strcasecmp(str, "DoNothing") == 0)
						 {
						 	staManageServCmd->MngCmds.MgtServ.mgtActions[index]= eWfdsMgtActionsDoNothing;
						 }
					 
						 index++;						
					}
					
					staManageServCmd->MngCmds.MgtServ.numMngActions= index;

				 }		
				 else if (strcasecmp(str, "send_fileList") == 0)
				 {
					staManageServCmd->MngCmds.MgtServ.sendFileList_flag= 1;				  		
					str = strtok_r(NULL, ",", &pcmdStr);

					char * str2, *subtoken, *saveptr2;
					int index=0;
					for (str2 = str; ; str2 = NULL) 
					{
						 subtoken = strtok_r(str2," ",&saveptr2);
						 if (subtoken == NULL)
							 break;
						 strncpy(staManageServCmd->MngCmds.MgtServ.fileList[index],str,16);
						 staManageServCmd->MngCmds.MgtServ.fileList[index][16]='\0';
						 index++;
					}
					staManageServCmd->MngCmds.MgtServ.numModFiles= index;					
				 }
				 else if (strcasecmp(str, "sendModify_FileList") == 0)
				 {
					staManageServCmd->MngCmds.MgtServ.modSendFileList_flag= 1;				  		
					str = strtok_r(NULL, ",", &pcmdStr);

					char * str2, *subtoken, *saveptr2;
					int index=0;
					for (str2 = str; ; str2 = NULL) 
					{
						 subtoken = strtok_r(str2," ",&saveptr2);
						 if (subtoken == NULL)
							 break;
						 strncpy(staManageServCmd->MngCmds.MgtServ.modFileList[index],str,16);
						 staManageServCmd->MngCmds.MgtServ.modFileList[index][16]='\0';
						 index++;
					}
					staManageServCmd->MngCmds.MgtServ.numModFiles= index;					
				 }
				 else if (strcasecmp(str, "PdlType") == 0)
				 {

					staManageServCmd->MngCmds.MgtServ.PdlType_flag= 1;				  		
					str = strtok_r(NULL, ",", &pcmdStr);
					if(strcasecmp(str, "PCLM") == 0)
					{
						staManageServCmd->MngCmds.MgtServ.PdlType= ePclmPdr;
					}
					else if (strcasecmp(str, "PWG") == 0)
					{
						staManageServCmd->MngCmds.MgtServ.PdlType= ePwgPdr;
					}
				 }				 
			  }
		  }


		  
	   }
	   

   	}
   
   	wfaEncodeTLV(WFA_STA_MANAGE_SERVICE_TLV, sizeof(caStaMngServ_t), (BYTE *)staManageServCmd, aBuf);
   
   	*aLen = 4+sizeof(caStaMngServ_t);
	
   	return WFA_SUCCESS;
}

int xcCmdProcStaGetEvents(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaGetEvents_t *staGetEvents = (caStaGetEvents_t *) (aBuf+sizeof(wfaTLV));
	char *str;
	
	if(aBuf == NULL)
	   return FALSE;
	
	memset(aBuf, 0, *aLen);
	
	for(;;)
	{
	   str = strtok_r(NULL, ",", &pcmdStr);
	   if(str == NULL || str[0] == '\0')
		  break;
	
	   if(strcasecmp(str, "interface") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);	
		  strncpy(staGetEvents->intf, str,WFA_IF_NAME_LEN-1);
		  staGetEvents->intf[WFA_IF_NAME_LEN-1]='\0';
	   }
	   else if(strcasecmp(str, "program") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);

		  if (strcasecmp(str, "WFDS") == 0)
		  {
	  		  staGetEvents->program= PROG_TYPE_WFDS;
		  }
		  if (strcasecmp(str, "NAN") == 0)
		  {
	  		  staGetEvents->program= PROG_TYPE_NAN;
		  }
	   	} 
		else if(strcasecmp(str, "action") == 0)
	    {
        	str = strtok_r(NULL, ",", &pcmdStr);  
        	strncpy(staGetEvents->action, str, WFA_EVT_ACTION_LEN-1);
        	staGetEvents->action[WFA_EVT_ACTION_LEN-1]='\0';
	    }
	   
	}
	
	wfaEncodeTLV(WFA_STA_GET_EVENTS_TLV, sizeof(caStaGetEvents_t), (BYTE *)staGetEvents, aBuf);
	
	*aLen = 4+sizeof(caStaGetEvents_t);
	 
	return WFA_SUCCESS;
}

int xcCmdProcStaGetEventDetails(char *pcmdStr, BYTE *aBuf, int *aLen)
{
	caStaGetEventDetails_t *staGetEventDetails = (caStaGetEventDetails_t *) (aBuf+sizeof(wfaTLV));
	char *str;
	
	if(aBuf == NULL)
	   return FALSE;
	
	memset(aBuf, 0, *aLen);
	
	for(;;)
	{
	   str = strtok_r(NULL, ",", &pcmdStr);
	   if(str == NULL || str[0] == '\0')
		  break;
	
	   if(strcasecmp(str, "interface") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);	
		  strncpy(staGetEventDetails->intf, str,WFA_IF_NAME_LEN-1);
		  staGetEventDetails->intf[WFA_IF_NAME_LEN-1]='\0';
	   }
	   else if(strcasecmp(str, "program") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);

		  if (strcasecmp(str, "WFDS") == 0)
		  {
	  		  staGetEventDetails->program= PROG_TYPE_WFDS;
		  }
	   	}
	   else if(strcasecmp(str, "EventName") == 0)
	   {
		  str = strtok_r(NULL, ",", &pcmdStr);

		  if (strcasecmp(str, "SearchResult") == 0)
		  {
	  		  staGetEventDetails->eventId= eSearchResult; 			  
		  }
		  else if(strcasecmp(str, "SearchTerminated") == 0)
		  {
	  		  staGetEventDetails->eventId= eSearchTerminated;
		  }
		  else if(strcasecmp(str, "AdvertiseStatus") == 0)
		  {
	  		  staGetEventDetails->eventId= eAdvertiseStatus;
		  }
		  else if(strcasecmp(str, "SessionRequest") == 0)
		  {
	  		  staGetEventDetails->eventId= eSessionRequest;
		  }
		  else if(strcasecmp(str, "ConnectStatus") == 0)
		  {
	  		  staGetEventDetails->eventId= eConnectStatus;
		  }
		  else if(strcasecmp(str, "SessionStatus") == 0)
		  {
	  		  staGetEventDetails->eventId= eSessionStatus;
		  }
		  else if(strcasecmp(str, "PortStatus") == 0)
		  {
	  		  staGetEventDetails->eventId= ePortStatus;
		  }
		  
	   	}
	   
	}
	
	wfaEncodeTLV(WFA_STA_GET_EVENT_DETAILS_TLV, sizeof(caStaGetEventDetails_t), (BYTE *)staGetEventDetails, aBuf);
	
	*aLen = 4+sizeof(caStaGetEventDetails_t);
	 
	return WFA_SUCCESS;
}




