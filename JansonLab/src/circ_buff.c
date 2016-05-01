/*
 * circ_buff.c
 *  Created on: Mar 8, 2016
 *	This file was created to define the circular buffer as well as methods used to access it,
 *	modify it, and check it for strings to process.
 *      Author: Shane Smith
 */

#include <string.h>
#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "USART.h"
#include "circ_buff.h"


void InitBuffer(commBuffer_t* comm)
{	// Initializes the struct commBuffer_t to zero
	comm->head=0; comm->tail=0;
	memset(comm->buffer,'+',MAXCOMMBUFFER + 1);	// Initialize the buffer to something we aren't looking for.
}


uint8_t haveStr(commBuffer_t* comm)
{	// Test if a complete string is in buffer
	uint8_t flag = 0;
	comm->str_len=0;

	// Check if buffer is empty
	if (comm->head == comm->tail)
		flag = 0;
	// Else, check if full string is in buffer.
	else
	{
		for (int i = comm->tail; i != comm->head ; i=(i+1)%MAXCOMMBUFFER)
		{
			if ((comm->buffer[i]=='\r')||(comm->buffer[i]=='\n'))
				{
					flag=1;
					break;
				}
			comm->str_len++;
		}
	}
	return flag;
}


void putChar(commBuffer_t* comm, char ch)
{	// Put character in buffer and update head
	uint8_t next = comm->head+1;
	if (next>=MAXCOMMBUFFER)
		next=0;
	comm->buffer[comm->head]=ch;
	comm->head=next;
	update_size(comm);
}


char getChar(commBuffer_t* comm)
{	// Get character from buffer and update tail;
	uint8_t next = comm->tail+1;
	if (next>=MAXCOMMBUFFER)
		next=0;
	char x = comm->buffer[comm->tail];
	comm->buffer[comm->tail]='+';
	comm->tail=next;
	update_size(comm);
	return x;
}


void putTxStr(commBuffer_t* comm, char* str, uint8_t length)
{	// put C string into buffer (used mainly with transmit buffer)
	// Putting a c-string 'str' into buffer 'comm' of length 'length'
	// The plan is to use our character functions we already wrote in here.

	for (int i=0; i<length; i++)
	{
		putChar(comm, str[i]);
	}
	USART_ITConfig(USART6,USART_IT_TC,ENABLE);
}


void getRxStr(commBuffer_t* comm, char* str)
{	// get C string from buffer (used mainly with receive buffer)
	// This function assumes a full string is in the buffer in question.
	// haveStr should be run before this function is called to prevent errors.

	uint8_t j = 0; char x;
	uint8_t n = comm->str_len;	// Grab size of buffer at start for # of iterations.

	for (int i = 0; i < n+1; i++)
	{	// The for loop runs through the full size of the buffer grabbing a full string.
		x=getChar(comm);
		if (i<n)
		{
			str[j]=x;
			j++;
		}
	}
	str[j]='\0'; // NULL Terminate str
}


void update_size(commBuffer_t* comm)
{	// Updates current size of buffer
	// This is called in the getChar and putChar functions to update size
	// when the head or tail change positions.
	comm->size = (comm->head - comm->tail + MAXCOMMBUFFER+1)%(MAXCOMMBUFFER+1);
}
