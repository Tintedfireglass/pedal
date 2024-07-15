/*
 * CAN_driver.h
 *
 *  Created on: Jun 15, 2024
 *      Author: verma
 */

#ifndef INC_CAN_DRIVER_H_
#define INC_CAN_DRIVER_H_

#include "main.h"
#include "commonDriver.h"

typedef struct{
	uint32_t ID;
	uint8_t IDE;
	uint8_t DLC;
	uint8_t data[8];
	uint8_t Fifo;
}canData;

//void canSelectBaudRate(uin32_t baudrate);
uint8_t canNotification();
void canTransmit(uint8_t data);
void canReceive(canData *data);
void processCanMsg(canData *data);
uint8_t getNodes();
void sendPing();

#endif /* INC_CAN_DRIVER_H_ */
