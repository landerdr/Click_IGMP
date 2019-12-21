//
// Created by LanderDeRoeck on 21/12/2019.
//

#ifndef CLICK_IGMP_IGMP_QUERY_HH
#define CLICK_IGMP_IGMP_QUERY_HH

#include <click/element.hh>
#include "IGMP.hh"

CLICK_DECLS

class IGMP_Query {
private:
    IPAddress source;
public:
    IGMP_Query() {};

    IGMP_Query(IPAddress source) : source(source) {};

    WritablePacket *create_general(unsigned max_resp_time, unsigned query_interval_time, unsigned robustness_variable) {
        return create_specific(IGMP_MULTICAST, max_resp_time, query_interval_time, robustness_variable);
    };

    WritablePacket *create_specific(IPAddress destination, unsigned max_resp_time, unsigned query_interval_time,
                                    unsigned robustness_variable) {
        int headerroom = sizeof(click_ip) + 4;
        int size = sizeof(IGMP_query);
        WritablePacket *packet = Packet::make(headerroom + size);
        memset(packet->data(), 0, headerroom + size);

        click_ip *iph = (click_ip * )(packet->data());
        // Set ip header
        iph->ip_v = 4;
        iph->ip_hl = 6;
        iph->ip_len = htons(packet->length());
        iph->ip_id = 0;
        iph->ip_off = 0;
        iph->ip_ttl = 1;
        iph->ip_p = 2;
        iph->ip_sum = 0;
        iph->ip_src = source.in_addr();
        iph->ip_dst = destination.in_addr();
        iph->ip_tos = 0xc0;
        // Setting router alert flag
        uint16_t *option = (uint16_t * )(iph + 1);
        *option = (uint16_t) htons(0x9404);

        // Calculating checksum
        iph->ip_sum = click_in_cksum((unsigned char *) iph, headerroom);

        IGMP_query *format = (struct IGMP_query *) (option + 2);
        *format = IGMP_query();
        format->max_resp_code = max_resp_time;
        format->qqic = query_interval_time;
        format->qrv = robustness_variable;
        if (destination == IGMP_MULTICAST) {
            format->multicast_address = BROADCAST;
        } else {
            format->multicast_address = destination.in_addr();
        }

        // Set checksum
        format->cksum = click_in_cksum((unsigned char *) format, size);

        // Setting annotations
        packet->set_dst_ip_anno(destination);
        packet->set_ip_header(iph, headerroom);
        packet->set_anno_u32(0, destination);

        return packet;
    };
};

CLICK_ENDDECLS
#endif //CLICK_IGMP_IGMP_QUERY_HH
