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
    timer.schedule_after_msec(100);
    return 0;
}


void IGMP_Router::run_timer(Timer *t) {
    // Set interval
    timer.schedule_after_msec(100);

    virtual_timer = (virtual_timer + 1) % (query_interval_time * 10);

    // Create general query
    if (virtual_timer == 0) {
        WritablePacket *packet = igmpQuery.create_general(max_resp_time, query_interval_time, robustness_variable);
        output(0).push(packet);
    }

    // For every group
    for (Group *g : igmp_groups.groups) {
        if (g->timer != 0) {
            g->timer--;
        }
        Vector<int> rem;
        // For every client
        for (int i = 0; i < g->sources.size(); i++) {
            clientTimer &t = g->sources[i];
            if (t.time != 0) {
                t.time--;
            } else {
                // If client timer 0 and mode INCLUDE -> remove client
                if (g->mode == Include) {
                    rem.push_back(i);
                }
            }
        }
        for (int i : rem) {
            g->sources.erase(g->sources.begin() + i);
        }
        // If mode is EXCLUDE
        if (g->mode == Exclude && g->timer == 0) {
            // If clients present, switch to include.
            if (g->sources.size() != 0) {
                g->mode = Include;
                g->timer = g->grouptimer;
            }
            // If no clients present, remove record.
            else {
                igmp_groups.remove(g);
            }
        }
        // If mode INCLUDE and no clients -> remove group
        else if (g->mode == Include && g->sources.size() == 0) {
            igmp_groups.remove(g);
        }
    }
}

void IGMP_Router::push(int input, Packet *p) {
    WritablePacket *n = p->uniqueify();
    if (n->ip_header()->ip_p == 2) {
        // Cast to IP-header
        click_ip *iph = (click_ip *) n->data();
        uint16_t *option = (uint16_t * )(iph + 1);

        // Cast to find format type
        test *format = (struct test *) (option + 2);
        // Membership report
        if (format->type == 0x22) {
            // Cast to membership report
            IGMP_report *rm = (struct IGMP_report *) (option + 2);

            // Cast to grouprecords
            IGMP_grouprecord *grouprecord = (struct IGMP_grouprecord *) (rm + 1);
            for (int i = 0; i < rm->num_group_records; i++) {
                // Leave message
                if (grouprecord->type == IGMP_recordtype::CHANGE_TO_INCLUDE_MODE || grouprecord->type == IGMP_recordtype::MODE_IS_INCLUDE) {
                    Group *grp = igmp_groups.find(grouprecord->multicast_address);
                    // Group found
                    if (grp != nullptr) {
                        // Update timer
                        grp->timer = grp->grouptimer;
                        // Look for client
                        int it = findKey(grp->sources, iph->ip_src);
                        // client found
                        if (it != -1) {
                            grp->sources.erase(grp->sources.begin() + it);
                            // if no more client in group, remove group
                            if (grp->isEmpty()) {
                                igmp_groups.remove(grp);
                            }
                        }
                    }
                }
                    // Join message
                else if (grouprecord->type == IGMP_recordtype::CHANGE_TO_EXCLUDE_MODE || grouprecord->type == IGMP_recordtype::MODE_IS_EXCLUDE) {
                    Group *grp = igmp_groups.find(grouprecord->multicast_address);
                    // Group not found yet (no clients subscribed), create group
                    if (grp == nullptr) {
                        grp = new Group;
                        grp->grouptimer = max_resp_time;
                        grp->groupaddress = grouprecord->multicast_address;
                        grp->robustness = robustness_variable;
                        igmp_groups.add(grp);
                    }

                    // Reset timer and mode
                    grp->timer = grp->grouptimer;
                    grp->mode = Exclude;

                    int i = findKey(grp->sources, iph->ip_src);
                    // Check if client not already present in group
                    if (i == -1) {
                        clientTimer ct;
                        ct.address = iph->ip_src;
                        ct.time = grp->grouptimer;
                        grp->sources.push_back(ct);
                    } else {
                        grp->sources[i].time = grp->grouptimer;
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
    Group *grp = igmp_groups.find(n->ip_header()->ip_dst);
    if (grp != nullptr && (grp->mode == Exclude || grp->sources.size() > 0)) {
        output(0).push(n);
        return;
    }
    // Kills all remaining traffic
    p->kill();
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMP_Router)
