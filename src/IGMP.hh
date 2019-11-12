#ifndef CLICKNET_IGMP_H
#define CLICKNET_IGMP_H
#include <clicknet/ip.h>
#include <click/vector.cc>


struct IGMP_groupmessage
{
    uint8_t	igmp_type;	// 0x11	
    uint8_t	igmp_code;		
    uint16_t	igmp_cksum;		
    in_addr	igmp_groupadress;
    unsigned igmp_resv  : 4;
    unsigned igmp_s     : 1;
    unsigned igmp_qrv   : 3;
    uint8_t igmp_qqic;
    uint16_t igmp_n;
    Vector<in_addr> igmp_sources;
};


struct IGMP_grouprecord
{
    uint8_t	igmp_type;	
    uint8_t	igmp_auxdlen;	// 0	
    uint16_t	igmp_n;
    in_addr    igmp_groupadress;
    Vector<in_addr> igmp_sources;
    //aux data
};
struct IGMP_reportmessage
{
    uint8_t	igmp_type;	// 0x22
    uint8_t	igmp_resv1;
    uint16_t	igmp_cksum;
    uint16_t    igmp_resv2;
    uint16_t    igmp_n;
    Vector<IGMP_grouprecord> igmp_grouprecords;
};



#endif
