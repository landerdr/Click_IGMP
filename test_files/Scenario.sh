#!/bin/sh


telnet localhost 10003 <<EOF
write client21/rs.join 224.4.4.4

EOF
sleep 10
telnet localhost 10005 <<EOF
write client31/rs.join 224.4.4.4

EOF
sleep 10
telnet localhost 10006 <<EOF
write client32/rs.join 224.4.4.4

EOF
sleep 10
telnet localhost 10003 <<EOF

write client21/rs.leave 224.4.4.4
EOF
sleep 10
telnet localhost 10006 <<EOF
write client32/rs.leave 224.4.4.4

EOF
sleep 10
telnet localhost 10004 <<EOF
write client22/rs.join 224.4.4.4

EOF
sleep 10
telnet localhost 10005 <<EOF
write client31/rs.leave 224.4.4.4

EOF
sleep 10
telnet localhost 10004 <<EOF
write client22/rs.leave 224.4.4.4

EOF
