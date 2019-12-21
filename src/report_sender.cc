#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include "report_sender.hh"

CLICK_DECLS

int findK(Vector <clientTimer> v, in_addr k) {
    for (int i = 0; i < v.size(); i++) {
        if (v[i].address == k) {
            return i;
        }
    }
    return -1;
};

int findK(Vector<Group *> v, in_addr k) {
    for (int i = 0; i < v.size(); i++) {
        if (v[i]->groupaddress == k) {
            return i;
        }
    }
    return -1;
};

Vector <String> split(const String &s, char delim) {
    Vector <String> v;
    String buf;

    for (char c : s) {
        if (c == delim) {
            if (buf.length() > 0)
                v.push_back(buf);
            buf = "";
        } else {
            buf += c;
        }
    }
    if (buf.length() > 0)
        v.push_back(buf);

    return v;
}

report_sender::report_sender() : timer(this) {
}

report_sender::~report_sender() {
}

int report_sender::configure(Vector <String> &conf, ErrorHandler *errh) {
    if (Args(conf, this, errh).read_mp("SOURCE", src).read_mp("DESTINATION", dst).complete() < 0)
        return -1;

    igmpReport = IGMP_Report(src);

    // Initialise timer
    timer.initialize(this);
    timer.schedule_after_msec(100);
    return 0;
}

void report_sender::push(int interface, Packet *p) {
    // output(interface).push(p);
    WritablePacket *n = p->uniqueify();

    // IGMP protocol traffic
    if (n->ip_header()->ip_p == 2) {
        click_ip *iph = (click_ip *) n->data();
        uint16_t *option = (uint16_t * )(iph + 1);

        // Cast to find format type
        test *format = (struct test *) (option + 2);
        // Query message
        if (format->type == 0x11) {
            IGMP_query *gm = (struct IGMP_query *) (option + 2);
            in_addr groupadd = gm->multicast_address;
            // Find group
            int it = findK(groups, groupadd);
            Group *group;
            if (it == -1) {
                group = new Group;
                group->mode = Filtermode::Include;
                group->groupaddress = groupadd;
            } else {
                group = groups[it];
            }

            group->grouptimer = gm->max_resp_code;
            group->robustness = gm->qrv;

            //set QQIC/QRV/MRT if general query
            if (groupadd == BROADCAST) {
                query_interval_time = gm->qqic;
                robustness_variable = gm->qrv;
                max_resp_time = gm->max_resp_code;
            }

            // If group specific and joined or general query, respond
            if (group->isJoined_client() || groupadd == BROADCAST) {
                unsigned random = click_random(0, gm->max_resp_code);
                // Queue response
                clientTimer ct;
                ct.time = random;
                ct.address = groupadd;
                // Check if response queued
                int timer_it = findK(timers, groupadd);
                if (timer_it == -1) {
                    timers.push_back(ct);
                } else {
                    // If new timer is faster, update
                    if (timers[timer_it].time > ct.time) {
                        timers[timer_it] = ct;
                    }
                }
            }

            // If group not yet added & not general group, add
            if (it == -1 && groupadd != BROADCAST) {
                groups.push_back(group);
            }
        }
    }

    // Kill all packets
    p->kill();
}

Vector<Packet *> pass1Timer(Vector <packetTimer> &v) {
    Vector < Packet * > retval;
    Vector<int> delval;
    for (int i = 0; i < v.size(); i++) {
        if (v[i].time == 0) {
            delval.push_back(i);
            retval.push_back(v[i].packet);
        } else {
            v[i].time--;
        }
    }
    for (int j = delval.size() - 1; j >= 0; j--) {
        v.erase(v.begin() + delval[j]);
    }
    return retval;
}

Vector <in_addr> pass1Timer(Vector <clientTimer> &v) {
    Vector <in_addr> retval;
    Vector<int> delval;
    for (int i = 0; i < v.size(); i++) {
        if (v[i].time == 0) {
            delval.push_back(i);
            retval.push_back(v[i].address);
        } else {
            v[i].time--;
        }
    }
    for (int j = delval.size() - 1; j >= 0; j--) {
        v.erase(v.begin() + delval[j]);
    }
    return retval;
}

