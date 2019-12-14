#ifndef CLICK_utils_HH
#define CLICK_utils_HH
#include <click/element.hh>
#include <click/vector.cc>
#include "IGMP.hh"
CLICK_DECLS

struct clientTimer{
	in_addr address;
	unsigned int time;
};

enum Filtermode {Include,Exclude};

class Group {
public:
    Vector<clientTimer> Include;
    Vector<clientTimer> Exclude;
    in_addr groupaddress;
    unsigned int grouptimer;
    Filtermode mode;
    int robustness;
    int timer;

    bool isEmpty(){
        return Include.size()==0 && Exclude.size()==0;

    };
};








CLICK_ENDDECLS
#endif
