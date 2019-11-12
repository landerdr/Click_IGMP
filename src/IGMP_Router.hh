#ifndef CLICK_SIMPLEPULLELEMENT_HH
#define CLICK_SIMPLEPULLELEMENT_HH
#include <click/element.hh>
#include <click/vector.cc>
#include <click/bighashmap.cc>
CLICK_DECLS

struct test {
    uint8_t type;
    String data;
};
enum Filtermode {Include,Exclude};
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
        in_addr addr;
        HashMap<in_addr,unsigned int> Include;
        HashMap<in_addr,unsigned int> Exclude;
        unsigned int grouptimer;
        Filtermode mode;
        int robustness;

};

CLICK_ENDDECLS
#endif
