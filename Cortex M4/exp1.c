 
#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "debug.h"
#include "gpio.h"
#include "hw_types.h"
#include "pin_map.h"
#include "sysctl.h"


#define   FASTFLASHTIME			(uint32_t)500000
#define   SLOWFLASHTIME			(uint32_t)4000000

uint32_t delay_time,key_value,flag,key_value2;


void 		Delay(uint32_t value);
void 		S800_GPIO_Init(void);
void		changeFreq(void);
void		LED3(void);
void		LED4(void);



int main(void)
{
	S800_GPIO_Init();
	//changeFreq();
	LED3();
	//LED4();
}

void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}


void Delay_LED(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++)
	{
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) == 0)
			break;
	}
}

void S800_GPIO_Init(void)
{
	//SysCtlClockFreqSet((SYSCTL_OSC_INT | SYSCTL_USE_OSC), 16000000); //16MHz PIOSC
	SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN |SYSCTL_USE_OSC), 25000000); //25MHz MOSC
	//SysCtlClockFreqSet((SYSCTL_OSC_INT |SYSCTL_USE_PLL | SYSCTL_CFG_VCO_320), 40000000); //40MHz PLL
	//SysCtlClockFreqSet((SYSCTL_XTAL_20MHZ |SYSCTL_OSC_MAIN |SYSCTL_USE_OSC), 10000000); // wrong 20MHz MOSC
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);						//Enable PortN	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));			//Wait for the GPIO moduleN ready	
	
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0| GPIO_PIN_1);			//Set PF0 PF1 PE2 as Output pin
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);//Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);  //set pad config
}

void		changeFreq(void)
{
	
	while(1)
  {
		key_value = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	;				//read the PJ0 key value

		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	== 0)						//USR_SW1-PJ0 pressed
			delay_time							= FASTFLASHTIME;
		else
			delay_time							= SLOWFLASHTIME;
		
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);			// Turn on the LED.
		Delay(delay_time);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);							// Turn off the LED.
		Delay(delay_time);
   }
}


void		LED3(void)
{
	while(1)
	{
		key_value = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	;				//read the PJ0 key value
		key_value2 = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)	;				//read the PJ1 key value
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) == 0)
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);			// Turn on the LED1.
		else
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);							// Turn off the LED2.
		
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1) == 0)
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);			// Turn on the LED1.
		else
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);							// Turn off the LED2.
		
	}	
}

void		LED4(void)
{
	flag = 0;
	delay_time = FASTFLASHTIME;
	key_value = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0); 
	while(1)
	{		
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) == 0)
		{
			while (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) == 0);
			flag = flag + 1;
			if (flag == 5)
				flag = 1;
		}
		else
		{
			switch(flag)
			{
				case 1:
					while(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) != 0)
					{
						GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);			// Turn on the LED.
						Delay_LED(delay_time);
						GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);							// Turn off the LED.
						Delay_LED(delay_time);
					}
					break;
				case 2:
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);							// Turn off the LED.
					break;
				case 3:
					while(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) != 0)
					{
						GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);			// Turn on the LED.
						Delay_LED(delay_time);
						GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);							// Turn off the LED.
						Delay_LED(delay_time);
					}
					break;
				case 4:
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x0);							// Turn off the LED.
					break;
			}
		}
	}
}


