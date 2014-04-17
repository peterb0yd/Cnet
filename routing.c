#include <cnet.h>

/*  This is a simple cnet protocol source file which demonstrates the use
    of timer events in cnet.

    The function reboot_node() will first be called for each node.
    reboot_node() informs cnet that the node is interested in the EV_TIMER1
    event and that cnet should call timeouts() each time EV_TIMER1 occurs.
    Finally, reboot_node() requests that EV_TIMER1 occurs in 1000000us (1sec).

    When EV_TIMER1 occurs, cnet will call timeouts(), which will print
    either tick or tock. As each timer event occurs only once, timeouts()
    must finally reschedule EV_TIMER1 to occur again in another 1000000us.
 */

typedef struct {
    char msg[5];
} Envelope;

static EVENT_HANDLER(timeouts)  {

	Envelope env = {"WAX"};
	size_t  len = 1;

	// SEND MESSAGE
	CHECK(CNET_write_physical_reliable(1,&env.msg,&len));

	// RESCHEDULE EV_TIMER1 TO OCCUR AGAIN IN 1SEC
   	CNET_start_timer(EV_TIMER1, 1000000, 0);
}

static void send_reliable()
{
	Envelope env = {"BBB"};
	size_t  len = 1;

    	printf("test");

    	printf("sending");
    	CHECK(CNET_write_physical_reliable(1, &env.msg, &len));
}

static void physical_ready()
{
	Envelope env = {"CCC"};
	size_t  len = 1;
	int link = 3;

   	CHECK(CNET_read_physical(&link, &env.msg, &len));

   	printf("%s\n",env.msg);

}


EVENT_HANDLER(reboot_node)
{
 	// SET HANDLERS
       	CHECK(CNET_set_handler(EV_PHYSICALREADY, physical_ready, 0));

   	CHECK(CNET_set_handler(EV_DEBUG1, send_reliable, 0));

	//  INDICATE THAT WE ARE INTERESTED IN THE EV_TIMER1 EVENT
	CHECK(CNET_set_handler(EV_TIMER1, timeouts, 0));

	//  REQUEST THAT EV_TIMER1 OCCUR IN 1SEC, IGNORING THE RETURN VALUE
	CNET_start_timer(EV_TIMER1, 1000000, 0);
}

