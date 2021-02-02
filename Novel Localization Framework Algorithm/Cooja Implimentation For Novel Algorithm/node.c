#include "contiki.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "lib/random.h"
#include "net/rime.h"
#include <stdio.h>
#include "dev/leds.h"
#include "node-id.h"
#include "functions.h"
#include <math.h>
#include "dev/cc2420.h"
#include "dev/cc2420_const.h"
#include <stdlib.h>
#include <string.h> 

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
}database; 
struct database *received_data_mote;
struct database routing_table[anchors_num];
int counter=0;

struct location {
uint8_t id;
float x;
float y;
} location;
struct location *received_location, estimated_received_location;
int ids_counter=0;
int dec_x; float frac_x;
int l=0,l1=0;
static struct broadcast_conn broadcast;
static struct unicast_conn unicast;

/*---------------------------------------------------------------------------*/
/* We first declare our two processes. */
PROCESS(broadcast_process, "Broadcast process");
PROCESS(flooding_process, "Flooding Process");
PROCESS(unicast_process, "Unicast process");
PROCESS(blink_process, "LED blink process");
PROCESS(display_process, "Display process");
PROCESS(trilateral_process, "trilateral process");

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
/*  Registration of database information in the databases table */
int i=0,j=0, k=0;
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
	
for (j=0;j<anchors_num;j++){
if(routing_table[j].id==received_data_mote->id){
break;
}
else if(routing_table[j].id==0){
routing_table[counter]=*received_data_mote;
counter++;ids_counter++;
process_start(&display_process, NULL);
 received_data_mote->hop_count++;
process_start(&flooding_process, NULL);
break;
}
}
}
}

