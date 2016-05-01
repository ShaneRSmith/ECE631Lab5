/*
 * json_coms.c *
 *  Created on: Mar 7, 2016
 *	This file was created to deal with the JSON communications, so it sets up strings for sending
 *	or decodes them to test the response for success. These functions are called in the main state machine.
 *      Author: Shane Smith
 */
#include <string.h>
#include "jansson.h"

char* setup_wifi_(void)
{	// Change wifi SSID and password strings as needed in 'wifi' variable.
	json_t *wifi = json_pack("{ssss}","SSID","ECE631Lab","Password","stm32F4.");
	json_t *full = json_pack("{ssso}","Action", "WifiSetup", "Wifi", wifi);
	char* x = json_dumps(full,0);
	strcat(x,"\n");
	return (x);
}
char* setup_MQTT_(void)
{	// Check IP and port number strings in 'mqtt' variable.
	json_t *mqtt = json_pack("{s,s,s,s}","Host", "192.168.1.5", "Port", "1883");
	json_t *full = json_pack("{s,s,s,o","Action","MQTTSetup","MQTT", mqtt);
	char* x = json_dumps(full,0);
	strcat(x,"\n");
	return (x);
}

char* setup_MQTT_Subs_(void)
{	// Fill in topic strings for correct topics in 'subs' variable.
	json_t *subs = json_pack("{s,s,s,s}","Topics","ece631/Topic1","ece631/Topic2","ece631/etc");
	json_t *full = json_pack("{s,s,s,o}","Action","MQTTSubs","MQTT",subs);
	char* x = json_dumps(full,0);
	strcat(x,"\n");
	return (x);
}

char* publish(char* str, char* tpc)
{	// This packs up a json message containing 'str' to be published to the topic 'tpc'.
	json_error_t error;
	json_t *jsonOutput=json_pack("{s,s,s,s}","Topic",tpc,"Message",str);
	json_t *full=json_pack("{s,s,s,o}","Action","MQTTPub","MQTT",jsonOutput);
	char* x = json_dumps(full,0);
	strcat(x,"\n");
	return (x);
}

uint8_t wifi_success_(char* str)
{	// This unpacks the json package we received and checks it for wifi-setup success.
	if(str[0] != '{') return 0; // Check to make sure the string can be a json object before continuing.
	uint8_t flag=0;
	json_error_t error;
	json_t *jsonResponse=json_loads(str,JSON_DECODE_ANY,&error);
	if (jsonResponse) { // Makes sure jsonResponse isn't NULL
		if (json_is_object(jsonResponse)){ // We got a response, make sure it is a WifiSetup response.
			char* ResponseType=json_string_value(json_object_get(jsonResponse,"Response"));
			char* Des_Resp_Type="WifiSetup";
			if (strcmp(ResponseType,Des_Resp_Type)==0)
			{	// We did get a response that matches WifiSetup, now check for Success.
				json_t* Wifi_Object=json_string_value(json_object_get(jsonResponse,"Wifi"));
				char* Result_Msg=json_string_value(json_object_get(Wifi_Object,"Result"));
				char* Des_Res_Msg="Success";
				if (strcmp(Result_Msg,Des_Res_Msg)==0)
				{	// We received a 'Success' message for wifi-setup.
					flag=1;
				}
			}
		}
	}
	return(flag);
}

