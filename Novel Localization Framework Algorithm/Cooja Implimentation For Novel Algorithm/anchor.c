#include "contiki.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "lib/random.h"
#include "net/rime.h"
#include <stdio.h>
#include "node-id.h"
#include "dev/leds.h"
#include "node-id.h"
#include <math.h>
#include "dev/cc2420.h"
#include "dev/cc2420_const.h"


#define anchors_num 4
#define MAX_NEIGHBORS 16
/* This structure holds information about neighbors. */
struct neighbor {
struct neighbor *next;
rimeaddr_t addr;
};
rimeaddr_t next_hop; uint8_t nbr_hop;

/* This structure holds information about database. */
struct database {
uint8_t  id;
float  x;
float  y;
uint8_t  hop_count;
} database;

struct database currently_information;
struct database *received_data_mote;
struct database routing_table[anchors_num-1];
int counter=0;


struct exploit {
uint8_t id;
float x;
float y;
} exploit;
struct exploit *received_exploit;

static struct broadcast_conn broadcast;
static struct unicast_conn unicast;
/*---------------------------------------------------------------------------*/
/* We first declare our processes. */
PROCESS(broadcast_process, "Broadcast process");
PROCESS(flooding_process, "Flooding Process");
PROCESS(unicast_process, "Unicast process");
PROCESS(blink_process, "LED blink process");
PROCESS(display_process, "Display process");
/* The AUTOSTART_PROCESSES() definition specifices what processes to
   start when this module is loaded. We put both our processes
   there. */
AUTOSTART_PROCESSES(&broadcast_process, &unicast_process);
/*---------------------------------------------------------------------------*/
/* This function is called whenever a broadcast message is received. */
static void broadcast_recv(struct broadcast_conn *c, const rimeaddr_t *from)
{
received_data_mote = packetbuf_dataptr();

process_start(&blink_process, NULL);
/*  Registration of database information in the databases table *///
int i=0; int j=0;
for(i=0;i<=counter;i++){
if ((received_data_mote->id==routing_table[i].id)&&(received_data_mote->id!=0)){

if( received_data_mote->hop_count < routing_table[i].hop_count ){
routing_table[i].hop_count=received_data_mote->hop_count;
printf("less than\n");
routing_table[i]=*received_data_mote;
process_start(&display_process, NULL);
 received_data_mote->hop_count++;
process_start(&flooding_process, NULL);   
}
else if(received_data_mote->hop_count >= routing_table[i].hop_count){
break;
}

}


if((received_data_mote->id!=node_id)&&(received_data_mote->id!=0)){
for (j=0;j<anchors_num-1;j++){
if(routing_table[j].id==received_data_mote->id){
break;
}
else if(routing_table[j].id==0){
routing_table[counter]=*received_data_mote;
counter++;
//printf("received_data_mote->hop_count=%d\n",received_data_mote->hop_count );
process_start(&display_process, NULL);
 received_data_mote->hop_count++;
process_start(&flooding_process, NULL);
break;
}

}
}

}

/*************************************************************/
}
/**************************************************************************************************/
/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static void recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
   
