#ifndef CLICK_ICMPEchoGen_HH
#define CLICK_ICMPEchoGen_HH
#include <click/element.hh>
#include "IGMP.hh"
CLICK_DECLS

class ICMPEchoGen : public Element {
	public:
        ICMPEchoGen();
		~ICMPEchoGen();
		
		const char *class_name() const	{ return "ICMPEchoGen"; }
		const char *port_count() const	{ return "1/1"; }
		const char *processing() const	{ return PULL; }
		int configure(Vector<String>&, ErrorHandler*);

        Packet* pull(int);

    private:
    IPAddress src;
    IPAddress dst;
        int id = 0;


};

CLICK_ENDDECLS
#endif
