//
// Created by LanderDeRoeck on 21/12/2019.
//

#ifndef CLICK_IGMP_IGMP_REPORT_HH
#define CLICK_IGMP_IGMP_REPORT_HH

#include <click/element.hh>
#include "utils.hh"
#include "IGMP.hh"

CLICK_DECLS

class IGMP_Report {
private:
    IPAddress source;
public:
    IGMP_Report() {};

    IGMP_Report(IPAddress source) : source(source) {};

    WritablePacket *create_specific(IPAddress group, IGMP_recordtype mode) {
        int headerroom = sizeof(click_ip) + 4;
        int size = sizeof(IGMP_report) + sizeof(IGMP_grouprecord);
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
        iph->ip_dst = IGMP_REPORT_ADDRESS.in_addr();
        iph->ip_tos = 0xc0;
        // Setting router alert flag
        uint16_t *option = (uint16_t * )(iph + 1);
        *option = (uint16_t) htons(0x9404);

        // Calculating checksum
        iph->ip_sum = click_in_cksum((unsigned char *) iph, headerroom);

        IGMP_report *format = (struct IGMP_report *) (option + 2);
        *format = IGMP_report();
        format->num_group_records = htons(1);
        IGMP_grouprecord *gr = (struct IGMP_grouprecord *) (format + 1);
        gr->type = mode;
        gr->multicast_address = group.in_addr();

        // Set checksum
        format->cksum = click_in_cksum((unsigned char *) format, size);

        // Setting annotations
        packet->set_dst_ip_anno(IGMP_REPORT_ADDRESS);
        packet->set_ip_header(iph, headerroom);
        packet->set_anno_u32(0, IGMP_REPORT_ADDRESS);

        return packet;
    };

    WritablePacket *create_general(Vector<Group *> &groups) {
        int headerroom = sizeof(click_ip) + 4;
        int size = sizeof(IGMP_report) + sizeof(IGMP_grouprecord) * groups.size();
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
        iph->ip_dst = IGMP_REPORT_ADDRESS.in_addr();
        iph->ip_tos = 0xc0;
        // Setting router alert flag
        uint16_t *option = (uint16_t * )(iph + 1);
        *option = (uint16_t) htons(0x9404);

        // Calculating checksum
        iph->ip_sum = click_in_cksum((unsigned char *) iph, headerroom);

        IGMP_report *format = (struct IGMP_report *) (option + 2);
        *format = IGMP_report();
        format->num_group_records = htons(groups.size());

        // Create group reports
        IGMP_grouprecord *gr = (struct IGMP_grouprecord *) (format + 1);
        for (Group *g : groups) {
            if (g->mode == Include) {
                gr->type = IGMP_recordtype::MODE_IS_INCLUDE;
            } else{
                gr->type = IGMP_recordtype::MODE_IS_EXCLUDE;
            }
            gr->multicast_address = g->groupaddress;
            gr = (struct IGMP_grouprecord *) (gr + 1);
        }

        // Set checksum
        format->cksum = click_in_cksum((unsigned char *) format, size);

        // Setting annotations
        packet->set_dst_ip_anno(IGMP_REPORT_ADDRESS);
        packet->set_ip_header(iph, headerroom);
        packet->set_anno_u32(0, IGMP_REPORT_ADDRESS);

        return packet;
    }
};

CLICK_ENDDECLS

#endif //CLICK_IGMP_IGMP_REPORT_HH
