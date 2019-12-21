#ifndef CLICK_IGMP_IGMP_ROUTER_HH
#define CLICK_IGMP_IGMP_ROUTER_HH

#include <click/element.hh>
#include <click/vector.cc>
#include <click/bighashmap.cc>
#include "IGMP.hh"
#include <click/timer.hh>
#include "utils.hh"
#include "IGMP_Query.hh"

CLICK_DECLS



class IGMP_Router : public Element {
public:
    IGMP_Router();

    ~IGMP_Router();

    const char *class_name() const { return "IGMP_Router"; }

    const char *port_count() const { return "1/1"; }

    const char *processing() const { return PUSH; }

    int configure(Vector <String> &, ErrorHandler *);

    void push(int input, Packet *p);

    void run_timer(Timer *t);

private:
    unsigned max_resp_time = 100; // maximum response time | default = 100 (10s)
    unsigned query_interval_time = 125; // QQIC: time between queries | default = 125 (seconds)
    unsigned robustness_variable = 2; // robustness from system | default = 2 (amount of joins/leaves send)

    // Packet generator
    IGMP_Query igmpQuery;

    // Timers
    Timer timer;
    Vector<Group *> active_groups;

};

CLICK_ENDDECLS
#endif
