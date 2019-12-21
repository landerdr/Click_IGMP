#ifndef CLICK_UTILS_HH
#define CLICK_UTILS_HH

#include <click/element.hh>
#include <click/vector.cc>
#include "IGMP.hh"

CLICK_DECLS

struct clientTimer {
    in_addr address;
    unsigned int time;
};

struct packetTimer {
    Packet *packet;
    unsigned int time;
};

enum Filtermode {
    Include, Exclude
};

class Group {
public:
    // Used parameters
    in_addr groupaddress;
    Filtermode mode;
    Vector <clientTimer> sources;

    // present for igmp compliance
    unsigned int grouptimer;
    int robustness;
    int timer;

    bool isEmpty() {
        return sources.size() == 0;
    };

    bool isJoined_client() {
        return mode == Filtermode::Exclude;
    };
};

CLICK_ENDDECLS
#endif
