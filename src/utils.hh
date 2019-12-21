#ifndef CLICK_UTILS_HH
#define CLICK_UTILS_HH

#include <click/element.hh>
#include <click/vector.cc>
#include "IGMP.hh"

CLICK_DECLS

struct clientTimer {
    in_addr address;
    unsigned int time;
};

struct packetTimer {
    Packet *packet;
    unsigned int time;
};

enum Filtermode {
    Include, Exclude
};

class Group {
public:
    // Used parameters
    in_addr groupaddress;
    Filtermode mode;
    Vector <clientTimer> sources;

    // present for igmp compliance
    unsigned int grouptimer;
    int robustness;
    int timer;

    bool isEmpty() {
        return sources.size() == 0;
    };

    bool isJoined_client() {
        return mode == Filtermode::Exclude;
    };
};

class Groups {
private:
    Vector<Group *> groups;
public:
    Groups() {};

    ~Groups() {
        for (Group *g : groups) {
            delete g;
        }
    };

    int findKey(in_addr &k) {
        for (int i = 0; i < groups.size(); i++) {
            if (groups[i]->groupaddress == k) {
                return i;
            }
        }
        return -1;
    };

    Group *find(in_addr &k) {
        int it = findKey(k);

        if (it == -1) return nullptr;

        return groups[it];
    };

    void remove(in_addr &k) {
        int it = findKey(k);

        if (it == -1) return;

        delete groups[it];
        groups.erase(groups.begin() + it);
    };

    void remove(Group *g) {
        for (int i = 0; i < groups.size(); i++) {
            if (groups[i] == g) {
                delete groups[i];
                groups.erase(groups.begin() + i);
            }
        }
    };

    void add(Group *g) {
        groups.push_back(g);
    };
};

CLICK_ENDDECLS
#endif
