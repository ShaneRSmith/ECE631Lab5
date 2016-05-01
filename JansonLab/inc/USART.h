/*
 * USART.h
 *
 *  Created on: Mar 1, 2016
 *      Author: srs56
 */

#ifndef USART_H_
#define USART_H_

#include "stm32f4xx.h"

void InitUSART6(void);
void SendCharArrayUSART6(char arr[], int len);

#endif /* USART_H_ */