/* This is where we define what function to be called when a broadcast
   is received. We pass a pointer to this structure in the
   broadcast_open() call below. */
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
/*---------------------------------------------------------------------------*/
/* This function is called for every incoming unicast packet. */
static void
recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
received_location= packetbuf_dataptr();
packetbuf_copyfrom(received_location, sizeof(struct location));
unicast_send(&unicast, &next_hop);
}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(broadcast_process, ev, data)
{
node_id_burn(1);
cc2420_set_txpower(2);
PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
PROCESS_BEGIN();
broadcast_open(&broadcast, 129, &broadcast_call);
PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(flooding_process, ev, data)
{
static struct etimer et1;
PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
PROCESS_BEGIN();
broadcast_open(&broadcast, 129, &broadcast_call);
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
packetbuf_copyfrom(*(&received_data_mote),sizeof(struct database));
broadcast_send(&broadcast);  
if(ids_counter==anchors_num){
etimer_set(&et1, (CLOCK_SECOND)*2+random_rand() % (CLOCK_SECOND)*2);
PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1));   
process_start(&trilateral_process, NULL);
}
PROCESS_EXIT();
PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unicast_process, ev, data)
{
PROCESS_EXITHANDLER(unicast_close(&unicast);)   
PROCESS_BEGIN();
unicast_open(&unicast, 146, &unicast_callbacks);
PROCESS_END();
}
/*-----------------------------------------------------------------------------------*/
 PROCESS_THREAD(trilateral_process, ev, data)
{
static struct etimer et1;
PROCESS_EXITHANDLER(unicast_close(&unicast);)
PROCESS_BEGIN();
unicast_open(&unicast, 146, &unicast_callbacks);
/****************Calcule Matrix A:(anchors_num-1*2)****************/

int l=0,l1=0;
int dec_x; 
float frac_x;
int i,j; 
float **A;
A=(float**)malloc((anchors_num-1)*sizeof(float*));
  for (i=0;i<anchors_num-1;i++)
     A[i]=(float*)malloc(3*sizeof(float));

  for(i=0;i<anchors_num-1;i++)
     for(j=0;j<2;j++)
  if (j==0)
    *(*(A+i)+j)=-2*(routing_table[i].x-routing_table[anchors_num-1].x);
  else if (j==1)
    *(*(A+i)+j)=-2*(routing_table[i].y-routing_table[anchors_num-1].y);


/***************************calculate avrhopsize based to the function*************************************/

float calcuh(float x){
int acc=25;
if(x<0){  
x=(-1)*x;
float ans=1;
float temp=1;
int i;
for(i=1;i<=acc;i++){
temp=(temp*x)/i;
ans=ans + temp;
}   
return 1/ans;
}
else{
float ans=1;
float temp=1;
int i;
for(i=1;i<=acc;i++){
temp=(temp*x)/i;
ans=ans + temp;
}   
return ans;
}
}
/******************************************/
float FX1(float x){
float function1=0;
function1=calcuh(pow(x,2));
return function1;
}
float solve_integral1(float initial, float tck1,float cuts1){
float sumation1=0;  int i=0;float delta1=0, xi1=0, value_i1=0;
 delta1=((float)(tck1-initial)/(float)cuts1);
 for(i=0;i<=cuts1;i++){
 xi1=(initial+delta1*i); 
 value_i1=delta1*(FX1(xi1));
 sumation1=sumation1+(value_i1);
} 
return sumation1;
}

/****************************************************************************/
float gethop(float gg){

  float h=gg,l=0.02,R=10;
int i=0;
float cuts1=0,cuts2=0;
float initial=0;
float tck1=0,tck2=0;
tck1=((-1+h)*sqrt(l)*sqrt(M_PI/3)*R);  
cuts1=50;
tck2=(h*sqrt(l)*sqrt(M_PI/3)*R);
cuts2=50;
float  finalanswer = (float)(calcuh((float)((-1)*pow(h,2)*M_PI*l*pow(R,2))/3 ))*(6*sqrt(3)*sqrt(l)*R*(( calcuh((float)(pow((-1+h),2)*l*M_PI*pow(R,2))/3) *(-1+h))+( -h* calcuh((float)(pow(h,2)*l*M_PI*pow(R,2))/3)  )) -9*solve_integral1(initial,tck1,cuts1) *(2/(float)sqrt(M_PI))  +9*((2/(float)sqrt(M_PI))*solve_integral1(initial,tck2,cuts2) )) / (float)(4*sqrt(l)*M_PI*(-1+calcuh((float)((1-2*h)*l*M_PI*pow(R,2))/3)));
return (float) finalanswer;
}

/**************************************************************************************************/

/****************Calcule Matrix B:(nbr_received_data_mote-1*1)*****************/
 float **B;float eval=0;
  B=(float**)malloc((anchors_num-1)*sizeof(float*));
  for (i=0;i<anchors_num-1;i++)
    B[i]=(float*)malloc(1*sizeof(float));
  for(i=0;i<anchors_num-1;i++)
  {
 *(*(B+i)+0)=pow(gethop(routing_table[i].hop_count),2)-pow(gethop(routing_table[anchors_num-1].hop_count),2)-
 pow(routing_table[i].x,2)+pow(routing_table[anchors_num-1].x,2)-pow(routing_table[i].y,2)+pow(routing_table[anchors_num-1].y,2);
  }

/****************Calcule Transpose A: A_T(2*nbr_anc-1)*****************/
 float **A_T;
 A_T=matrix_transpose(A,anchors_num-1,2);

 /****************Calcule A_T*A:ATA(2*2)*****************/
 float **ATA;
 ATA= matrix_multiplication(A_T,2,anchors_num-1,A,2);
  
 /****************Calcule ATA inverse:ATA_Inv(2*2)*****************/
 float **ATA_Inv;
 ATA_Inv=matrix_inverse(ATA,2);

 /****************Calcule ATA_Inv*A_T: ATA_AT(2*nbr_anc-1)*****************/
 float **ATA_AT;
 ATA_AT=matrix_multiplication(ATA_Inv, 2, 2, A_T, anchors_num-1);

 /****************Calcule ATA_AT*B: P(2*1)*****************/
 float **P;        
 P=matrix_multiplication(ATA_AT, 2, anchors_num-1, B, 1);
estimated_received_location.id=node_id;
estimated_received_location.x=*(*(P+0)+0); estimated_received_location.y=*(*(P+1)+0);
dec_x =*(*(P+0)+0);  frac_x = *(*(P+0)+0) - dec_x; 
printf("x=%d.%04d ",dec_x,abs((int)(frac_x*10000)));
dec_x =*(*(P+1)+0);  frac_x = *(*(P+1)+0) - dec_x; 
printf("y=%d.%04d \n",dec_x,abs((int)(frac_x*10000)));
for(i=0;i<anchors_num;i++)
free(A[i]);
free(A);
for(i=0;i<anchors_num;i++)
free(B[i]);
free(B);
free(A_T);
free(ATA);
free(ATA_Inv);
free(ATA_AT);
free(P);
PROCESS_EXIT();
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
printf("anchor=%u,hops=%u",routing_table[j].id,routing_table[j].hop_count);
}
printf("\n");
PROCESS_END();
}
/*---------------------------------------------------------------------------*/
