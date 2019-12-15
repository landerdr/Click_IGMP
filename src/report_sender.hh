#ifndef CLICK_report_sender_HH
#define CLICK_report_sender_HH

#include <click/element.hh>
#include <click/timer.hh>
#include <click/vector.cc>
#include "utils.hh"
#include "IGMP.hh"

CLICK_DECLS

class report_sender : public Element {
public:
    report_sender();

    ~report_sender();

    const char *class_name() const { return "report_sender"; }

    const char *port_count() const { return "1/1"; }

    const char *processing() const { return PUSH; }

    int configure(Vector <String> &, ErrorHandler *);

    void push(int interface, Packet *p);

    Vector<Group *> groups;
    Vector <clientTimer> timers;
    IPAddress src;

    void run_timer(Timer *t);

    Vector <packetTimer> mode_changes;

private:
    static int join_group(const String &conf, Element *e, void *thunk, ErrorHandler *errh);

    static int leave_group(const String &conf, Element *e, void *thunk, ErrorHandler *errh);

    void add_handlers();

    IPAddress dst;
    Timer timer;
    unsigned max_resp_time = 100; // maximum response time | default = 100 (10s)
    unsigned query_interval_time = 125; // QQIC: time between queries | default = 125 (seconds)
    unsigned robustness_variable = 2; // robustness from system | default = 2 (amount of joins/leaves send)
};

CLICK_ENDDECLS
#endif
