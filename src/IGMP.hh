#ifndef CLICKNET_IGMP_HH
#define CLICKNET_IGMP_HH

#include <clicknet/ip.h>

#define BROADCAST IPAddress("0.0.0.0")
#define IGMP_MULTICAST IPAddress("224.0.0.1")
#define IGMP_REPORT_ADDRESS IPAddress("224.0.0.22")

enum class IGMPTypes : uint8_t {
    QUERY = 0x11,
    REPORT = 0x22
};

enum class IGMP_recordtype : uint8_t {
    MODE_IS_INCLUDE = 1,
    MODE_IS_EXCLUDE = 2,
    CHANGE_TO_INCLUDE_MODE = 3,
    CHANGE_TO_EXCLUDE_MODE = 4
};

/*
 * Query
 * 
*/
// Membership QUERY message
struct IGMP_query {
    IGMPTypes type = IGMPTypes::QUERY;
    uint8_t max_resp_code; // timout value 10x s
    uint16_t cksum = 0;
    in_addr multicast_address; // MULTICAST ADRESS
    unsigned resv : 4;         // RESERVED
    unsigned s : 1;            // SUPRESS ROUTER FLAG
    unsigned qrv : 3;          // ROBUSTNESS VARIABLE
    uint8_t qqic;              // QUERY INTERVAL CODE
    uint16_t num_sources = 0;  // ALWAYS 0 in our implementation
    //Vector of sources                 // WE DON'T NEED THIS
};

/*
 * Report
 * 
*/
// Group membership report
struct IGMP_report {
    IGMPTypes type = IGMPTypes::REPORT;
    uint8_t resv1 = 0;
    uint16_t cksum = 0;
    uint16_t resv2 = 0;
    uint16_t num_group_records = 0;
    // Vector of grouprecords
};
struct IGMP_grouprecord {
    IGMP_recordtype type;
    uint8_t aux_data_len = 0; // ALWAYS 0
    uint16_t num_sources = 0;
    in_addr multicast_address; // MULTICAST ADDRESS
    // Vector of sources            // UNICAST ADDRESSES
    //aux data // Not used in our implementation
};

struct test {
    uint8_t type;
};

#endif
