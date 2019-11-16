#ifndef CLICKNET_IGMP_H
#define CLICKNET_IGMP_H
#include <clicknet/ip.h>
#include <click/vector.cc>

enum class IGMPTypes: uint8_t
{
    QUERY=0x11;
    REPORT=0x22;
};

/*
 * Query
 * 
*/
// Membership QUERY message
struct IGMP_query
{
    IGMPTypes   type=IGMPTypes::QUERY;
    uint8_t	    max_resp_code;          // timout value 10x s
    uint16_t	cksum;		
    in_addr	    multicast_address;      // MULTICAST ADRESS
    unsigned    resv  : 4;              // RESERVED
    unsigned    s     : 1;              // SUPRESS ROUTER FLAG
    unsigned    qrv   : 3;              // ROBUSTNESS VARIABLE
    uint8_t     qqic;                   // QUERY INTERVAL CODE
    uint16_t    num_sources=0;          // ALWAYS 0 in our implementation
    //Vector of sources                 // WE DON'T NEED THIS
};

/*
 * Report
 * 
*/

enum class IGMP_recordtype: unit8_t
{
    MODE_IS_INCLUDE = 1,
	MODE_IS_EXCLUDE = 2,
	CHANGE_TO_INCLUDE_MODE = 3,
	CHANGE_TO_EXCLUDE_MODE = 4,
	ALLOW_NEW_SOURCES = 5,
	BLOCK_OLD_SOURCES = 6
};

// Group membership report
struct IGMP_report
{
    IGMPTypes   type=IGMPTypes::REPORT;
    uint8_t	    resv1=0;
    uint16_t	cksum=0;
    uint16_t    resv2=0;
    uint16_t    num_group_records=0;
    // Vector of grouprecords
};
struct IGMP_grouprecord
{
    IGMP_recordtype	type;	
    uint8_t	    aux_data_len=0;     // ALWAYS 0	
    uint16_t	num_sources=0;     
    in_addr     multicast_address;  // MULTICAST ADDRESS
    // Vector of sources            // UNICAST ADDRESSES
    //aux data // Not used in our implementation
};



#endif
