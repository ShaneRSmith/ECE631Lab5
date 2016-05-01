/**
 ******************************************************************************
 * @file    main.c
 * @author  Shane Smith
 * @version V1.0
 * @date    01-March-2016
 * @brief   Default main function.
 ******************************************************************************
 */

// Include Required Header Files
#include <string.h>
#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "USART.h"
#include "jansson.h"
#include "json_coms.h"
#include <sys/time.h>
#include "circ_buff.h"

// Global variables
static uint32_t ticks = 0;
static uint32_t lastTicks = 0;
static uint8_t button_flag = 0;
uint8_t str_start_flag=0;
char* msg;		// Message to Publish
char* topic;	// Topic to Publish

// Required Function Prototypes
uint8_t time_out(uint32_t delay_time);	// Timeout function

// UART6
#define MAX_STRLEN 2048
volatile int haveString;	// volatile tells the compiler this value can change very quickly

// Circular Buffers
commBuffer_t receive_buff;
commBuffer_t send_buff;

// Mode Enumeration
typedef enum
{
	STARTUP1,
	STARTUP2,
	WIFI,
	WIFI_RESP,
	MQTT,
	MQTT_RESP,
	MQTT_SUBS,
	MQTT_SUBS_RESP,
	PUBLISH,
	IDLE
}modes;

modes mode=STARTUP1;	// Initialize mode to look at first startup message.

int main(void) 
{
	haveString = 0;	// Initialize haveString to false
	InitUSART6();	// Initialize USART
	
	// Setup SysTick
	ticks = 0;
	lastTicks = 0;
	SystemCoreClockUpdate();

	InitBuffer(&receive_buff);	// Initialize receive buffer to '+'
	InitBuffer(&send_buff);		// Initialize send buffer to '+'
	STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);	// Initialize Push Button

	// Configure so 1 tick = 1ms or 1000tick = 1 second
	if (SysTick_Config(SystemCoreClock / 1000))
		while (1);

	while (1)
	{
		// Switch statement for State Machine cases. TODO: Add timeout to states.
		switch(mode){
		case STARTUP1:
			{	// We want to stay in this mode unless we successfuly startup, then we can start
				// setting up the other stuff.
				if (haveStr(&receive_buff)==1)
				{
					char* temp = (char*)malloc(sizeof(char)*receive_buff.str_len+sizeof(char));
					getRxStr(&receive_buff,temp);
					if (startup_success_(temp)==1)
					{	// We passed the startup test #1
						mode=STARTUP2;
					}
					free(temp);
				}
				break;
			}
		case STARTUP2:
			{
				if (haveStr(&receive_buff)==1)
				{
					char* temp = (char*)malloc(sizeof(char)*receive_buff.str_len+sizeof(char));
					getRxStr(&receive_buff,temp);
					if (startup_success_(temp)==1)
					{	// We passed the startup test #1
						mode=WIFI;
					}
					free(temp);
				}
				break;
			}
		case WIFI:
			{	// Setup wifi
				char* temp = (char*)malloc(sizeof(char)*98); // TODO: Size this more dynamically
				temp = setup_wifi_();
				putTxStr(&send_buff, temp, strlen(temp));
				free(temp);
				mode=WIFI_RESP;
				break;
			}
		case WIFI_RESP:
			{	// Look for wifi-response
				if (haveStr(&receive_buff)==1)
				{
					char* temp = (char*)malloc(sizeof(char)*receive_buff.str_len);
					getRxStr(&receive_buff,temp);
					// Let's check it to make sure we haven't reset the Huzzah
					if (startup_success_(temp)==1)
					{	// If we have we need to start our setup process again.
						mode=STARTUP2;
					}
					else
					{
						if (wifi_success_(temp)==1)
						{	// We have successfully setup wifi, now setup mqtt.
							mode=MQTT;
						}
					}
					free(temp);
				}
				else
				{
					if (time_out(5000)==1)	// Wait for 5 seconds, then try resending wifi_setup
						mode = WIFI;
				}
				break;
			}
		case MQTT:
			{	// Setup MQTT once the wifi is set up successfully
				char* temp = (char*)malloc(sizeof(char)*64); // TODO: Size this more dynamically
				temp=setup_MQTT_();
				putTxStr(&send_buff, temp, strlen(temp));
				free(temp);
				mode = MQTT_RESP;
				break;
			}
			case MQTT_RESP:
			{	// Look for MQTT response
				if (haveStr(&receive_buff)==1)
				{
					char* temp = (char*)malloc(sizeof(char)*receive_buff.str_len);
					getRxStr(&receive_buff,temp);
					// Let's check it to make sure we haven't reset the Huzzah
					if (startup_success_(temp)==1)
					{	// If we have we need to start our setup process again.
						mode=STARTUP2;
					}
					else
					{
						if (MQTT_success_(temp)==1)
						{	// We have successfully setup mqtt, now setup mqtt-subs
							mode=MQTT_SUBS;
						}
					}
					free(temp);
				}
				else
				{
					if (time_out(5000)==1)	// Wait for 5 seconds, then try resending MQTT_setup
						mode = MQTT;
				}
				break;
			}
		case MQTT_SUBS:
			{	// Setup MQTT subscriptions once MQTT is set up successfully
				char* temp = (char*)malloc(sizeof(char)*64); // TODO: Size this more dynamically
				temp=setup_MQTT_Subs_();
				putTxStr(&send_buff, temp, strlen(temp));
				free(temp);
				mode = MQTT_SUBS_RESP;
				break;
			}
		case MQTT_SUBS_RESP:
			{
				if (haveStr(&receive_buff)==1)
				{
					char* temp = (char*)malloc(sizeof(char)*receive_buff.str_len);
					getRxStr(&receive_buff,temp);
					// Let's check it to make sure we haven't reset the Huzzah
					if (startup_success_(temp)==1)
					{	// If we have we need to start our setup process again.
						mode=STARTUP2;
					}
					else
					{
						if (SUBS_success_(temp)==1)
						{	// We have successfully setup mqtt-subs, now go to idle.
							mode=IDLE;
						}
					}
					free(temp);
				}
				else
				{
					if (time_out(5000)==1)	// Wait for 5 seconds, then try resending MQTT subscriptions setup
						mode = MQTT_SUBS;
				}
				break;
			}
		case PUBLISH:
			{	// This case is used to publish messages defined in the IDLE case.
				char* temp = (char*)malloc(sizeof(char)*64);	// TODO: Allocate this size more dynamically.
				temp=publish(msg,topic);	// This takes the message and wraps it up in a JSON object
				free(temp);
				if (haveStr(&receive_buff)==1)
				{
					char* temp = (char*)malloc(sizeof(char)*receive_buff.str_len);
					getRxStr(&receive_buff,temp);
					// Let's check it to make sure we haven't reset the Huzzah
					if (startup_success_(temp)==1)
					{	// If we have we need to start our setup process again.
						mode=STARTUP2;
					}
					else
					{
						if (publish_success_(temp)==1)
						{	// We successfully published our message, go back to idle.
							mode=IDLE;
						}
					}
					free(temp);
				}
				break;
			}
		case IDLE:
			{	// Sit here waiting for the push_button.
				// We want to output button state when button is pressed and released.
				if (button_flag==1)
				{	// Button has been pressed, check for release
					if (STM_EVAL_PBGetState(BUTTON_USER)==0)
					{	// Button has been released
						msg="Up";
						topic="PushButton";
						button_flag=0;
					}
				}
				if (STM_EVAL_PBGetState(BUTTON_USER)==1)
				{	// Button was pressed, send message that it was pushed down.
					msg="Down";
					topic="PushButton";
					mode=PUBLISH;
					button_flag=1;
				}
				break;
			}
		}
	}
}


