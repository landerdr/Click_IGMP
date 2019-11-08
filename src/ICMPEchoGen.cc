#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include "ICMPEchoGen.hh"

CLICK_DECLS
ICMPEchoGen::ICMPEchoGen()
{}

ICMPEchoGen::~ ICMPEchoGen()
{}

int ICMPEchoGen::configure(Vector<String> &conf, ErrorHandler *errh) {
    if (Args(conf, this, errh).read_mp("SOURCE", src).read_mp("DESTINATION", dst).complete() < 0) return -1;
    return 0;
}

Packet* ICMPEchoGen::pull(int){
    //generate packet

    WritablePacket *packet = Packet::make(148);//is mij gezegd dat ik dit moest doen, anders werkte het niet
//    if(packet == 0) return click_chatter("cannot make packet");
//    memset(packet->data(),0, packet->length());
//    MyPacketFormat* format = (MyPacketFormat*) packet->data();
    click_ip* ip = (click_ip*)(packet->data());
    IGMP_groupmessage* ICMP = (IGMP_groupmessage*)(ip + 1);
    ip->ip_v = 4;
    ip->ip_hl = 5;
    ip->ip_tos = htons(0);
    ip->ip_len = htons(28);
    ip->ip_id = htons(id);
    ip->ip_off = 0;
    ip->ip_ttl = 255;
    ip->ip_p = 1;
    ip->ip_sum = 0;
    ip->ip_src = src.in_addr();
    ip->ip_dst = dst.in_addr();

    uint16_t checksum = click_in_cksum(packet->data(),20);
    ip->ip_sum = checksum;
    packet->set_ip_header(ip, sizeof(click_ip));

//    ICMP->type = 0;
//    ICMP->code = 0;
//    ICMP->checksum = 0;
//    ICMP->Identifier =0;
//    ICMP->Sequence_number =id;


    ICMP->igmp_type=0x11;
    ICMP->igmp_code=0;
    ICMP->igmp_cksum=0;
    ICMP->igmp_groupadress=0;
    ICMP->igmp_resv=0;
    ICMP->igmp_s=0;
    ICMP->igmp_qrv=0;


    ICMP->igmp_qqic=0;
    ICMP->igmp_n=1;
    ICMP->igmp_sources=Vector<in_addr>();


    uint16_t ICMPchecksum = click_in_cksum((const unsigned char*)ICMP,128);
    ICMP->igmp_cksum = ICMPchecksum;
    packet->set_ip_header(ip, sizeof(click_ip));
    packet->set_anno_u32(0, dst);

    id++;

    return packet;
}


CLICK_ENDDECLS
EXPORT_ELEMENT(ICMPEchoGen)
