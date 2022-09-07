
#include "sys.h"

void _WAIT(uint32_t ms)
{
   static volatile uint32_t nCount;
   nCount=(18000)*ms;
   for (; nCount!=0; nCount--);
}		

void _WAIT_us(uint32_t us)
{
    static volatile uint32_t nCount;  
    nCount=18*us;
    for (; nCount >0; nCount--);
}

void _WAIT_t(uint32_t t)
{
    static volatile uint32_t nCount;  
    nCount=t;
    for (; nCount >0; nCount--);
}

void _WAIT_NOP (uint32_t stepDelay)
{    uint32_t iz = 0;
     for (iz=0; iz<(stepDelay); iz++) {
     __asm__ volatile("nop");
     }

}







void WriteDAC0(int val)
{
	SIM_SCGC2 |= SIM_SCGC2_DAC0_MASK;
	
	DAC0_C0 = DAC_C0_DACEN_MASK;  // 1.2V ref is DACREF_1
	
	//DAC0_C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK; // 3.3V VDDA is DACREF_2
	
	__asm__ ("usat    %[value], #12, %[value]\n\t" : [value] "+r" (val));  // 0 <= val <= 4095

	*(volatile aliased_int16_t *)&(DAC0_DAT0L) = val;
}



void WriteDAC1(int val)
{
	SIM_SCGC2 |= SIM_SCGC2_DAC1_MASK;
	
	DAC1_C0 = DAC_C0_DACEN_MASK;  // 1.2V ref is DACREF_1
	//DAC1_C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK; // 3.3V VDDA is DACREF_2
	
	__asm__ ("usat    %[value], #12, %[value]\n\t" : [value] "+r" (val));  // 0 <= val <= 4095

	*(volatile aliased_int16_t *)&(DAC1_DAT0L) = val;
}




int GetWDay(int d, int mon, int yr, int cent) 
{
  int wday;
  int a = (14 -  mon) / 12;
  int y = yr+cent*100 - a;
  int m =  mon + 12 * a - 2;
  wday  = ((7000 + (d + y + (y/4) - y / 100 + y / 400 + (31 * m) / 12)) % 7);
  return wday;
}


int number_days(int m, int y)
  {
    y=y+2000;
    int leap = (1 - (y % 4 + 2) % (y % 4 + 1)) * ((y % 100 + 2) % (y % 100 + 1)) + (1 - (y % 400 + 2) % (y % 400 + 1));
    return 28 + ((m + (m>>3)) % 2) + 2 % m + ((1 + leap) / m) + (1/m) - (leap/m);
  }
