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
#include <stdio.h>
#include "mpx.h"

void
print_hex_string(char* buf, int len)
{
    int i;

    if (len==0)
    {
        printf("<empty string>");
        return;
    }

    for (i = 0; i < len; i++)
    {
        printf("%02x ", *((unsigned char *)buf + i));
        if ((i&0xf)==15) printf("\n   ");
    }
    if ((i&0xf))
        printf("\n");
}

void
mpx(char *m, void *buf_v, int len)
{
    char *buf = buf_v;
    printf("%s   MSG: %s\n   ", m, &buf[44] );
    print_hex_string(buf, len);
}