uint8_t MQTT_success_(char* str)
{	// This unpacks the json package we received and checks it for mqtt-setup success.
	if(str[0] != '{') return 0; // Check to make sure the string can be a json object before continuing.
	uint8_t flag=0;
	json_error_t error;
	json_t *jsonResponse=json_loads(str,JSON_DECODE_ANY,&error);
	if (jsonResponse) { // Makes sure jsonResponse isn't NULL
		if (json_is_object(jsonResponse)) { // we got a response, make sure it is a MQTTSetup response.
			char* ResponseType=json_string_value(json_object_get(jsonResponse,"Response"));
			char* Des_Resp_Type="MQTTSetup";
			if (strcmp(ResponseType,Des_Resp_Type)==0)
			{	// We did get a response that matches MQTTSetup, now check for Success
				json_t* MQTT_Object=json_string_value(json_object_get(jsonResponse,"Message"));
				json_t* Result_Object=json_string_value(json_object_get(MQTT_Object,"MQTT"));
				char* Result_Msg=json_string_value(json_object_get(Result_Object,"Result"));
				char* Des_Res_Msg="Success";
				if (strcmp(Result_Msg,Des_Res_Msg)==0)
				{	// We received a 'Success' message for mqtt-setup.
					flag=1;
				}
			}
		}
	}
	return(flag);
}

uint8_t SUBS_success_(char* str)
{	// This unpacks the json package we received and checks it for mqtt-subs-setup success.
	uint8_t flag=0;
	json_error_t error;
	json_t *jsonResponse=json_loads(str,JSON_DECODE_ANY,&error);
	if (jsonResponse) { // Makes sure jsonResponse isn't NULL
		if (json_is_object(jsonResponse)) { // we got a response, make sure it is a MQTTSubs response.
			char* ResponseType=json_string_value(json_object_get(jsonResponse,"Response"));
			char* Des_Resp_Type="MQTTSubs";
			if (strcmp(ResponseType,Des_Resp_Type)==0)
			{	// We did get a response that matches MQTTSubs, now check for Subscribed.
				json_t* MQTT_Obj=json_string_value(json_object_get(jsonResponse,"Message"));
				json_t* Action_Obj=json_string_value(json_object_get(MQTT_Obj,"MQTT"));
				char* Result_Msg=json_string_value(json_object_get(Action_Obj,"Action"));
				char* Des_Res_Msg="Subscribed";
				if (strcmp(Result_Msg,Des_Res_Msg)==0)
				{	// We received a 'Subscribed' message for mqtt-subs-setup
					flag=1;
				}
			}
		}
	}
	return(flag);
}

uint8_t publish_success_(char* str)
{	// This unpacks the json package we received and checks it for publish success.
	uint8_t flag=0;
	json_error_t error;
	json_t *jsonResponse=json_loads(str,JSON_DECODE_ANY,&error);
	if (jsonResponse) { // Makes sure jsonResponse isn't NULL
		if (json_is_object(jsonResponse)) { // We got a response, make sure it is a MQTTPub response.
			char* ResponseType=json_string_value(json_object_get(jsonResponse,"Response"));
			char* Des_Resp_Type="MQTTPub";
			if (strcmp(ResponseType,Des_Resp_Type)==0)
			{	// We did get a response that matches MQTTPub, now check for Sent
				json_t* Msg_Obj=json_string_value(json_object_get(jsonResponse,"Message"));
				char* Result_Msg=json_string_value(json_object_get(Msg_Obj,"MQTT"));
				char* Des_Res_Msg="Sent";
				if (strcmp(Result_Msg,Des_Res_Msg)==0)
				{	// We recieved a 'Sent' message for mqtt-publish
					flag=1;
				}
			}
		}
	}
	return(flag);
}

uint8_t startup_success_(char* str)
{	// This unpacks the json package we received and checks it for startup description success.
	if(str[0] != '{') return 0; // Check to make sure the string can be a json object before continuing.
	uint8_t flag=0;
	json_error_t error;
	json_t *jsonResponse=json_loads(str,JSON_DECODE_ANY,&error);
	if (jsonResponse) { // Makes sure jsonResponse isn't NULL
		if (json_is_object(jsonResponse)) { // We got a response, make sure it is a startup response.
			char* ResponseType=json_string_value(json_object_get(jsonResponse,"Response"));
			char* Des_Resp_Type="StartUp";
			if (strcmp(ResponseType,Des_Resp_Type)==0)
			{	// We did get a response that matches startup, assume we're good.
				flag=1;
			}
		}
	}
	return(flag);
}
