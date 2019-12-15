#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include "IGMP_Router.hh"
CLICK_DECLS

int findKey(Vector<clientTimer> v, in_addr k)
{
    for (int i=0; i<v.size(); i++)
    {
        if (v[i].address == k)
        {
            return i;
        }
    }
    return -1;
};

int findKey(Vector<Group*> v, in_addr k)
{
    for (int i=0; i<v.size(); i++)
    {
        if (v[i]->groupaddress == k)
        {
            return i;
        }
    }
    return -1;
};
IGMP_Router::IGMP_Router() : timer(this)
{}

IGMP_Router::~ IGMP_Router()
{}

int IGMP_Router::configure(Vector<String> &conf, ErrorHandler *errh) {
//    if (Args(conf, this, errh).read_m("MAXPACKETSIZE", maxSize).complete() < 0) return -1;
//	if (maxSize <= 0) return errh->error("maxsize should be larger than 0");
	timer.initialize(this);
	timer.schedule_after_msec(1000);
	return 0;
}

void IGMP_Router::run_timer(Timer* t)
{
    timer.schedule_after_msec(20000);

    int size = sizeof(IGMP_query);
    WritablePacket *packet = Packet::make(size);
    memset(packet->data(), 0, size);

    IGMP_query* format = (struct IGMP_query*) packet->data();
    *format = IGMP_query();
    format->max_resp_code = 20; // timout value in cs: 20 -> 2s
    format->qqic = 60; // query interval (s)
    format->multicast_address = IPAddress("0.0.0.0").in_addr();
    
    format->cksum = click_in_cksum((unsigned char*)format, size);

    output(1).push(packet);
	return;

    for (int i=0; i<active_groups.size(); i++)
    {
        active_groups[i]->grouptimer--;
        if (active_groups[i]->grouptimer == 0)
        {
            active_groups[i]->grouptimer = 60;
            
        }
    }
}

void IGMP_Router::push(int input, Packet* p ){
    WritablePacket* n = p->uniqueify();
    if(n->ip_header()->ip_p == 2){

        //click_chatter("testing123");

        click_ip* iph = (click_ip*) n->data();
        test* format = (struct test*) (iph+1);
        //if(format->type==0x11){
            //click_chatter("gp");
            //IGMP_query* gm = (struct IGMP_query*) (iph + 1);
        //}
        if(format->type==0x22){
            //click_chatter("rp");
            IGMP_report* rm = (struct IGMP_report*) (iph + 1);
            IGMP_grouprecord* grouprecord =(struct IGMP_grouprecord*) (rm+1);
            if(grouprecord->type==IGMP_recordtype::MODE_IS_INCLUDE){
                //leave
                int it = findKey(active_groups,grouprecord->multicast_address);
                if ( it!=-1){
                    Group* p = active_groups[it];
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
                click_chatter("join");
                Group* grp;
                int it = findKey(active_groups,grouprecord->multicast_address);

                if (it== -1){
                    click_chatter("renewed");
                    grp = new Group;
		    grp-> groupaddress=grouprecord->multicast_address;
                    active_groups.push_back(grp);
                }
                else {
                    click_chatter("not renewed");
                    grp = active_groups[it];
                }
                if ((findKey(grp->Include,iph->ip_src)==-1 && findKey(grp->Exclude,iph->ip_src)==-1)){
                    click_chatter("not in include/exclude");
		    clientTimer ct;
		    ct.address = iph->ip_src;
		    ct.time = 200;
                    grp->Include.push_back(ct);
                }
            }
        }
        p->kill();
        return;	
    }
    int pos = findKey(active_groups, n->ip_header()->ip_dst);
    if (pos != -1)
    {
        click_chatter("in groups to send");
        output(0).push(n);
        return;
    }
    p->kill();
	
}

CLICK_ENDDECLS
EXPORT_ELEMENT(IGMP_Router)
