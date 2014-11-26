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
 

#ifndef WFA_VER_H
#define WFA_VER_H

#define WFA_VERNAM_LEN      32
/*
 * Current version support baseline programs and upcoming programs:
 *     Voice Enterprise
 *     WMM-AC
 *     PMF
 *     TDLS
 *     VHT
 *     Passpoint release 1
 *
 *     Releases:
 *     3.2 -- This release includes PMF and TDLS support
 *     3.3.2
 *     3.3.3 -- add the argument "channel" to TDLS optional
 *     3.3.4 -- add the argument "type" and "peer" to set_uapsd for TDLS
 *     3.3.5 -- add the "wfaStaSetMacAddress()" to wfa_cmdproc.c 
 *     3.3.6 -- update all KeyMgmt options to support "sha256", "ft" etc.
 *              Add NeigReq and TransMgmtQuery Frame to "STA_SEND_FRAME"
 *              Remove Send_NeigReq command.
 *     3.3.7 -- Add "wpa2" to keyMgmt
 *     3.3.8 -- add "sta_set_radio" command for TDLS
 *     3.3.9 -- add "sta_set_rfeature" command for TDLS
 *              add "tdlsMode" to Preset Command
 *     3.3.10 - fix a few bug during TDLS PF 
 *     6.0.0 -- This release includes Wi-Fi Display support ( Base )
 *     6.0.1 -- PF#1 - based on CAPI version 6.0.1
 *     6.0.2 -- PF#2 - based on CAPI version 6.0.2
 *	   6.0.0-DISP-v0.3 -- PF#3 - based on CAPI version 6.0.2 	
 *	   6.0.0-DISP-v0.4 -- PF#4 - based on Test plan version 0.13 	
 *	   6.0.0-DISP-v0.5 -- PF#5 
 *	   6.0.0-DISP-v0.5 -- TBRE 
 *     8.0.0 -- This release includes all changes from VHT program.
 *              This includes IPv6 ping support needed for Passpoint
 */
#define WFA_SYSTEM_VER      "PCEDUT-SIGMA-8.1.0"     
                                 

#endif
