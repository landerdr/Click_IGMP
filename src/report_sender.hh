#ifndef CLICK_REPORT_SENDER_HH
#define CLICK_REPORT_SENDER_HH

#include <click/element.hh>
#include <click/timer.hh>
#include <click/vector.cc>
#include "utils.hh"
#include "IGMP.hh"
#include "IGMP_Report.hh"

CLICK_DECLS

class report_sender : public Element {
public:
    report_sender();

    ~report_sender();

    const char *class_name() const { return "report_sender"; }

    const char *port_count() const { return "1/2"; }

    const char *processing() const { return PUSH; }

    int configure(Vector <String> &, ErrorHandler *);

    void push(int interface, Packet *p);

    void run_timer(Timer *t);

    Vector <packetTimer> mode_changes;

private:
    // Write handlers
    void add_handlers();

    static int join_group(const String &conf, Element *e, void *thunk, ErrorHandler *errh);

    static int leave_group(const String &conf, Element *e, void *thunk, ErrorHandler *errh);


    // Packet generator
    IGMP_Report igmpReport;

    // Group list
    Groups igmp_groups;
    // Timer list (queued for response)
    Vector <clientTimer> timers;

    // Source/Destination
    IPAddress src;
    IPAddress dst;

    // Timer
    Timer timer;

    // Default values
    unsigned max_resp_time = 100; // maximum response time | default = 100 (10s)
    unsigned query_interval_time = 125; // QQIC: time between queries | default = 125 (seconds)
    unsigned robustness_variable = 2; // robustness from system | default = 2 (amount of joins/leaves send)
    unsigned unsolicited_report_interval = 1; // max time in between sending join/leave reports | default = 1 (s)
};

CLICK_ENDDECLS
#endif