received_exploit= packetbuf_dataptr();
if(node_id==1){
int dec_x = received_exploit->x; float frac_x = received_exploit->x - dec_x;  
int dec_y = received_exploit->y; float frac_y = received_exploit->y - dec_y;
printf("Node %u: x=%d.%02d, y=%d.%02d\n",received_exploit->id,dec_x,abs((int)(frac_x*100)),dec_y,abs((int)(frac_y*100)));
}
 else{
packetbuf_copyfrom(received_exploit, sizeof(struct exploit));
unicast_send(&unicast, &next_hop);
}
}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
/*---------------------------------------------------------------------------*/
/*--------------------------unicast_process-----------------------------------------------*/
PROCESS_THREAD(unicast_process, ev, data)
{
PROCESS_EXITHANDLER(unicast_close(&unicast);)   
PROCESS_BEGIN();
unicast_open(&unicast, 146, &unicast_callbacks);
PROCESS_END();
}
/*---------------------------------broadcast_process------------------------------------------*/
PROCESS_THREAD(broadcast_process, ev, data)
{
node_id_burn(1);
cc2420_set_txpower(2);
static struct etimer et1;
currently_information.id=node_id; currently_information.hop_count=1;
PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
PROCESS_BEGIN();
broadcast_open(&broadcast, 129, &broadcast_call);
switch(node_id){
case 1: currently_information.x =10; currently_information.y =0;break;
case 2: currently_information.x =30; currently_information.y =10;break;
case 3: currently_information.x =0; currently_information.y =20;break;
case 4: currently_information.x =27; currently_information.y =26;break;
}

if (node_id==1){
leds_on(LEDS_YELLOW);
etimer_set(&et1, (CLOCK_SECOND)*1+random_rand() % (CLOCK_SECOND)*1);
PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1));
printf("anchor function %u start broadcast\n",node_id);
packetbuf_copyfrom(&currently_information, sizeof(struct database));
broadcast_send(&broadcast);
leds_off(LEDS_YELLOW);
PROCESS_EXIT();
}
else if (node_id==2){
PROCESS_WAIT_EVENT_UNTIL(received_data_mote->id==1);
leds_on(LEDS_YELLOW);
etimer_set(&et1, (CLOCK_SECOND)*5+random_rand() % (CLOCK_SECOND)*5);
PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1));
printf("anchor %u start broadcast\n",node_id);
packetbuf_copyfrom(&currently_information, sizeof(struct database));
broadcast_send(&broadcast);
leds_off(LEDS_YELLOW);
PROCESS_EXIT();
}
else if (node_id==3){
PROCESS_WAIT_EVENT_UNTIL(received_data_mote->id==2);
leds_on(LEDS_YELLOW);
etimer_set(&et1, (CLOCK_SECOND)*5+random_rand() % (CLOCK_SECOND)*5);
PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1));
printf("anchor %u start broadcast\n",node_id);
packetbuf_copyfrom(&currently_information, sizeof(struct database));
broadcast_send(&broadcast);
leds_off(LEDS_YELLOW);
PROCESS_EXIT();
}

else if (node_id==4){
PROCESS_WAIT_EVENT_UNTIL(received_data_mote->id==3);
leds_on(LEDS_YELLOW);
etimer_set(&et1, (CLOCK_SECOND)*5+random_rand() % (CLOCK_SECOND)*5);
PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1));
printf("anchor %u start broadcast\n",node_id);
packetbuf_copyfrom(&currently_information, sizeof(struct database));
broadcast_send(&broadcast);
leds_off(LEDS_YELLOW);
PROCESS_EXIT();
}

PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(flooding_process, ev, data)
{
static struct etimer et1;
PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
PROCESS_BEGIN();


broadcast_open(&broadcast, 129, &broadcast_call);

 /*etimer_set(&et1,CLOCK_SECOND*1);
PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1));*/

etimer_set(&et1, (CLOCK_SECOND)*1+random_rand() % (CLOCK_SECOND)*1);
PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1));

int k=0;
for(k=0;k<anchors_num;k++){
if(received_data_mote->id==routing_table[k].id){
	if(received_data_mote->hop_count==routing_table[k].hop_count){
		printf("add hop\n");
		received_data_mote->hop_count++;
	}
}

}
packetbuf_copyfrom(*(&received_data_mote), sizeof(struct database));
broadcast_send(&broadcast);  
PROCESS_END();
}
/*---------------------------------------------------------------------------*/
 PROCESS_THREAD(blink_process, ev, data)
 {
 static struct etimer timer;
 PROCESS_BEGIN(); 
 leds_on(LEDS_GREEN);
 etimer_set(&timer, CLOCK_SECOND/4);
 PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
 leds_off(LEDS_GREEN);	 	 
 PROCESS_END();
 }
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(display_process, ev, data)
{ 
PROCESS_BEGIN(); 
int j=0;
for(j=0;j<counter;j++) {
printf("mote=%u,hops=%u/",routing_table[j].id,routing_table[j].hop_count);
}
printf("\n");
PROCESS_END();
}
/*---------------------------------------------------------------------------*/
