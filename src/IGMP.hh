#ifndef CLICKNET_IGMP_H
#define CLICKNET_IGMP_H
#include <clicknet/ip.h>

struct IGMP_groupmessage {
    uint8_t	igmp_type;	// 0x11	
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
    struct in_addr[igmp_n] igmp_sources;
};

struct IGMP_reportmessage
{
    uint8_t	igmp_type;	// 0x22	
    uint8_t	igmp_resv1;		
    uint16_t	igmp_cksum;
    uint16_t    igmp_resv2;
    uint16_t    igmp_n;
    struct IGMP_grouprecord[igmp_n] igmp_grouprecords;
};

struct IGMP_grouprecord
{
    uint8_t	igmp_type;	
    uint8_t	igmp_auxdlen;		
    uint16_t	igmp_n;
    uint32_t    igmp_groupadress;
    struct in_addr[igmp_n] igmp_sources;
    //aux data
};


#endif
