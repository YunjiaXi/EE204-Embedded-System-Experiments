#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "debug.h"
#include "gpio.h"
#include "hw_i2c.h"
#include "hw_types.h"
#include "i2c.h"
#include "pin_map.h"
#include "sysctl.h"
#include "systick.h"
#include "interrupt.h"
#include "uart.h"
#include "hw_ints.h"
#include <string.h>
#include <ctype.h>

#define SYSTICK_FREQUENCY		1000			//1000hz

#define	I2C_FLASHTIME				500				//500mS
#define GPIO_FLASHTIME			300				//300mS
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




void 		Delay(uint32_t value);
void 		S800_GPIO_Init(void);
uint8_t 	I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData);
uint8_t 	I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr);
void		S800_I2C0_Init(void);
void 		S800_UART_Init(void);
//systick software counter define
volatile uint16_t systick_10ms_couter,systick_100ms_couter;
volatile uint8_t	systick_10ms_status,systick_100ms_status, systick_1ms_status;

volatile uint8_t result,cnt,key_value,gpio_status, dig;
volatile uint8_t rightshift = 0x01;
uint32_t ui32SysClock;
uint8_t seg7[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x58,0x5e,0x079,0x71,0x5c};
char *month[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
uint8_t uart_receive_char;
char read_buf[100], write_buf[100];
int hour, min, second, timer = 0, i;
int hour_num,min_num,sed_num, num;



void req1()
{
	volatile uint16_t	i2c_flash_cnt,gpio_flash_cnt;
	//use internal 16M oscillator, PIOSC
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 16000000);		
	//ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 8000000);		
	//use external 25M oscillator, MOSC
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN |SYSCTL_USE_OSC), 25000000);		

	//use external 25M oscillator and PLL to 120M
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 120000000);;		
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 20000000);
	
  SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY);
	SysTickEnable();
	SysTickIntEnable();																		//Enable Systick interrupt
	  

	S800_GPIO_Init();
	S800_I2C0_Init();
	S800_UART_Init();
	
	hour= 0;
	min = 0;
	second = 0;
	
	IntEnable(INT_UART0);
  UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);	//Enable UART0 RX,TX interrupt
  IntMasterEnable();	
	
	while (1)
	{
		if (systick_1ms_status)
		{
			systick_1ms_status = 0;
			if (++i2c_flash_cnt		>= 3)
			{
				i2c_flash_cnt				= 0;
				
				switch(cnt)
				{
					case 0:
						dig=seg7[hour/10]; break;
					case 1:
						dig=seg7[hour%10]; break;
					case 2:
						dig=0x40; break;
					case 3:
						dig=seg7[min/10]; break;
					case 4:
						dig=seg7[min%10]; break;
					case 5:
						dig=0x40; break;
					case 6:
						dig=seg7[second/10]; break;
					case 7:
						dig=seg7[second%10]; break;
				}
				
				rightshift=1<<(cnt);
				result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,dig);	//write port 1 				
				result 							= I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);	//write port 2
				cnt = (cnt+1)%8;
			

			}
		}
		if (systick_10ms_status)
		{
			systick_10ms_status		= 0;
			if (++gpio_flash_cnt	>= GPIO_FLASHTIME/10)
			{
				gpio_flash_cnt			= 0;
				if (gpio_status)
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0,GPIO_PIN_0 );
				else
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0,0);
				gpio_status					= !gpio_status;
			
			}
		}
		if (systick_100ms_status)
		{
			systick_100ms_status	= 0;
			if(++timer>10)
			{
				timer = 0;
				second++;
				if (second==60)
				{	
					second=0;
					min++;
				}
				if(min==60)
				{
					min=0;
					hour = (hour+1)%24;
				}
			}
			
			
		}
	}
}
	

int main(void)
{
	req1();

}

void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}


void UARTStringPut(uint8_t *cMessage)
{
	while(*cMessage!='\0')
		UARTCharPut(UART0_BASE,*(cMessage++));
}
void UARTStringPutNonBlocking(const char *cMessage)
{
	while(*cMessage!='\0')
		UARTCharPutNonBlocking(UART0_BASE,*(cMessage++));
}

