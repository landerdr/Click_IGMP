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

unsigned decodeF(unsigned value) {
    if (value < 128) return value;

    unsigned exp = (value & 112) >> 4;
    unsigned mant = (value & 15);
    return (mant | 0x10) << (exp + 3);
};

report_sender::report_sender() : timer(this) {
}

report_sender::~report_sender() {
}

int report_sender::configure(Vector <String> &conf, ErrorHandler *errh) {
    if (Args(conf, this, errh).read_mp("SOURCE", src).read_mp("DESTINATION", dst).read("URP",
                                                                                       unsolicited_report_interval).complete() <
        0)
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
            Group *group = igmp_groups.find(groupadd);
            bool new_group = group == nullptr;
            if (new_group) {
                group = new Group;
                group->mode = Filtermode::Include;
                group->groupaddress = groupadd;
            }

            group->grouptimer = decodeF(gm->max_resp_code);
            if (gm->qrv != 0) {
                group->robustness = gm->qrv;
            } else {
                group->robustness = robustness_variable;
            }


            //set QQIC/QRV/MRT if general query
            if (groupadd == BROADCAST) {
                query_interval_time = gm->qqic;
                if (gm->qrv != 0) {
                    robustness_variable = gm->qrv;
                }
                max_resp_time = decodeF(gm->max_resp_code);
            }

            // If group specific and joined or general query, respond
            if (group->isJoined_client() || group->wait || groupadd == BROADCAST) {
                unsigned random = click_random(0, decodeF(gm->max_resp_code));
                group->wait = false;
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
            if (new_group && groupadd != BROADCAST) {
                igmp_groups.add(group);
            }
        }
    } else if (n->ip_header()->ip_p == 17) {
        Group *grp = igmp_groups.find(n->ip_header()->ip_dst);
        if (grp != nullptr && grp->mode == Exclude) {
            output(1).push(p);
            return;
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
            Vector < Group * > joined_groups = igmp_groups.get_joined_groups();

            // No groups joined -> ignore
            if (joined_groups.size() == 0) {
                return;
            }

            WritablePacket *packet = igmpReport.create_general(joined_groups);
            output(0).push(packet);

        } else {
            Group *g = igmp_groups.find(add);
            // Group exists and is joined
            if (g != nullptr && (g->isJoined_client() || g->wait)) {
                WritablePacket *packet;
                if (g->mode == Include) {
                    packet = igmpReport.create_specific(g->groupaddress, IGMP_recordtype::MODE_IS_INCLUDE);
                } else {
                    packet = igmpReport.create_specific(g->groupaddress, IGMP_recordtype::MODE_IS_EXCLUDE);
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
    Group *g = rs->igmp_groups.find(groupaddress.in_addr());

    if (groupaddress == IGMP_MULTICAST) {
        return -1;
    }

    // If group not found, create group and join
    if (g == nullptr) {
        g = new Group;
        g->groupaddress = groupaddress.in_addr();
        g->grouptimer = rs->max_resp_time;
        g->mode = Filtermode::Exclude;
        g->robustness = rs->robustness_variable;
        rs->igmp_groups.add(g);
    }
        // Group exists, but already joined
    else if (g->isJoined_client()) {
        return -1;
    }
        // Group exists, but not joined
    else {
        g->mode = Filtermode::Exclude;
    }

    for (int i = 0; i < rs->robustness_variable; i++) {
        // Create package
        WritablePacket *packet = rs->igmpReport.create_specific(groupaddress, IGMP_recordtype::CHANGE_TO_EXCLUDE_MODE);

        // Put in delay queue
        unsigned random = click_random(0, rs->unsolicited_report_interval * 10);
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
    Group *g = rs->igmp_groups.find(groupaddress.in_addr());

    // If not found or if group empty -> ignore
    if (g == nullptr || !g->isJoined_client() || groupaddress == IGMP_MULTICAST) {
        return -1;
    }

    // Set group to empty
    g->mode = Filtermode::Include;
    g->wait = true;

    for (int i = 0; i < rs->robustness_variable; i++) {
        // Create packet
        WritablePacket *packet = rs->igmpReport.create_specific(groupaddress, IGMP_recordtype::CHANGE_TO_INCLUDE_MODE);

        // Put in delay queue
        unsigned random = click_random(0, rs->unsolicited_report_interval * 10);
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
