#ifndef CLICK_SIMPLEPULLELEMENT_HH
#define CLICK_SIMPLEPULLELEMENT_HH
#include <click/element.hh>
#include <click/vector.cc>
#include <click/bighashmap.cc>
#include "IGMP.hh"
CLICK_DECLS

struct test {
    uint8_t type;
};

enum Filtermode {Include,Exclude};

class Group {
public:
    HashMap<in_addr,unsigned int> Include;
    HashMap<in_addr,unsigned int> Exclude;
    unsigned int grouptimer;
    Filtermode mode;
    int robustness;

    bool isEmpty(){
        return Include.size()==0 && Exclude.size()==0

    }


};

class IGMP_Router : public Element {
	public:
        IGMP_Router();
		~IGMP_Router();
		
		const char *class_name() const	{ return "IGMP_Router"; }
		const char *port_count() const	{ return "1/1"; }
		const char *processing() const	{ return PUSH; }
		int configure(Vector<String>&, ErrorHandler*);
		
		void push(int input, Packet* p );
	private:
//		HashMap<int, Vector<int>> ports;
        HashMap<in_addr, Group*> active_groups;

};

CLICK_ENDDECLS
#endif
