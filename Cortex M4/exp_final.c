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
#define   FASTFLASHTIME			(uint32_t)500000
#define   SLOWFLASHTIME			(uint32_t)4000000

uint32_t delay_time,key_value,flag,key_value2,ui32SysClock,count,flag;
char read_buf[100],write_buf[100];


void 		Delay(uint32_t value);
void 		S800_GPIO_Init(void);
uint8_t I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData);
void S800_SYSTICK_Init(void);
void S800_I2C0_Init(void);
void GPIOReadWrite();



int main(void)
{
	S800_GPIO_Init();
	
	//IntPrioritySet(INT_UART0,0x0);													//Set INT_UART0 to highest priority
	//IntPrioritySet(FAULT_SYSTICK,0x0e0);									//Set INT_SYSTICK to lowest priority
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
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN |SYSCTL_USE_OSC), 25000000); //25MHz MOSC
	//SysCtlClockFreqSet((SYSCTL_OSC_INT |SYSCTL_USE_PLL | SYSCTL_CFG_VCO_320), 40000000); //40MHz PLL
	//SysCtlClockFreqSet((SYSCTL_XTAL_20MHZ |SYSCTL_OSC_MAIN |SYSCTL_USE_OSC), 10000000); // wrong 20MHz MOSC
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);						//Enable PortN	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));			//Wait for the GPIO moduleN ready	
	
	//Set PF0(LED_M0&D4) PF1(LED_M1) PF2(LED_M2) PF3(LED_M4) PF4(D3) as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0| GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3| GPIO_PIN_4);
	// set PN0(D2) PN1(D1) as output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0| GPIO_PIN_1);
	//Set the PJ0(USR_SW1),PJ1(USR_SW2) as input pin
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);
	//set pad of PJ0(USR_SW1),PJ1(USR_SW2) as weak_pull_up
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}


void GPIOReadWrite()
{
		GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	;				//read the USR_SW1-PJ0 key value
		GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)	;				//read the USR_SW2-PJ1 key value
	
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);			// Turn on the LED(PF0,LED_M0&M4).
		Delay(delay_time);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);							// Turn off the LED(PF0,LED_M0&M4).
		Delay(delay_time);
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

void S800_SYSTICK_Init(void)
{
	SysTickPeriodSet(ui32SysClock/1000);
	SysTickEnable();
	SysTickIntEnable();
	IntMasterEnable();
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
	IntEnable(INT_UART0);
  UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);	//Enable UART0 RX,TX interrupt
  IntMasterEnable();	
}

void I2CReadWrite()
{

}



void SysTick_Handler(void) 
{
	count++;
	if(count==100)
	{
		flag=1;
		count=0;
	}
}


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
	if (strncmp(read_buf,"class",4)==0)
	{
	
	}
}
