# Click_IGMP
IGMP module for click library

[Click library](https://github.com/kohler/click)

## How to run:
We still require the reference-solution for the server-binary, as this one has yet to be implemented. Excecute in root of this project.

    > cp ./src /home/student/click/elements/local/igmp
    > sudo /home/student/click/scripts/setup.sh
    > cp ./library /home/student/click/scripts/library
    > bash ./library/start_click.sh

Make sure the reference solution is in the correct place, this has been written with the reference vm in mind.

To make a client join/leave a multicast address:

    > telnet localhost 10003
    $ write client21/igmp.join 224.4.4.4
    $ write client21/igmp.leave 224.4.4.4
