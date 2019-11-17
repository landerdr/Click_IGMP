#!/bin/sh

cd /home/student/click-reference/solution/

/home/student/click/userlevel/click /home/student/click/scripts/glue.click &

sleep 1

/home/student/click/userlevel/click -p 10001 /home/student/click/scripts/router.click &
#./router.bin &
./server.bin &   # port 10002
#./client21.bin &
#./client22.bin &
#./client31.bin &
#./client32.bin
/home/student/click/userlevel/click -p 10003 /home/student/click/scripts/client21.click &
/home/student/click/userlevel/click -p 10004 /home/student/click/scripts/client22.click &
/home/student/click/userlevel/click -p 10005 /home/student/click/scripts/client31.click &
/home/student/click/userlevel/click -p 10006 /home/student/click/scripts/client32.click &


wait

