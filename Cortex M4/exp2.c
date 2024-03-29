
#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "debug.h"
#include "gpio.h"
#include "hw_i2c.h"
#include "hw_types.h"
#include "hw_gpio.h"
#include "i2c.h"
#include "pin_map.h"
#include "sysctl.h"
#include "systick.h"
#include "interrupt.h"
//*****************************************************************************
//
//I2C GPIO chip address and resigster define
//
//*****************************************************************************
#define TCA6424_I2CADDR 					0x22
#define PCA9557_I2CADDR						0x18

#define PCA9557_INPUT							0x00
#define	PCA9557_OUTPUT						0x01
#define PCA9557_POLINVERT					0x02
#define PCA9557_CONFIG						0x03

#define TCA6424_CONFIG_PORT0			0x0c
#define TCA6424_CONFIG_PORT1			0x0d
#define TCA6424_CONFIG_PORT2			0x0e

#define TCA6424_INPUT_PORT0				0x00
#define TCA6424_INPUT_PORT1				0x01
#define TCA6424_INPUT_PORT2				0x02

#define TCA6424_OUTPUT_PORT0			0x04
#define TCA6424_OUTPUT_PORT1			0x05
#define TCA6424_OUTPUT_PORT2			0x06


#define SYSTICK_FREQUENCY  1000
#define SYSTICK_FREQUENCY_1  500
#define SYSTICK_FREQUENCY_2  5000
#define SYSTICK_FREQUENCY_3  2000
#define LED_TIMES  500
#define PF0_TIMES  300
#define Seconds  1000
#define Fast_Seconds  200
#define Tremble_threshold  50
#define Longclick_threshold  1000
#define Second_Per_Hour  3600
#define Second_Per_Minute  60
#define minute_pos  (1<<0)
#define _minute_pos  (1<<1)
#define second_pos  (1<<2)
#define _second_pos  (1<<3)

void 		Delay(uint32_t value);
void 		S800_GPIO_Init(void);
void S800_SYSTICK_Init(void);
uint8_t 	I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData);
uint8_t 	I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr);
void		S800_I2C0_Init(void);
volatile uint8_t result;
uint32_t ui32SysClock;
uint8_t seg7[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x58,0x5e,0x079,0x71,0x5c};
int elect=1;
int task3=0, mode=0;

uint32_t LED_td = 0, PF0_td = 0, LED_flag = 0, PF0_flag = 0, cnt=0, _cnt=0,second_counter=0, minute_counter=0;
uint32_t second_flag = 0, fast_second_flag = 0, Long_click = 0, _Long_click = 0, workmode=0,second_td=0, fast_second_td=0;;

void req1(void);
void req2(void);
void req3(void);
void req4(void);
void req5(void);
void Q2(void);
void Q3(void);
void Q4(void);
void Q5(void);

int main(void)
{
	//use internal 16M oscillator, HSI
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 16000000);		
	
		S800_SYSTICK_Init();
		S800_GPIO_Init();
		S800_I2C0_Init();
		//req5();
		Q3();
}

void req1(void)
{
	while (1)	{
		
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2, 0xff);			// Select all digits
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[7]);		// Display number 7			
		
			I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0);	

			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);												// Turn on the PF0 
			Delay(800000);
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);																// Turn off the PF0.
			Delay(800000);	

		}
}

void req2(void)
{
	while (1)	{
		
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2, 0x01<<(elect-1));			// Select all digits
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[elect]);		        // Display number 7			
		
			I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~(0x01<<(elect-1)));	

			Delay(800000);

			elect=(elect%8)+1;
		}
}

void req3(void)
{
	while (1)	{
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2, 0x01<<(elect-1));			// Select all digits
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[elect]);		        // Display number 7			
		
			I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~(0x01<<(elect-1)));	

			Delay(800000);
		  if(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) != 0)
				elect=(elect%8)+1;
		}
}

void req4(void)
{
	while (1)	{
		if(LED_flag)
		{
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2, 0x01<<(elect-1));			// Select all digits
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[elect]);		        // Display number 7			
		
			I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~(0x01<<(elect-1)));	

			LED_flag=0;
		  if(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) != 0)
				elect=(elect%8)+1;
		}
		}
}

