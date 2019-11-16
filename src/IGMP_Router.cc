#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include "IGMP_Router.hh"
CLICK_DECLS

int findKey(Vector<std::tuple<in_addr, unsigned int>> v, in_addr k)
{
    for (int i=0; i<v.size(); i++)
    {
        if (std::get<0>(v[i]) == k)
        {
            return i;
        }
    }
    return -1;
}

int findKey(Vector<std::tuple<in_addr, Group*>> v, in_addr k)
{
    for (int i=0; i<v.size(); i++)
    {
        if (std::get<0>(v[i]) == k)
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
            if(format->type==0x11){
                click_chatter("gp");
                IGMP_query* gm = (struct IGMP_query*) (iph + 1);
            }
            if(format->type==0x22){
                click_chatter("rp");
                IGMP_report* rm = (struct IGMP_report*) (iph + 1);
                IGMP_grouprecord* grouprecord =(struct IGMP_grouprecord*) (rm+1);
                if(grouprecord->type==IGMP_recordtype::MODE_IS_INCLUDE){
                    //leave
                    int it = findKey(active_groups,grouprecord->multicast_address);
                    if ( it!=-1){
                        Group* p = std::get<1>(active_groups[it]);
                        int it2 =findKey(p->Include,iph->ip_src);
                        if(it2!=-1){
                            p->Include.erase(p->Include.begin()+it2);
                            if(p->isEmpty()){
                                delete p;
                                active_groups.erase(active_groups.begin()+it);

                            }
                        }

                        it2 =findKey(p->Exclude,iph->ip_src);
                        if(it2!=-1){
                            p->Exclude.erase(p->Exclude.begin()+it2);

                            if(p->isEmpty()){
                                delete p;
                                active_groups.erase(active_groups.begin()+it);

                            }
                        }

                    }

                }

                if(grouprecord->type==IGMP_recordtype::MODE_IS_EXCLUDE){
                    //join

                        Group* grp;
                        int it = findKey(active_groups,grouprecord->multicast_address);

                        if (it== -1){
                            grp = new Group;

                        }
                        else{
                            grp = std::get<1>(active_groups[it]);

                        }

                        if (!(findKey(grp->Include,iph->ip_src)||findKey(grp->Exclude,iph->ip_src))){
                            grp->Include.push_back(std::make_tuple(iph->ip_src,200));
                        }






                }





            }
            p->kill();
        }

        int pos = findKey(active_groups, n->ip_header()->ip_dst);
        if (pos != -1)
        {
            Group* g = std::get<1>(active_groups[pos]);
            for (auto const& i : g->Include)
            {
                WritablePacket* k = p->uniqueify();
                k->ip_header()->ip_dst = std::get<0>(i);
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
