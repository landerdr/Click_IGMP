#ifndef CLICK_IGMPEchoGen_HH
#define CLICK_IGMPEchoGen_HH
#include <click/element.hh>
#include "IGMP.hh"
CLICK_DECLS

class IGMPEchoGen : public Element {
	public:
        IGMPEchoGen();
		~IGMPEchoGen();
		
		const char *class_name() const	{ return "IGMPEchoGen"; }
		const char *port_count() const	{ return "0/1"; }
		const char *processing() const	{ return PULL; }
		int configure(Vector<String>&, ErrorHandler*);

        Packet* pull(int);

    private:
        IPAddress src;
        IPAddress dst;
        IPAddress grp;
        int id = 0;


};

CLICK_ENDDECLS
#endif
