#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include "report_sender.hh"
CLICK_DECLS
report_sender::report_sender()
{}

report_sender::~ report_sender()
{}

int report_sender::configure(Vector<String> &conf, ErrorHandler *errh) {
    if (Args(conf, this, errh).read_mp("SOURCE", src).read_mp("DESTINATION", dst).complete() < 0) return -1;
    return 0;
}

void report_sender::push(int interface, Packet* p ){
    output(interface).push(p);
}

void report_sender::join_group(const String &conf, Element* e, void* thunk, ErrorHandler* errh)
{
    IPAddress groupaddress;
    if (Args(conf, this, errh).read_mp("GROUP", groupaddress).complete() < 0) return -1;

    int size = sizeof(IGMP_report) + sizeof(IGMP_grouprecord);
    WritablePacket *packet  = Packet::make(size);
    memset(packet->data(), 0, size);

    IGMP_report* format = (struct IGMP_report*) packet->data();
    *format = IGMP_report();
    format->num_group_records = htons(1);
    IGMP_grouprecord* gr = (struct IGMP_grouprecord*) (format + 1);
    gr->type = IGMP_recordtype::MODE_IS_EXCLUDE;
    gr->multicast_address = groupaddress.in_addr();
    
    format->cksum = click_in_cksum((unsigned char*)format, size);

    output(0).push(packet);
}

void leave_group(const String& conf)
{
    IPAddress groupaddress;
    if (Args(conf, this, errh).read_mp("GROUP", groupaddress).complete() < 0) return -1;

    int size = sizeof(IGMP_report) + sizeof(IGMP_grouprecord);
    WritablePacket *packet  = Packet::make(size);
    memset(packet->data(), 0, size);

    IGMP_report* format = (struct IGMP_report*) packet->data();
    *format = IGMP_report();
    format->num_group_records = htons(1);
    IGMP_grouprecord* gr = (struct IGMP_grouprecord*) (format + 1);
    gr->type = IGMP_recordtype::MODE_IS_INCLUDE;
    gr->multicast_address = groupaddress.in_addr();
    
    format->cksum = click_in_cksum((unsigned char*)format, size);

    output(0).push(packet);

}

CLICK_ENDDECLS
EXPORT_ELEMENT(report_sender)
