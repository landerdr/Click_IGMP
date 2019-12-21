#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include "IGMP_Router.hh"

CLICK_DECLS

int findKey(Vector <clientTimer> v, in_addr k) {
    for (int i = 0; i < v.size(); i++) {
        if (v[i].address == k) {
            return i;
        }
    }
    return -1;
};

int findKey(Vector<Group *> v, in_addr k) {
    for (int i = 0; i < v.size(); i++) {
        if (v[i]->groupaddress == k) {
            return i;
        }
    }
    return -1;
};

IGMP_Router::IGMP_Router() : timer(this) {}

IGMP_Router::~ IGMP_Router() {}

int IGMP_Router::configure(Vector <String> &conf, ErrorHandler *errh) {
    IPAddress src;
    // Read optional arguments
    if (Args(conf, this, errh).read_m("SRC", src).read("QQIC", query_interval_time).read("MRT", max_resp_time).read(
            "QRV", robustness_variable).complete() < 0)
        return -1;
    // Check correctness
    if (robustness_variable < 1 && robustness_variable >= 7) return errh->error("QRV should be between [1,6]");
    if (query_interval_time <= 0 && query_interval_time > 128) return errh->error("QQIC should be between [1,128]");
    if (max_resp_time <= 0) return errh->error("MRT should be larger than 0");

    // Warning for qrv = 1
    if (robustness_variable == 1) click_chatter("Warning: QRV should not be 1");

    // Successfully configured
    igmpQuery = IGMP_Query(src);

    // Initialise timer
    timer.initialize(this);
    timer.schedule_after_msec(1000);
    return 0;
}

void IGMP_Router::run_timer(Timer *t) {
    // Set interval
    timer.schedule_after_msec(query_interval_time * 1000);

    // Create general query
    WritablePacket *packet = igmpQuery.create_general(max_resp_time, query_interval_time, robustness_variable);
    output(0).push(packet);
    return;

//    for (int i = 0; i < active_groups.size(); i++) {
//        active_groups[i]->grouptimer--;
//        if (active_groups[i]->grouptimer == 0) {
//            active_groups[i]->grouptimer = 60;
//
//        }
//    }
}

void IGMP_Router::push(int input, Packet *p) {
    WritablePacket *n = p->uniqueify();
    if (n->ip_header()->ip_p == 2) {
        // Cast to IP-header
        click_ip *iph = (click_ip *) n->data();
        uint16_t *option = (uint16_t * )(iph + 1);

        // Cast to find format type
        test *format = (struct test *) (option + 2);
        // Membership query
        //if(format->type==0x11){
        //click_chatter("gp");
        //IGMP_query* gm = (struct IGMP_query*) (iph + 1);
        //}
        // Membership report
        if (format->type == 0x22) {
            // Cast to membership report
            IGMP_report *rm = (struct IGMP_report *) (option + 2);

            // Cast to grouprecords
            IGMP_grouprecord *grouprecord = (struct IGMP_grouprecord *) (rm + 1);
            for (int i = 0; i < rm->num_group_records; i++) {
                // Leave message
                if (grouprecord->type == IGMP_recordtype::CHANGE_TO_INCLUDE_MODE) {
                    int it = findKey(active_groups, grouprecord->multicast_address);
                    // group found
                    if (it != -1) {
                        Group *p = active_groups[it];
                        int it2 = findKey(p->sources, iph->ip_src);
                        // client found
                        if (it2 != -1) {
                            p->sources.erase(p->sources.begin() + it2);
                            // if no more client in group, remove group
                            if (p->isEmpty()) {
                                delete p;
                                active_groups.erase(active_groups.begin() + it);
                            }
                        }
                    }
                }
                    // Join message
                else if (grouprecord->type == IGMP_recordtype::CHANGE_TO_EXCLUDE_MODE) {
                    Group *grp;
                    int it = findKey(active_groups, grouprecord->multicast_address);
                    // Group not found yet (no clients subscribed)
                    if (it == -1) {
                        grp = new Group;
                        grp->groupaddress = grouprecord->multicast_address;
                        active_groups.push_back(grp);
                    }
                        // Use existing group
                    else {
                        grp = active_groups[it];
                    }

                    // Check if client not already present in group
                    if (findKey(grp->sources, iph->ip_src) == -1) {
                        clientTimer ct;
                        ct.address = iph->ip_src;
                        ct.time = 200;
                        grp->sources.push_back(ct);
                    }
                }

                // Sets pointer to next group-record
                grouprecord = (struct IGMP_grouprecord *) (grouprecord + 1);
            }
        }
        p->kill();
        return;
    }

    // Passes IGMP multicast traffic if a client subscribed
    int pos = findKey(active_groups, n->ip_header()->ip_dst);
    if (pos != -1) {
//        click_chatter("in groups to send");
        output(0).push(n);
        return;
    }
    // Kills all remaining traffic
    p->kill();
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMP_Router)
