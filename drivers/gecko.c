/*
	USB Gecko SE API - Part of the Gecko Basic SDK
 
	gecko.c
 
	2010 www.usbgecko.com - code by ian@unrom.com
	
	All the functions below are used to interface with the USB Gecko SE device, the functions are highly optimized to use with the hardware
	and you should never have to modify them.
	
	Code is Public Domain.
*/

#include "drivers/drivers.h"

#define EXI_CHAN1SR			*(volatile unsigned long*) 0x0D006814 // Channel 1 Status Register
#define EXI_CHAN1CR			*(volatile unsigned long*) 0x0D006820 // Channel 1 Control Register
#define EXI_CHAN1DATA		*(volatile unsigned long*) 0x0D006824 // Channel 1 Immediate Data

#define EXI_TSTART			1

char gecko_cmd;

static unsigned int gecko_sendbyte(char data)
{
	unsigned int i = 0;
	
	EXI_CHAN1SR = 0x000000D0;						
	EXI_CHAN1DATA = 0xB0000000 | (data << 20);	
	EXI_CHAN1CR = 0x19;			

	while((EXI_CHAN1CR) & EXI_TSTART);						
	i = EXI_CHAN1DATA;	
	EXI_CHAN1SR = 0;
	
	if (i&0x04000000)
	{
		return 1;									
	}
	
    return 0;										
}

void gecko_putchar(int c)
{
	gecko_sendbyte((unsigned char) (c & 0xff));
}


unsigned int gecko_receivebyte(char* data)
{
	unsigned int i = 0;


	EXI_CHAN1SR = 0x000000D0;			
	EXI_CHAN1DATA = 0xA0000000;		
	EXI_CHAN1CR = 0x19;		

	while((EXI_CHAN1CR) & EXI_TSTART);				
	i = EXI_CHAN1DATA;					
	EXI_CHAN1SR = 0;
	
	if (i&0x08000000)
	{
		*data = (i>>16)&0xff;
		return 1;						
	} 
	
	return 0;							
}


// return 1, it is ok to send data to PC
// return 0, FIFO full
static unsigned int gecko_checktx(void)
{
	unsigned int i  = 0;
	
	EXI_CHAN1SR = 0x000000D0;					
	EXI_CHAN1DATA = 0xC0000000;						
	EXI_CHAN1CR = 0x09;								

	while((EXI_CHAN1CR) & EXI_TSTART);		
	i = EXI_CHAN1DATA;									
	EXI_CHAN1SR = 0x0;

	if (i&0x04000000)
	{
		return 1;
	}
	
    return 0;										
}


// return 1, there is data in the FIFO to recieve
// return 0, FIFO is empty
unsigned int gecko_checkrx(void)
{
	unsigned int i = 0;
	
	EXI_CHAN1SR = 0x000000D0;						
	EXI_CHAN1DATA = 0xD0000000;			
	EXI_CHAN1CR = 0x09;								

	while((EXI_CHAN1CR) & EXI_TSTART);							   	
	i = EXI_CHAN1DATA;								
	EXI_CHAN1SR = 0x0;
	
	if (i&0x04000000)
	{
		return 1;
	}							

    return 0;										
}

void gecko_send(const void *buffer, unsigned int size)
{
	char *sendbyte = (char*) buffer;				
	unsigned int ret = 0;

	while (size  > 0)
	{
		if(gecko_checktx())
		{
			ret = gecko_sendbyte(*sendbyte);		
			if(ret == 1)
			{							
				sendbyte++;							
				size--;						
			}
		}
	}
}


void gecko_receive(void *buffer, unsigned int size)
{
	char *receivebyte = (char*)buffer;					
	unsigned int ret = 0;

	while (size > 0)
	{
		if(gecko_checkrx())
		{
			ret = gecko_receivebyte(receivebyte);	
			if(ret == 1)
			{							
				receivebyte++;							
				size--;						
			}
		}
	}
}

char gecko_getchar(void)
{
 	while (!gecko_checkrx());
	gecko_receivebyte(&gecko_cmd);
 	return gecko_cmd;
}
