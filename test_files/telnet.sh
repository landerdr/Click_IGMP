#!/bin/sh


telnet localhost 10003 <<EOF
write client21/rs.join 224.4.4.4

EOF
sleep 60
telnet localhost 10003 <<EOF

write client21/rs.leave 224.4.4.4
EOF
