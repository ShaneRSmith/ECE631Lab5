/*
 * json_coms.h
 *
 *  Created on: Mar 7, 2016
 *      Author: Shane Smith
 */

#ifndef JSON_COMS_H_
#define JSON_COMS_H_

char* setup_wifi_(void);

char* setup_MQTT_(void);

char* setup_MQTT_Subs_(void);

char* publish(char*, char*);

uint8_t wifi_success_(char*);

uint8_t MQTT_success_(char*);

uint8_t SUBS_success_(char*);

uint8_t publish_success_(char*);

uint8_t startup_success_(char*);

#endif /* JSON_COMS_H_ */