void S800_UART_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);						//Enable PortA
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));			//Wait for the GPIO moduleA ready

	GPIOPinConfigure(GPIO_PA0_U0RX);												// Set GPIO A0 and A1 as UART pins.
  GPIOPinConfigure(GPIO_PA1_U0TX);    			

  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Configure the UART for 115,200, 8-N-1 operation.
  UARTConfigSetExpClk(UART0_BASE, ui32SysClock,115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));
	UARTStringPut((uint8_t *)"\r\nHello, world!\r\n");
}
void S800_GPIO_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);						//Enable PortN	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));			//Wait for the GPIO moduleN ready		
	
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);			//Set PF0 as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);			//Set PN0 as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);		//Set PN1 as Output pin	

	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);//Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}

void S800_I2C0_Init(void)
{
	uint8_t result;
  SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);
  GPIOPinConfigure(GPIO_PB3_I2C0SDA);
  GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
  GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

	I2CMasterInitExpClk(I2C0_BASE,ui32SysClock, true);										//config I2C0 400k
	I2CMasterEnable(I2C0_BASE);	

	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT0,0x0ff);		//config port 0 as input
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT1,0x0);			//config port 1 as output
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT2,0x0);			//config port 2 as output 

	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_CONFIG,0x00);					//config port as output
	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0ff);				//turn off the LED1-8
	
}


uint8_t I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData)
{
	uint8_t rop;
	while(I2CMasterBusy(I2C0_BASE)){};
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
	I2CMasterDataPut(I2C0_BASE, RegAddr);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(I2C0_BASE)){};
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);

	I2CMasterDataPut(I2C0_BASE, WriteData);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
	while(I2CMasterBusy(I2C0_BASE)){};

	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	return rop;
}

uint8_t I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr)
{
	uint8_t value,rop;
	while(I2CMasterBusy(I2C0_BASE)){};	
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
	I2CMasterDataPut(I2C0_BASE, RegAddr);
//	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);		
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_SEND);
	while(I2CMasterBusBusy(I2C0_BASE));
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	Delay(1);
	//receive data
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, true);
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_RECEIVE);
	while(I2CMasterBusBusy(I2C0_BASE));
	value=I2CMasterDataGet(I2C0_BASE);
		Delay(1);
	return value;
}

/*
	Corresponding to the startup_TM4C129.s vector table systick interrupt program name
*/
void SysTick_Handler(void)
{
	systick_1ms_status = 1;
	if (systick_100ms_couter	!= 0)
		systick_100ms_couter--;
	else
	{
		systick_100ms_couter	= SYSTICK_FREQUENCY/10;
		systick_100ms_status 	= 1;
	}
	
	if (systick_10ms_couter	!= 0)
		systick_10ms_couter--;
	else
	{
		systick_10ms_couter		= SYSTICK_FREQUENCY/100;
		systick_10ms_status 	= 1;
	}
	if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) == 0)
	{
		systick_100ms_status	= systick_10ms_status = 0;
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,GPIO_PIN_0);		
	}
	else
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,0);		
}

