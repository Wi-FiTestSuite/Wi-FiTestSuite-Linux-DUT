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
 * file: wfa_tlv.c
 * The file contains all TLV process functions, including Encode, Decode, etc
 * TLV stands for Tag, Length, Value.
 *
 */
#include "wfa_portall.h"
#include "wfa_stdincs.h"
#include "wfa_main.h"
#include "wfa_debug.h"
#include "wfa_types.h"
#include "wfa_tlv.h"

extern unsigned short wfa_defined_debug;

/*
 * wfaEncodeTLV(): Encoding a packet to TLV format
 * input: the_tag - packet type
 *        the_len - the value length
 *        the_value - the value buffer
 *
 * output: tlv_data - encoded TLV packet buffer. Caller must allocate the buffer
 */
BOOL wfaEncodeTLV(WORD the_tag, WORD the_len, BYTE *the_value, BYTE *tlv_data)
{
    void *data = tlv_data;

    ((wfaTLV *)data)->tag = the_tag;
    ((wfaTLV *)data)->len = the_len;
    if(the_value != NULL && the_len != 0)
        wMEMCPY((data+4), (BYTE *)the_value, the_len);

    return WFA_SUCCESS;
}

/*
 * wfaDecodeTLV(); Decoding a TLV format into actually values
 * input:  tlv_data - the TLV format packet buffer
 *         tlv_len  - the total length of the TLV
 * output: ptag - the TLV type
 *         pval_len - the value length
 *         pvalue - value buffer, caller must allocate the buffer
 */

BOOL wfaDecodeTLV(BYTE *tlv_data, int tlv_len, WORD *ptag, int *pval_len, BYTE *pvalue)
{
    wfaTLV *data = (wfaTLV *)tlv_data;

    if(pvalue == NULL)
    {
        DPRINT_ERR(WFA_ERR, "Parm buf invalid\n");
        return WFA_FAILURE;
    }
    *ptag = data->tag;
    *pval_len = data->len;

    if(tlv_len < *pval_len)
        return WFA_FAILURE;

    if(*pval_len != 0 && *pval_len < MAX_PARMS_BUFF)
    {
        wMEMCPY(pvalue, tlv_data+4, *pval_len);
    }

    return WFA_SUCCESS;
}

/*
 * wfaGetTLVTag(): the individual function to retrieve a TLV type.
 * input: tlv_data - TLV buffer
 * return: the TLV type.
 */

WORD wfaGetTLVTag(BYTE *tlv_data)
{
    wfaTLV *ptlv = (wfaTLV *)tlv_data;

    if(ptlv != NULL)
        return ptlv->tag;

    return WFA_SUCCESS;
}

/*
 * wfaSetTLVTag(): the individual function to set TLV type.
 * input: new_tag - the new TLV type.
 * Output: tlv_data - a TLV buffer, caller must allocate this buffer.
 */

BOOL wfaSetTLVTag(WORD new_tag, BYTE *tlv_data)
{
    wfaTLV *ptlv = (wfaTLV *)tlv_data;

    if(tlv_data == NULL)
        return WFA_FAILURE;

    ptlv->tag = new_tag;

    return WFA_SUCCESS;
}

/*
 * wfaGetTLVLen(): retrieve a TLV value length
 * input: tlv_data - a TLV buffer
 * return: the value length.
 */

WORD wfaGetTLVLen(BYTE *tlv_data)
{
    wfaTLV *ptlv = (wfaTLV *)tlv_data;

    if(tlv_data == NULL)
        return WFA_FAILURE;

    return ptlv->len;
}

/*
 * wfaGetTLVValue(): retrieve a TLV value
 * input: value_len - TLV value length
 *        tlv_data - a TLV data buffer
 * output: pvalue - the value buffer, caller must allocate it.
 */

BOOL wfaGetTLVvalue(int value_len, BYTE *tlv_data, BYTE *pvalue)
{
    if(tlv_data == NULL)
        return WFA_FAILURE;

    wMEMCPY(pvalue, tlv_data+WFA_TLV_HEAD_LEN, value_len);

    return WFA_SUCCESS;
}
