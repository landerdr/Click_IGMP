#ifndef CLICK_report_sender_HH
#define CLICK_report_sender_HH
#include <click/element.hh>
#include "IGMP.hh"
CLICK_DECLS

class report_sender : public Element {
	public:
    report_sender();
		~report_sender();
		
		const char *class_name() const	{ return "report_sender"; }
		const char *port_count() const	{ return "1/1"; }
		const char *processing() const	{ return PUSH; }
		int configure(Vector<String>&, ErrorHandler*);

        void push(int interface, Packet* p );

    private:
		static int join_group(const String &conf, Element* e, void* thunk, ErrorHandler* errh);
		static int leave_group(const String &conf, Element* e, void* thunk, ErrorHandler* errh);
		void add_handlers();

        IPAddress src;
        IPAddress dst;
        int id = 0;


};

CLICK_ENDDECLS
#endif