/*
	Corresponding to the startup_TM4C129.s vector table UART0_Handler interrupt program name
*/
void UART0_Handler(void)
{
	int32_t uart0_int_status;
	int index = 0;
  uart0_int_status = UARTIntStatus(UART0_BASE, true);		// Get the interrrupt status.

  UARTIntClear(UART0_BASE, uart0_int_status);								//Clear the asserted interrupts

	while(UARTCharsAvail(UART0_BASE))    											// Loop while there are characters in the receive FIFO.
	{
		///Read the next character from the UART and write it back to the UART.
		read_buf[index] = toupper(UARTCharGetNonBlocking(UART0_BASE));
		index++;
		//UARTIntClear(UART0_BASE, uart0_int_status);								//Clear the asserted interrupts
		Delay(100);
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,GPIO_PIN_1 );		
	}
	
	read_buf[index]='\0';
	memset(write_buf,0,sizeof(write_buf));
	
	//UARTStringPutNonBlocking(read_buf);	
	//strcpy(write_buf,"Error!");
	if (strncmp(read_buf,"AT+CLASS",8)==0)
		strcpy(write_buf,"CLASS EE204");
	else if (strncmp(read_buf,"AT+STUDENTCODE",14)==0)
		strcpy(write_buf,"CODE 517030910102"); 
	else if (strncmp(read_buf,"SETHO:MI:SE",3)==0)
	{
		strcpy(write_buf,"TIMEHO:MI:SE");
		for(i = 3; i < 11;i++)
			write_buf[i+1] = read_buf[i];
		hour = (read_buf[3]-'0')*10 + read_buf[4]-'0';
		min = (read_buf[6]-'0')*10 + read_buf[7]-'0';
		second = (read_buf[9]-'0')*10 + read_buf[10]-'0';
	}
	else if(read_buf[2]=='-' && read_buf[5]=='-')
	{
		hour = (read_buf[0]-'0')*10 + read_buf[1]-'0';
		min = (read_buf[3]-'0')*10 + read_buf[4]-'0';
		second = (read_buf[6]-'0')*10 + read_buf[7]-'0';
		memset(read_buf,0,sizeof(read_buf));
		strcpy(write_buf,"TIMEHO:MI:SE");
		write_buf[4] = hour/10 + '0';
		write_buf[5] = hour%10 + '0';
		write_buf[7] = min/10 + '0';
		write_buf[8] = min%10 + '0';
		write_buf[10] = second/10 + '0';
		write_buf[11] = second%10 + '0';
	}
	else if (strncmp(read_buf,"INCHO:MI:SE",3)==0)
	{
		second = second + (read_buf[9]-'0')*10 + read_buf[10]-'0';
		i = second/60;
		second = second%60;
		min = min + (read_buf[6]-'0')*10 + read_buf[7]-'0' + i;
		i = min/60;
		min = min%60;
		hour = (hour + (read_buf[3]-'0')*10 + read_buf[4]-'0' + i)%24;
		
		strcpy(write_buf,"TIMEHO:MI:SE");
		write_buf[4] = hour/10 + '0';
		write_buf[5] = hour%10 + '0';
		write_buf[7] = min/10 + '0';
		write_buf[8] = min%10 + '0';
		write_buf[10] = second/10 + '0';
		write_buf[11] = second%10 + '0';
	}
	else if (strncmp(read_buf,"GETTIME",7)==0)
	{
		strcpy(write_buf,"TIMEHO:MI:SE");
		write_buf[4] = hour/10 + '0';
		write_buf[5] = hour%10 + '0';
		write_buf[7] = min/10 + '0';
		write_buf[8] = min%10 + '0';
		write_buf[10] = second/10 + '0';
		write_buf[11] = second%10 + '0';
	}
	else if (read_buf[5]=='+' || read_buf[5]=='-')
	{
		if (read_buf[5]=='+')
		{
			sed_num = (read_buf[3] + read_buf[9] - 2*'0')*10 +read_buf[4] + read_buf[10] - 2*'0';
			i = sed_num/60;
			sed_num = sed_num%60;
			min_num = i + (read_buf[0] + read_buf[6] - 2*'0')*10 +read_buf[1] + read_buf[7] - 2*'0';
			hour_num = min_num/60;
			min_num = min_num%60;
		}
		else
		{
			sed_num = (read_buf[3] - read_buf[9])*10 +read_buf[4] - read_buf[10];
			i = -(-sed_num+60)/60;
			sed_num = (sed_num+60)%60;
			min_num = i + (read_buf[0] - read_buf[6])*10 +read_buf[1] - read_buf[7];
			hour_num = -(60-min_num)/60;
			min_num = (min_num+60)%60;
		}
		
		
		
		memset(read_buf,0,sizeof(read_buf));
		write_buf[0] = hour_num/10 + '0';
		write_buf[1] = hour_num%10 + '0';
		write_buf[2] = ':';
		write_buf[3] = min_num/10 + '0';
		write_buf[4] = min_num%10 + '0';
		write_buf[5] = ':';
		write_buf[6] = sed_num/10 + '0';
		write_buf[7] = sed_num%10 + '0';
		write_buf[8] = '\0';
	}
	else if(read_buf[3]=='+' || read_buf[3]=='-')
	{
		i = 0;
		while(strncmp(month[i],read_buf,3))
			i++;
		num = (read_buf[4] - '0')*10 + read_buf[5] - '0';
		if (read_buf[3]=='+')
			i = (num + i)%12;
		else
			i = (i - num + 12)%12;
		memset(read_buf,0,sizeof(read_buf));
		strcpy(write_buf, month[i]);
	}
	else
	{
		//strcpy(write_buf, "Error");
	}
	
	
	UARTStringPutNonBlocking(write_buf);	
	Delay(10000);
	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,0 );	
}
