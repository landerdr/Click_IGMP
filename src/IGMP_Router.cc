#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include "IGMP_Router.hh"
CLICK_DECLS

int findKey(Vector<std::tuple<in_addr,unsigned int>> v, in_addr k)
{
    for (int i=0; i<v.size(); i++)
    {
        if (v[i]->first == k)
        {
            return i;
        }
    }
    return -1;
}

IGMP_Router::IGMP_Router()
{}

IGMP_Router::~ IGMP_Router()
{}

int IGMP_Router::configure(Vector<String> &conf, ErrorHandler *errh) {
//    if (Args(conf, this, errh).read_m("MAXPACKETSIZE", maxSize).complete() < 0) return -1;
//	if (maxSize <= 0) return errh->error("maxsize should be larger than 0");
	return 0;
}

void IGMP_Router::push(int input, Packet* p ){
	if (input == 0){
        WritablePacket* n = p->uniqueify();
        if(n->ip_header()->ip_p == 2){

            click_chatter("testing123");

            click_ip* iph = (click_ip*) n->data();
            test* format = (struct test*) (iph+1);
            if(format->type==IGMPTypes::QUERY){
                click_chatter("gp");
                IGMP_query* gm = (struct IGMP_query*) (iph + 1);
            }
            if(format->type==IGMPTypes::REPORT){
                click_chatter("rp");
                IGMP_report* rm = (struct IGMP_report*) (iph + 1);
                IGMP_grouprecord* grouprecord =(struct IGMP_grouprecord*) (rm+1);
                if(grouprecord->type==IGMP_recordtype::MODE_IS_INCLUDE){
                    //leave
                    auto it = active_groups.find(grouprecord->multicast_address);
                    if (it!= nullptr){
                        Group* p = active_groups[grouprecord->multicast_address];
                        auto it2 =p->Include.find(iph->ip_src);
                        if(it2!=p->Include.end()){
                            p->Include.erase(it2);
                            if(p->isEmpty()){
                                delete p;
                                active_groups.erase(grouprecord->multicast_address);

                            }
                        }

                        auto it2 =p->Exclude.find(iph->ip_src);
                        if(it2!=p->Exclude.end()){
                            p->Exclude.erase(it2);

                            if(p->isEmpty()){
                                delete p;
                                active_groups.erase(grouprecord->multicast_address);

                            }
                        }

                    }

                }

                if(grouprecord->type==IGMP_recordtype::MODE_IS_EXCLUDE){
                    //join

                        Group* grp;
                        auto it = active_groups.find(grouprecord->multicast_address);

                        if (it== nullptr){
                            grp = new Group;

                        }
                        else{
                            grp = active_groups[multicast_address];

                        }

                        grp->Include[iph->ip_src] = 200;





                }





            }
            p->kill();
        }

        auto pos = active_groups.find(n->ip_header()->ip_dst);
        if (pos != active_groups.end())
        {
            Group* g = pos->second;
            for (auto const& i : g->Include)
            {
                WritablePacket* k = p->uniqueify();
                k->ip_header()->ip_dst = i.first;
                output(0).push(k);
            }
            p->kill();
            return;
        }
        output(0).push(p);
	}
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMP_Router)
