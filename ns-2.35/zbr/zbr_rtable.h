/*
Copyright (c) 1997, 1998 Carnegie Mellon University.  All Rights
Reserved. 

Permission to use, copy, modify, and distribute this
software and its documentation is hereby granted (including for
commercial or for-profit use), provided that both the copyright notice and this permission notice appear in all copies of the software, derivative works, or modified versions, and any portions thereof, and that both notices appear in supporting documentation, and that credit is given to Carnegie Mellon University in all publications reporting on direct or indirect use of this code or its derivatives.

ALL CODE, SOFTWARE, PROTOCOLS, AND ARCHITECTURES DEVELOPED BY THE CMU
MONARCH PROJECT ARE EXPERIMENTAL AND ARE KNOWN TO HAVE BUGS, SOME OF
WHICH MAY HAVE SERIOUS CONSEQUENCES. CARNEGIE MELLON PROVIDES THIS
SOFTWARE OR OTHER INTELLECTUAL PROPERTY IN ITS ``AS IS'' CONDITION,
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE OR
INTELLECTUAL PROPERTY, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.

Carnegie Mellon encourages (but does not require) users of this
software or intellectual property to return any improvements or
extensions that they make, and to grant Carnegie Mellon the rights to redistribute these changes without encumbrance.

The ZBR code developed by the CMU/MONARCH group was optimized and tuned by Samir Das and Mahesh Marina, University of Cincinnati. The work was partially done in Sun Microsystems.
*/


#ifndef __zbr_rtable_h__
#define __zbr_rtable_h__

#include <assert.h>
#include <sys/types.h>
#include <config.h>
#include <lib/bsd-list.h>
#include <scheduler.h>

#define CURRENT_TIME    Scheduler::instance().clock()
#define INFINITY2        0xff

/*
   ZBR Neighbor Cache Entry
*/
class ZBR_Neighbor {
       // friend class ZBR;
        friend class ZBR;		//<zheng: add>
        friend class zbr_rt_entry;
 public:
        ZBR_Neighbor(u_int32_t a) { nb_addr = a; }

 protected:
        LIST_ENTRY(ZBR_Neighbor) nb_link;
        nsaddr_t        nb_addr;
        double          nb_expire;      // ALLOWED_HELLO_LOSS * HELLO_INTERVAL
	unsigned char Rel;		// zhu: add -- 2-bit storage for Relationship (Neighbor, Parent, Child). 
	unsigned int NodeFailure;	//Yong modified on 10/10/03 for ZigBee routing
	unsigned int FailCounter;       //Yong modified on 11/13/03 for ZigBee routing
};

LIST_HEAD(zbr_ncache, ZBR_Neighbor);

/*
   ZBR Precursor list data structure
*/
//*<zheng: no precursor list>
class ZBR_Precursor {
        //friend class ZBR;
        friend class ZBR;		//<zheng: add>
        friend class zbr_rt_entry;
 public:
        ZBR_Precursor(u_int32_t a) { pc_addr = a; }

 protected:
        LIST_ENTRY(ZBR_Precursor) pc_link;
        nsaddr_t        pc_addr;	// precursor address
};

LIST_HEAD(zbr_precursors, ZBR_Precursor);
//</zheng: no precursor list>*/


/*
  Route Table Entry
*/

class zbr_rt_entry {
        friend class zbr_rtable;
        //friend class ZBR;
        friend class ZBR;			//<zheng: add>
	friend class LocalRepairTimer;
	friend class ZBR_LocalRepairTimer;	//<zheng: add>
 public:
        zbr_rt_entry();
        ~zbr_rt_entry();

        void            nb_insert(nsaddr_t id);
        ZBR_Neighbor*  nb_lookup(nsaddr_t id);

//*<zheng: no precursor list>
        void            pc_insert(nsaddr_t id);
        ZBR_Precursor* pc_lookup(nsaddr_t id);
        void 		pc_delete(nsaddr_t id);
        void 		pc_delete(void);
        bool 		pc_empty(void);
//</zheng: no precursor list>*/

        double          rt_req_timeout;         // when I can send another req
        u_int8_t        rt_req_cnt;             // number of route requests
	
 protected:
        LIST_ENTRY(zbr_rt_entry) rt_link;

        nsaddr_t        rt_dst;
	nsaddr_t	rt_ori_src;		//Yong modified on 10/10/03 for ZigBee routing
/*//<zheng: no sequence number>*/        u_int32_t       rt_seqno;
	/* u_int8_t 	rt_interface; */
        u_int16_t       rt_hops;       		// hop count
	int 		rt_last_hop_count;	// last valid hop count
        nsaddr_t        rt_nexthop;    		// next hop IP address
	/* list of precursors */ 
/*//<zheng: no precursor list>*/        zbr_precursors rt_pclist;
        double          rt_expire;     		// when entry expires
        u_int8_t        rt_flags;

	//NEW: Yong modified on 10/10/03 for ZigBee routing
        unsigned int	all_brct_flag;
	unsigned int	loc_repl_flag;
	//NEW: END

#define RTF_DOWN 0
#define RTF_UP 1
#define RTF_IN_REPAIR 2

        /*
         *  Must receive 4 errors within 3 seconds in order to mark
         *  the route down.
        u_int8_t        rt_errors;      // error count
        double          rt_error_time;
#define MAX_RT_ERROR            4       // errors
#define MAX_RT_ERROR_TIME       3       // seconds
         */

#define MAX_HISTORY	3
	double 		rt_disc_latency[MAX_HISTORY];
	char 		hist_indx;
        int 		rt_req_last_ttl;        // last ttl value used
	// last few route discovery latencies
	// double 		rt_length [MAX_HISTORY];
	// last few route lengths

        /*
         * a list of neighbors that are using this route.
         */
        zbr_ncache          rt_nblist;
};


/*
  The Routing Table
*/

class zbr_rtable {
 public:
	zbr_rtable() { LIST_INIT(&rthead); }

        zbr_rt_entry*       head() { return rthead.lh_first; }

        zbr_rt_entry*       rt_add(nsaddr_t id);
        void                 rt_delete(nsaddr_t id);
        zbr_rt_entry*       rt_lookup(nsaddr_t id);

 private:
        LIST_HEAD(zbr_rthead, zbr_rt_entry) rthead;
};

#endif /* _zbr__rtable_h__ */
