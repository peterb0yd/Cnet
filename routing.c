#include <cnet.h>
#include <stdlib.h>
#include <string.h>

/* Project - Routing

  How it works: 
	- Frames originate from Atlanta 
	- Message is the name of destination
	- Frames sent along link until destination node is reached
*/

typedef struct {
    char        data[MAX_MESSAGE_SIZE];
} MSG;

typedef struct {
    size_t	len;       	// the length of the msg field only
    MSG 	msg;
    CnetAddr 	dest;
} FRAME;

#define FRAME_HEADER_SIZE  (sizeof(FRAME) - sizeof(MSG))
#define FRAME_SIZE(f)      (FRAME_HEADER_SIZE + f.len)

static  char       	*lastmsg;
static  size_t		lastlength		= 0;
int count = 0;

static void transmit_frame(MSG msg, size_t length, CnetAddr destaddr)		// sends a frame to physical layer
{
   	FRAME 	f;
	f.msg 		= msg;
	f.dest		= destaddr;		
	length		= sizeof(FRAME);
   	int link 	= 1;
	printf("\n DATA transmitted   :   msg = %s \n", f.msg.data);	

	if (nodeinfo.nodenumber+1 == 1) {	// send frame along correct link
		link = destaddr - 1;
		CHECK(CNET_write_physical_reliable(link, &f, &length));
	} else {
		link = 1;
		CHECK(CNET_write_physical_reliable(link, &f, &length));
	}
}

static EVENT_HANDLER(application_ready)				// reads application layer
{
	CnetAddr destaddr;
    	lastlength  = sizeof(MSG);

	MSG austin = {"Austin!"};	// messages = destination
	MSG atlanta = {"Atlanta!"};
	MSG newyork = {"New York!"};
	MSG la = {"Los Angelos!"};

    	CHECK(CNET_read_application(&destaddr, lastmsg, &lastlength));
	
	if (nodeinfo.nodenumber == 1) {		// only send data from node 1 (atlanta)
		count++;    	
		if (count == 1 || count % 15 == 0) {
			switch (destaddr) {
				case 1: 
					transmit_frame(austin, 0, destaddr);
					break;
				case 2: 
					transmit_frame(atlanta, 0, destaddr);
					break;
				case 3: 
					transmit_frame(newyork, 0, destaddr);
					break;
				case 4: 
					transmit_frame(la, 0, destaddr);
					break;
			}
		}
	}
}

static EVENT_HANDLER(physical_ready)				// reads physical layer
{
	FRAME	f;
    	size_t	len;
   	int     link;
    	len = sizeof(FRAME);

    	CHECK(CNET_read_physical(&link, &f, &len));

	if (nodeinfo.nodenumber+1 == f.dest) {	// if current node is destination node, frame received
		printf("\n\tDATA received! \t message = %s \n", f.msg.data);	
		printf("\t\tup to application\n");
	} else {
		if (nodeinfo.nodenumber+1 == 1) {	// middle node (links to all others) - 3 links possible
			link = f.dest - 1;
			CHECK(CNET_write_physical_reliable(link, &f, &len));
		} else {				// outside nodes (links to middle) - 1 link possible
			link = 1;
			CHECK(CNET_write_physical_reliable(link, &f, &len));
		}
	}
}


EVENT_HANDLER(reboot_node)
{
    	lastmsg	= calloc(1, sizeof(MSG));

    	CHECK(CNET_set_handler( EV_APPLICATIONREADY, application_ready, 0));
    	CHECK(CNET_set_handler( EV_PHYSICALREADY,    physical_ready, 0));

	// REQUEST THAT EV_TIMER1 OCCUR IN 1SEC, IGNORING THE RETURN VALUE
	CNET_start_timer(EV_TIMER1, 1000000, 0);
   	
	CNET_enable_application(ALLNODES);
}