void req5(void)
{
	while(1){
		if(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) == 0)
			task3=1;
		if(task3)
		{
		  if(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) != 0)
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);	
			else
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
		}
		else if(mode==0)
		{ 
			if(LED_flag)
			{
				I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2, 0x01<<(elect-1));			// Select all digits
				I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[elect]);		        // Display number 7			
		
				I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~(0x01<<(elect-1)));	
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);	
				elect=(elect%8)+1;
				LED_flag=0;
			}
		}
		else if (mode==1)
		{
			if (PF0_flag == 1)
				{	
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);	
				}					
				else if (PF0_flag == 2)
				{	
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);	
				}	
				//I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2, 0x00);			// Select all digits
			
			
		}
		
	}
}

void Q2(void)
{
	while (1)	{
		
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2, 0x01<<(elect-1));			// Select all digits
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[elect]);		        // Display number 
			if(elect==8)
				I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x7e);
			else
				I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~(0x03<<(elect-1)));	
		
			elect=(elect%8)+1;
			
			Delay(800000);
		}
}


void Q3(void)
{
	uint8_t pos;
	while (1)	{
		  
			if(elect==8)
				pos=0x81;
			else
				pos=0x03<<(elect-1);
			
			I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~pos);	
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0);		                  // Display number 	
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2, 0x01<<(elect-1));			// Select all digits
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[elect]);		        // Display number 
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0);		                  // Display number 	
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2, pos-(0x01<<(elect-1)));			// Select all digits
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[elect%8+1]);		        // Display number 
			
			if(LED_flag)
			{
				LED_flag=0;
				elect=(elect%8)+1;
			}
				
		}
}

void Q4(void)
{
	uint8_t pos;
	int mode=0;
	while (1)	{
		  
			if(elect==8)
				pos=0x81;
			else
				pos=0x03<<(elect-1);
			
			I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~pos);	
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0);		                  // Display number 	
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2, 0x01<<(elect-1));			// Select all digits
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[elect]);		        // Display number 
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0);		                  // Display number 	
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2, pos-(0x01<<(elect-1)));			// Select all digits
			I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[elect%8+1]);		        // Display number 
			
			
		  if (!GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0))
				cnt++;
			else
				cnt = 0;
			if (cnt == 100) mode = (mode + 1) % 4;  //avoid trembles
			switch (mode)
		{
			case 0:
				SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY);
				break;
			case 1:
				SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY_1);
				break;
			case 2:
				SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY_2);
				break;
			case 3:
				SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY_3);
				break;
		}
			
			if(LED_flag)
			{
				LED_flag=0;
				elect=(elect%8)+1;
			}
				
		}
}

void Q5(void)
{
		while (true)
		{
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0);
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,(uint8_t)(minute_pos));  //execute before port 1
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[((second_counter % Second_Per_Hour) / Second_Per_Minute) / 10]);
		
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0);
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,(uint8_t)(_minute_pos));  //execute before port1
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[((second_counter % Second_Per_Hour) / Second_Per_Minute) % 10]);	
		
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0);
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,(uint8_t)(second_pos));  //execute before port 1
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(second_counter % Second_Per_Minute) / 10]);
		
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0);
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,(uint8_t)(_second_pos));  //execute before port 1
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(second_counter % Second_Per_Minute) % 10]);
		
		if (second_flag && !Long_click)
		{
			second_flag = 0;
			second_counter++;
		}
		
		if (fast_second_flag && Long_click && !_Long_click)
		{
			fast_second_flag = 0;
			second_counter++;
		}
		else if (fast_second_flag && !Long_click && _Long_click)
		{
			fast_second_flag = 0;
			second_counter += Second_Per_Minute;
		}
		else if (fast_second_flag && Long_click && _Long_click)
		{
			fast_second_flag = 0;
			second_counter++;
			second_counter += Second_Per_Minute;
		}
		
		if (!GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0))
			cnt++;
		else
		{
			cnt = 0;
			Long_click = 0;
		}
		if (cnt == Tremble_threshold)
			second_counter++;
		if (cnt == Longclick_threshold)
			Long_click = 1;
		
		if (!GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_1))
			_cnt++;
		else
		{
			_cnt = 0;
			_Long_click = 0;
		}
		if (_cnt == Tremble_threshold)
			second_counter += Second_Per_Minute;
		if (_cnt == Longclick_threshold)
			_Long_click = 1;
	}
}



void S800_SYSTICK_Init(void)
{
	SysTickPeriodSet(ui32SysClock/1000);
	SysTickEnable();
	SysTickIntEnable();
	IntMasterEnable();
}

void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}


