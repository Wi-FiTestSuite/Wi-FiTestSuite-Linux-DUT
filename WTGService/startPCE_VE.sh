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
