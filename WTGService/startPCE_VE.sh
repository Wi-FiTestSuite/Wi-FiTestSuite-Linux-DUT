#
# Copyright (c) 2016 Wi-Fi Alliance
# 
# Permission to use, copy, modify, and/or distribute this software for any 
# purpose with or without fee is hereby granted, provided that the above 
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES 
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF 
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY 
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
# RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
# NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
# USE OR PERFORMANCE OF THIS SOFTWARE.
#

nice -n 20 /usr/bin/wfa_dut lo 8000 &
sleep 1
/usr/bin/wfa_ca eth0 9000 127.0.0.1 8000 &

sleep 2

nice -n 18 /usr/bin/wfa_dut lo 8001 &
sleep 1
/usr/bin/wfa_ca eth0 9001 127.0.0.1 8001 &

sleep 2

nice -n 16 /usr/bin/wfa_dut lo 8002 &
sleep 1
/usr/bin/wfa_ca eth0 9002 127.0.0.1 8002 &
