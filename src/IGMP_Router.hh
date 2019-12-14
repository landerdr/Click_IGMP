#ifndef CLICK_SIMPLEPULLELEMENT_HH
#define CLICK_SIMPLEPULLELEMENT_HH
#include <click/element.hh>
#include <click/vector.cc>
#include <click/bighashmap.cc>
#include "IGMP.hh"
#include <tuple>
#include <click/timer.hh>
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

    bool isEmpty(){
        return Include.size()==0 && Exclude.size()==0;

    };
};

class IGMP_Router : public Element {
	public:
        IGMP_Router();
		~IGMP_Router();
		
		const char *class_name() const	{ return "IGMP_Router"; }
		const char *port_count() const	{ return "1/2"; }
		const char *processing() const	{ return PUSH; }
		int configure(Vector<String>&, ErrorHandler*);
		
		void push(int input, Packet* p );
        void run_timer(Timer* t);
        
	private:
        Timer timer;
//		HashMap<int, Vector<int>> ports;
        Vector<Group*> active_groups;
};

CLICK_ENDDECLS
#endif
