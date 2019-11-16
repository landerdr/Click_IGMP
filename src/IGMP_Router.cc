#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include "IGMP_Router.hh"
CLICK_DECLS

IGMP_Router::IGMP_Router()
{}

IGMP_Router::~ IGMP_Router()
{}

int IGMP_Router::configure(Vector<String> &conf, ErrorHandler *errh) {
//    if (Args(conf, this, errh).read_m("MAXPACKETSIZE", maxSize).complete() < 0) return -1;
//	if (maxSize <= 0) return errh->error("maxsize should be larger than 0");
	return 0;
}

void IGMP_Router::push(int input, Packet* p ){
	if (input == 0){
        WritablePacket* n = p->uniqueify();
        if(n->ip_header()->ip_p ==2){

            click_chatter("testing123");

            click_ip* iph = (click_ip*) n->data();
            test* format = (struct test*) (iph+1);
            if(format->type==0x11){
                click_chatter("gp");
                IGMP_query* gm = (struct IGMP_query*) (iph + 1);
            }
            if(format->type==0x22){
                click_chatter("rp");
                IGMP_report* rm = (struct IGMP_report*) (iph + 1);
            }

        }

        output(0).push(p);
	}
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMP_Router)
