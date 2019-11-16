#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include "report_sender.hh"
#include <string>
CLICK_DECLS
report_sender::report_sender()
{}

report_sender::~ report_sender()
{}

int report_sender::configure(Vector<String> &conf, ErrorHandler *errh) {
    if (Args(conf, this, errh).read_mp("SOURCE", src).read_mp("DESTINATION", dst).complete() < 0) return -1;
    return 0;
}

void report_sender::push(int  interface,Packet* p ){

}


CLICK_ENDDECLS
EXPORT_ELEMENT(report_sender)
