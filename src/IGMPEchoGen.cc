#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include "IGMPEchoGen.hh"

CLICK_DECLS
IGMPEchoGen::IGMPEchoGen()
{}

IGMPEchoGen::~ IGMPEchoGen()
{}

int IGMPEchoGen::configure(Vector<String> &conf, ErrorHandler *errh) {
    if (Args(conf, this, errh).read_mp("SOURCE", src).read_mp("DESTINATION", dst).read_mp("GROUP", grp).complete() < 0) return -1;
    return 0;
}

Packet* IGMPEchoGen::pull(int){
    //generate packet
    id++;

    // WritablePacket *packet  = Packet::make(32);
    // if (packet == 0) {
    //     click_chatter("cannot make packet!");
    //     return nullptr;
    // }
    
    // memset(packet->data(), 0, 32);

    // click_ip* iph = (click_ip*) packet->data();
    // iph->ip_v = 4;
    // iph->ip_hl = 5;
    // iph->ip_src = src.in_addr();
    // iph->ip_dst = dst.in_addr();
    // iph->ip_p = 2;
    // iph->ip_ttl = 1;
    // iph->ip_id = htons(id);
    // iph->ip_len = htons(32);
    // iph->ip_tos = 0;
    // iph->ip_sum = click_in_cksum((unsigned char*) iph, sizeof(click_ip));

    // IGMP_query* format = (struct IGMP_query*) (iph + 1); 
    // format->max_resp_code=0x64;
    // format->multicast_address=grp.in_addr();

    // format->cksum = click_in_cksum((unsigned char*)format, sizeof(IGMP_query));

    // packet->set_dst_ip_anno(dst); 
    // packet->set_ip_header(iph, sizeof(click_ip));
    // packet->set_anno_u32(0, dst);µ

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
    gr->multicast_address = grp.in_addr();

    format->cksum = click_in_cksum((unsigned char*)format, sizeof(IGMP_report) + sizeof(IGMP_grouprecord)*format->num_group_records);

    packet->set_dst_ip_anno(dst); 
    packet->set_ip_header(iph, sizeof(click_ip));
    packet->set_anno_u32(0, dst);

    return packet;
}


CLICK_ENDDECLS
EXPORT_ELEMENT(IGMPEchoGen)
