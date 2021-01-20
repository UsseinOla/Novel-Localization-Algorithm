
#include "contiki.h"
 #include "math.h"
#include <stdio.h> 
int dec_x; float frac_x;

/*********************************************************************/
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
float gethop(int gg){
int i=0;
float cuts1=0,cuts2=0;
float initial=0;
float tck1=0,tck2=0;
float h=gg,l=0.02,R=10;

int loop=25;
tck1=((-1+h)*sqrt(l)*sqrt(M_PI/3)*R);  
if (tck1<1){
cuts1=loop;
} 
else{
cuts1=tck1*loop;
}

tck2=(h*sqrt(l)*sqrt(M_PI/3)*R);

if (tck1<2){
cuts2=loop;
} 
else{
cuts2=tck2*loop;
}

float  finalanswer = (float)(calcuh((float)((-1)*pow(h,2)*M_PI*l*pow(R,2))/3 ))*(6*sqrt(3)*sqrt(l)*R*(( calcuh((float)(pow((-1+h),2)*l*M_PI*pow(R,2))/3) *(-1+h))+( -h* calcuh((float)(pow(h,2)*l*M_PI*pow(R,2))/3)  )) -9*solve_integral1(initial,tck1,cuts1) *(2/(float)sqrt(M_PI))  +9*((2/(float)sqrt(M_PI))*solve_integral1(initial,tck2,cuts2) )) / (float)(4*sqrt(l)*M_PI*(-1+calcuh((float)((1-2*h)*l*M_PI*pow(R,2))/3)));
dec_x =finalanswer;  frac_x = finalanswer - dec_x; 
printf("finalanswer  %d.%04d \n ",dec_x,abs((int)(frac_x*10000)));

return (float) finalanswer;
}
/*---------------------------------------------------------------------------*/
PROCESS(calculate_exponantila, "calculate_exponantila process");
AUTOSTART_PROCESSES(&calculate_exponantila);
/*---------------------------------------------------------------------------*/


PROCESS_THREAD(calculate_exponantila, ev, data)
{
  PROCESS_BEGIN();

int i;
for(i=1;i<=3;i++){
float solve=gethop(i);

}

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
