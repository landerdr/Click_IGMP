#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include "report_sender.hh"
#include <vector>
CLICK_DECLS

std::vector<String> split(const String& s, char delim) {
    std::vector<String> v;
    String buf;
    
    for (char c: s) {
        if (c == delim) {
            if (buf.length() > 0) v.push_back(buf);
            buf = "";
        } else {
            buf += c;
        }
    }
    if (buf.length() > 0) v.push_back(buf);
    
    return v;
}

report_sender::report_sender()
{}

report_sender::~ report_sender()
{}

int report_sender::configure(Vector<String> &conf, ErrorHandler *errh) {
    if (Args(conf, this, errh).read_mp("SOURCE", src).read_mp("DESTINATION", dst).complete() < 0) return -1;
    return 0;
}

void report_sender::push(int interface, Packet* p ){
    // output(interface).push(p);
    p.kill();
}

int report_sender::join_group(const String &conf, Element* e, void* thunk, ErrorHandler* errh)
{
    IPAddress groupaddress;
    std::vector<String> parts = split(conf, ' ');

    if (parts.size() != 2) return -1;
    if (parts[0] == String("GROUP")) {
		groupaddress = IPAddress(parts[1]);
	}  else {
		return -1;
	}

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

    e->output(0).push(q);
	return 0;
}

int report_sender::leave_group(const String &conf, Element* e, void* thunk, ErrorHandler* errh)
{
    IPAddress groupaddress;
    std::vector<String> parts = split(conf, ' ');
    if (parts.size() != 2) return -1;
    if (parts[0] == String("GROUP")) {
		groupaddress = IPAddress(parts[1]);
	}  else {
		return -1;
	}

    int size = sizeof(IGMP_report) + sizeof(IGMP_grouprecord);
    WritablePacket *packet  = Packet::make(size);
    memset(packet->data(), 0, size);
void report_sender::leaveGroup(const String& conf){

    id++;
    IPAddress grp = IPAddress("0.0.0.0")
    if (Args(conf, this, errh).read_mp("GROUP", grp).complete() < 0)
        return -1;


    WritablePacket *packet  = Packet::make(36);
    if (packet == 0) {
        click_chatter("cannot make packet!");
        return nullptr;
    }

    memset(packet->data(), 0, 36);

    click_ip* iph = (click_ip*) packet->data();
    iph->ip_v = 4;
    iph->ip_hl = 5;
    iph->ip_src = src.in_addr();
    iph->ip_dst = dst.in_addr();
    iph->ip_p = 2;
    iph->ip_ttl = 1;
    iph->ip_id = htons(id);
    iph->ip_len = htons(36);
    iph->ip_tos = 0;
    iph->ip_sum = click_in_cksum((unsigned char*) iph, sizeof(click_ip));

    IGMP_report* format = (struct IGMP_report*) (iph + 1);
    *format = IGMP_report();
    format->num_group_records = htons(1);
    IGMP_grouprecord* gr = (struct IGMP_grouprecord*) (format + 1);
    gr->type = IGMP_recordtype::MODE_IS_INCLUDE;
    gr->multicast_address = groupaddress.in_addr();
    
    format->cksum = click_in_cksum((unsigned char*)format, size);

    e->output(0).push(packet);

	return 0;

}

void report_sender::add_handlers()
{
	add_write_handler("join", join_group, (void *) 0);
	add_write_handler("leave", leave_group, (void *) 0);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(report_sender)
