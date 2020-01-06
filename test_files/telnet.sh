#!/bin/sh


telnet localhost 10003 <<EOF
write client21/igmp.join 224.4.4.4
EOF
sleep 30
telnet localhost 10003 <<EOF

write client21/igmp.leave 224.4.4.4
EOF