void SysTick_Handler(void)
{
	ticks++;
}


void USART6_IRQHandler(void)
{
	if (USART_GetITStatus(USART6, USART_IT_RXNE)==SET)
	{
		// We have a character, let's take it and put it into our buffer for later processing.
		char ch = USART_ReceiveData(USART6);
		putChar(&receive_buff,ch);
	}
	if (USART_GetFlagStatus(USART6,USART_FLAG_TXE)==SET && haveStr(&send_buff))
	{	// Send characters from our send buffer.
		char ch = getChar(&send_buff);
		USART_SendData(USART6,ch);
	}
	else
	{	// We want to disable the transmit interupts until we have stuff to send.
		USART_ITConfig(USART6,USART_IT_TC,DISABLE);
	}
}


int _gettimeofday(struct timeval *tv, void *tzvp)
{
	uint64_t t = ticks*1000; // get uptime in nanoseconds
	tv->tv_sec = t/1000000000; // convert to seconds
	tv->tv_usec = (t%1000000000)/1000; // get remaining microseconds
	return 0; // return non-zero for error
}

uint8_t time_out(uint32_t delay_time)
{	// This function functions as a time delay. It assumes it is being called at 1000 Hz.
	// The delay_time value is in ms, and is the size of the delay.
	static uint32_t count = 0;
	if (count >= delay_time)
	{
		count = 0;
		return (1);
	}
	else
	{
		count++;
		return (0);
	}
}