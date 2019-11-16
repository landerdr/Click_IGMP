#ifndef CLICKNET_IGMP_H
#define CLICKNET_IGMP_H
#include <clicknet/ip.h>
#include <click/vector.cc>


// Membership QUERY message
struct IGMP_groupmessage
{
    uint8_t	igmp_type;	        // 0x11	
    uint8_t	igmp_code;	        // timout value 10x s
    uint16_t	igmp_cksum;		
    in_addr	    igmp_groupadress;   // MULTICAST ADRESS
    unsigned    igmp_resv  : 4;     // RESERVED
    unsigned    igmp_s     : 1;     // SUPRESS ROUTER FLAG
    unsigned    igmp_qrv   : 3;     // ROBUSTNESS VARIABLE
    uint8_t igmp_qqic;          // QUERY INTERVAL CODE
    uint16_t    igmp_n;         // ALWAYS 0 in our implementation
    //Vector<in_addr> igmp_sources; // WE DON'T NEED THIS
};


struct IGMP_grouprecord
{
    uint8_t	record_type;        // 1: INCLUDE MODE, 2: EXCLUDE MODE, 3: CHANGE TO INCLUDE, 4: CHANGE TO EXCLUDE	
    uint8_t	igmp_auxdlen;	    // always 0	
    uint16_t	igmp_n;     
    in_addr     igmp_groupadress;   // MULTICAST ADDRESS
    //Vector<in_addr> igmp_sources;   // UNICAST ADDRESSES
    //aux data
};

// Group membership report
struct IGMP_reportmessage
{
    uint8_t	igmp_type;	    // 0x22
    uint8_t	igmp_resv1;     // RESERVED: 0
    uint16_t	igmp_cksum;
    uint16_t    igmp_resv2; // RESERVED: 0
    uint16_t    igmp_n;
    //Vector<IGMP_grouprecord> igmp_grouprecords;
};



#endif
