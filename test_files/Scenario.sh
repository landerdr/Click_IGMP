#!/bin/sh


telnet localhost 10003 <<EOF
write client21/igmp.join 224.4.4.4

EOF
sleep 10
telnet localhost 10005 <<EOF
write client31/igmp.join 224.4.4.4

EOF
sleep 10
telnet localhost 10006 <<EOF
write client32/igmp.join 224.4.4.4

EOF
sleep 10
telnet localhost 10003 <<EOF

write client21/igmp.leave 224.4.4.4
EOF
sleep 10
telnet localhost 10006 <<EOF
write client32/igmp.leave 224.4.4.4

EOF
sleep 10
telnet localhost 10004 <<EOF
write client22/igmp.join 224.4.4.4

EOF
sleep 10
telnet localhost 10005 <<EOF
write client31/igmp.leave 224.4.4.4

EOF
sleep 10
telnet localhost 10004 <<EOF
write client22/igmp.leave 224.4.4.4

EOF