void SysTick_Handler(void) 
{
	LED_td++;
	PF0_td++;
	workmode++;
	if (workmode == 3000)
	{
		mode = 1;
	}
	else if (workmode == 3000 * 2)
	{
		mode = 0;
		workmode = 0;
	}
	
	if (LED_td == 500)
	{
		LED_flag = 1;
	}
	else if (LED_td == 500 * 2)
	{
		LED_flag = 2;
		LED_td = 0;
	}
	if (PF0_td == PF0_TIMES)
	{
		PF0_flag = (PF0_flag + 1) % 3;
		PF0_td = 0;
	}
	
	second_td++;
	fast_second_td++;
	
	if (second_td == Seconds)
	{
		second_flag = 1;
		second_td = 0;
	}
	if (fast_second_td == Fast_Seconds)
	{
		fast_second_flag = 1;
		fast_second_td = 0;
	}
	
}

void S800_GPIO_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);						//Enable PortN	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));			//Wait for the GPIO moduleN ready	
	
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);							//Set PF0 as Output pin
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);			//Set PN0 as Output pin
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);	//Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}


void S800_I2C0_Init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);			// Provide clock to enable I2C0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);		// I2C0 uses pin PB2 and PB3, which by default are used as GPIO. We need to program to change the use of PB2 and PB3
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);							// Program on GPIOPCTL to select I2C0 SCL to be connected to PB2
  GPIOPinConfigure(GPIO_PB3_I2C0SDA);							// Program on GPIOPCTL to select I2C0 SDA to be connected to PB3
  GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);	// Program on GPIOAFSEL to swith PB2 to be used by hardware and program the pad control of this pin for this
  GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);		// Program on GPIOAFSEL to swith PB3 to be used by hardware and program the pad control of this pin for this

	I2CMasterInitExpClk(I2C0_BASE,ui32SysClock, true);						//enable I2C0 master, the input clock for I2C0 is the system clock and set I2C0 to run in fast mode (400kbps) or normal mode

	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT0,0x0ff);		//config port0 of TCA6424 as input
	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT1,0x0);			//config port1 of TCA6424 as output
	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT2,0x0);			//config port2 of TCA6424 as output

	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0xff);		//Select all digits
	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x0);			//Turn off all LED tubes
	


	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_CONFIG,0x0);						//config the port of PCA9557 as output
	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xff);					//turn off the LED1-8
	
}


uint8_t I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData)
{
	uint8_t rop;
	while(I2CMasterBusy(I2C0_BASE));								// Wait if I2C0 master is busy
		
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false); // Set the slave address in the master and indicate the direction of the following transmission (false means send, true means read)		
	I2CMasterDataPut(I2C0_BASE, RegAddr);							// Set the data (the command of the expender chip) to be sent
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);	// Start the transmission and send multiple bytes
	while(I2CMasterBusy(I2C0_BASE));								// Wait if I2C0 master is busy
		
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);						// if there is error in the transmission, terminate the transmission
	if(rop) {
		I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_STOP);
		return rop;
	}

	I2CMasterDataPut(I2C0_BASE, WriteData);						// Set the data (the data to the register specified by the above command) to be sent
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);// send the second byte and terminate the transmission when finished
	while(I2CMasterBusy(I2C0_BASE));								// Wait if I2C0 master is busy

	rop = (uint8_t)I2CMasterErr(I2C0_BASE);						// if there is error in the transmission, return the error status
	return rop;
}

uint8_t I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr)
{
	uint8_t value,rop;
	while(I2CMasterBusy(I2C0_BASE));								// Wait if I2C0 master is busy

	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);	// Set the slave address in the master and indicate the direction of the following transmission (false means send, true means read)	
	I2CMasterDataPut(I2C0_BASE, RegAddr);							// Set the data (the command of the expender chip) to be sent
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_SEND);	// Start the transmission and send one byte
	while(I2CMasterBusBusy(I2C0_BASE));								// Wait if I2C0 master is busy
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	if(rop) {																					// if there is error in the transmission, return the error status
		return rop;
	}
	Delay(1);

	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, true);	// Set the slave address in the master and indicate the direction of the following transmission (false means send, true means read)	
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_RECEIVE);// Start the transmission and read one byte
	while(I2CMasterBusBusy(I2C0_BASE));								// Wait if I2C0 master is busy
	value=I2CMasterDataGet(I2C0_BASE);								// Get the data from the data register of the master
	Delay(1);
	return value;
}

