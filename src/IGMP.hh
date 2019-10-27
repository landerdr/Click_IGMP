#ifndef CLICKNET_IGMP_H
#define CLICKNET_IGMP_H
#include <clicknet/ip.h>

struct click_igmp {
    uint8_t	igmp_type;		
    uint8_t	igmp_code;		
    uint16_t	igmp_cksum;		
    uint32_t	igmp_groupadress;
    struct igmp_c
    {
        unsigned int igmp_resv : 4;
        unsigned int igmp_s : 1;
        unsigned int igmp_grv: 3;
    };
    uint8_t igmp_qqic;
    uint16_t igmp_n;
    struct in_addr igmp_source;
};

#endif
