

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

//Write a value to an address

void write(int address, int value){
	int* ptr = address;
	*ptr = value;
}
//Read and return a value at an address

int read(int address) {
	int* ptr = address;
	return *ptr;
}

//Read Modify Write (condensed version of previous)

void readModifyWrite(int address, int bitmask){
	int newValue = (read(address) & ~bitmask) | bitmask;
	write(address, newValue);
}

void setupLed(){

	readModifyWrite(0x40048038, 1 << 10); 	//open port B
	write(0x4004A058, 0x00000140); 			//Configure PORTB_PCR22
	write(0x400FF054, 1 << 22); 			//Configure PDDR pin 22 output
	readModifyWrite(0x400FF040, 1 << 22); 	//Configure PSOR,  (1 = light off)

}

void toggleLED(){
		int ledState = read(0x400FF040) & (1 << 22);
		ledState ^= (1 << 22);
		write(0x400FF040, ledState);
	}

void initPIT(){
	readModifyWrite(0x4004803C, 1 << 23); 	// SIM_SCGC6
	write(0x40037000, 0x00000001); 			// PIT_MCR
	write(0x40037100, 0x0FFF4240); 			// PIT_LDVAL0, set the timeout period
	write(0x40037108, 0x00000003); 			// (PIT_TCTRL0) (enabled interrupts)
	write(0x4003710C, 0x00000001); 			// (PIT_TFLG0)

}
// Details of pit interrupt in NVIC
// Vector = 64
// IRQ Pit channel 0 = 48
// NVIC non-IPR register number = 1
// NVIC IPR register number = 12
// NVICISER1 = (IRQ%32) = 16
// NVICICER1 = 16
// NVICISPR1 = 16
// NVICICPR1 = 16
// NVICIABR1 = 16
// NVICIPR12: (8*(IRQ mod 4) + 4 = 4 => bitfield [7:4]

// Enable the PIT0 interrupt in the  ISER (Interrupt set enable register)
// write(0x00000100, 1 << 16); //  NVICISER1 = (IRQ%32) = (48 % 32) = 16
// This is not allowed (use PIT0_IRQn)


void PIT0_IRQHandler(){
			write(0x4003710C, 0x00000001); 	//clear flag
			toggleLED();
}


int main(void)
{
	char ch;

	setupLed();
	initPIT();
	NVIC_EnableIRQ(PIT0_IRQn);


	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitDebugConsole();


	while (1)
	{
		ch = GETCHAR();
		PUTCHAR(ch);
	}
}