void report_sender::run_timer(Timer *t) {
    // Schedule timer
    timer.schedule_after_msec(100);

    // Sending queued packages (join/leave)
    Vector < Packet * > send = pass1Timer(mode_changes);
    for (Packet *p : send) {
        output(0).push(p);
    }

    // Check virtual timers for groups
    Vector <in_addr> addrs = pass1Timer(timers);

    // For every expired timer
    for (in_addr add : addrs) {
        // If general query response
        if (add == BROADCAST) {
            // Get joined groups
            Vector < Group * > groupsin;
            for (Group *g : groups) {
                // Joined group
                if (g->isJoined_client()) {
                    groupsin.push_back(g);
                }
            }

            // No groups joined -> ignore
            if (groupsin.size() == 0) {
                return;
            }

            WritablePacket *packet = igmpReport.create_general(groupsin);
            output(0).push(packet);

        } else {
            int it = findK(groups, add);
            // Group exists and is joined
            if (it != -1 && groups[it]->isJoined_client()) {
                WritablePacket *packet;
                if (groups[it]->mode == Include) {
                    packet = igmpReport.create_specific(groups[it]->groupaddress, IGMP_recordtype::MODE_IS_INCLUDE);
                } else {
                    packet = igmpReport.create_specific(groups[it]->groupaddress, IGMP_recordtype::MODE_IS_EXCLUDE);
                }
                output(0).push(packet);
            }
        }
    }
}

int report_sender::join_group(const String &conf, Element *e, void *thunk, ErrorHandler *errh) {
    IPAddress groupaddress;
    Vector <String> parts = split(conf, ' ');
    if (parts.size() != 1)
        return -1;
    groupaddress = IPAddress(parts[0]);

    report_sender *rs = (report_sender *) e;

    // Find group
    int it = findK(rs->groups, groupaddress.in_addr());

    // If group not found, create group and join
    if (it == -1) {
        Group *g = new Group;
        g->groupaddress = groupaddress.in_addr();
        g->grouptimer = rs->max_resp_time;
        g->mode = Filtermode::Exclude;
        g->robustness = rs->robustness_variable;
        rs->groups.push_back(g);
    }
        // Group exists, but already joined
    else if (rs->groups[it]->isJoined_client()) {
        return -1;
    }
        // Group exists, but not joined
    else {
        rs->groups[it]->mode = Filtermode::Exclude;
    }

    for (int i = 0; i < rs->robustness_variable; i++) {
        // Create package
        int size = sizeof(click_ip) + 4 + sizeof(IGMP_report) + sizeof(IGMP_grouprecord);
        WritablePacket *packet = rs->igmpReport.create_specific(groupaddress, IGMP_recordtype::CHANGE_TO_EXCLUDE_MODE);

        // Put in delay queue
        unsigned random = click_random(0, rs->max_resp_time);
        packetTimer pt;
        pt.packet = packet;
        pt.time = random;
        rs->mode_changes.push_back(pt);
    }

    click_chatter("Joined group...");
    return 0;
}

int report_sender::leave_group(const String &conf, Element *e, void *thunk, ErrorHandler *errh) {
    IPAddress groupaddress;
    // Parse the input
    Vector <String> parts = split(conf, ' ');
    if (parts.size() != 1)
        return -1;
    groupaddress = IPAddress(parts[0]);
    report_sender *rs = (report_sender *) e;

    // Finds group
    int it = findK(rs->groups, groupaddress.in_addr());

    // If not found or if group empty -> ignore
    if (it == -1 || !rs->groups[it]->isJoined_client()) {
        return -1;
    }

    // Set group to empty
    rs->groups[it]->mode = Filtermode::Include;

    for (int i = 0; i < rs->robustness_variable; i++) {
        // Create packet
        WritablePacket *packet = rs->igmpReport.create_specific(groupaddress, IGMP_recordtype::CHANGE_TO_INCLUDE_MODE);

        // Put in delay queue
        unsigned random = click_random(0, rs->max_resp_time);
        packetTimer pt;
        pt.packet = packet;
        pt.time = random;
        rs->mode_changes.push_back(pt);
    }

    click_chatter("Left group...");
    return 0;
}

void report_sender::add_handlers() {
    add_write_handler("join", join_group, (void *) 0);
    add_write_handler("leave", leave_group, (void *) 0);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(report_sender)
