/*
 * CAN_driver.c
 *
 *  Created on: Jun 15, 2024
 *      Author: verma
 */

#include "CAN_driver.h"

extern CAN_HandleTypeDef hcan1;

extern bmsData bmsDataObj;
extern commonData commonDataObj;

//macros for can requests from motor controller
#define SPEED_ACTUAL 		0x30
#define SPEED_RPMMAX_INT 	0xCE
#define TORQUE_COMMAND      0x90
#define MC_READ 				0x3D
#define MC_RX_ADDR			0x201
#define MC_TX_ADDR			0x181

char* txFailure = "No data sent through can!\r\n";
char* rxFailure = "NO data received through can!\r\n";
char* canActivationFault = "CAN not activated!\r\n";
char* txMailboxesFull = "Tx mailboxes full!\r\n";

uint8_t canNotification(){
	uint8_t state = 0;

	if (HAL_CAN_ActivateNotification(&hcan1,CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING) != HAL_OK) {
		LOGS((uint8_t*)canActivationFault, strlen(canActivationFault));
		state = 1;
	} else {
		state = 0;
	}

	return state;
}

void canTransmit(uint8_t choice){
	CAN_TxHeaderTypeDef txHeader;
	txHeader.DLC = 3;
	txHeader.StdId = 0x201;
	txHeader.IDE = CAN_ID_STD;
	txHeader.TransmitGlobalTime = DISABLE;
	txHeader.RTR = CAN_RTR_DATA;

	static int counter = 0;

	if(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1)==0){
		LOGS((uint8_t*)txMailboxesFull,strlen(txMailboxesFull));
		char msg[16];
		sprintf(msg,"%d\r\n",counter);
		LOGS((uint8_t*)msg,strlen(msg));
	}


	uint8_t data1[3] = {TORQUE_COMMAND,commonDataObj.torqueLow,commonDataObj.torqueHigh};
	uint8_t data2[3] = {MC_READ,SPEED_ACTUAL,0x00};
	uint8_t data3[3] = {MC_READ,SPEED_RPMMAX_INT,0x00};

	if ((HAL_CAN_IsTxMessagePending(&hcan1, CAN_TX_MAILBOX0) && HAL_CAN_IsTxMessagePending(&hcan1, CAN_TX_MAILBOX1) && HAL_CAN_IsTxMessagePending(&hcan1, CAN_TX_MAILBOX2)) == 0) {
		uint32_t mailBox;
		counter++;
		char state1 = HAL_CAN_AddTxMessage(&hcan1,&txHeader,data1,&mailBox);
		if (state1 != HAL_OK) {
			LOGS((uint8_t*)txFailure,strlen(txFailure));
//			Error_Handler();
		}

		char state2 = HAL_CAN_AddTxMessage(&hcan1,&txHeader,data2,&mailBox);
		if (state2 != HAL_OK) {
			LOGS((uint8_t*)txFailure,strlen(txFailure));
		//	Error_Handler();
		}

		char state3 = HAL_CAN_AddTxMessage(&hcan1,&txHeader,data3,&mailBox);
		if (state3 != HAL_OK) {
			LOGS((uint8_t*)txFailure,strlen(txFailure));
		//	Error_Handler();
		}
	}
	else {
		LOGS((uint8_t*)"CBSY\n", 5);
	}
}
//this is used for bms
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){
	CAN_RxHeaderTypeDef rxHeader;

	canData data[] = {0};

	if(HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rxHeader, data->data) != HAL_OK){
		LOGS((uint8_t*)rxFailure,strlen(rxFailure));
	}
	if (rxHeader.ExtId == 0x1806E5F4) {
		data->ID = rxHeader.ExtId;
		data->IDE = CAN_ID_EXT;
	} else if (rxHeader.ExtId == 0x1806E9F4) {
		data->ID = rxHeader.ExtId;
		data->IDE = CAN_ID_EXT;
	} else if (rxHeader.ExtId == 0x1806E7F4) {
		data->ID = rxHeader.ExtId;
		data->IDE = CAN_ID_EXT;
	} else {
		char msg[32];
		sprintf(msg, "%lu\r\n", rxHeader.ExtId);
		LOGS((uint8_t*)msg, strlen(msg));
	}
	data->DLC = rxHeader.DLC;
	data->Fifo = 0;

	processCanMsg(data);

	uint8_t fill0 = HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0);
	if(fill0 == 0){
		return;
	}
}
//this is used for motor controller
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan){
	CAN_RxHeaderTypeDef rxHeader;

	canData data[] = {0};

	if(HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO1, &rxHeader, data->data) != HAL_OK){
		LOGS((uint8_t*)rxFailure,strlen(rxFailure));
	}
	if(rxHeader.StdId == 0x181){
		data->ID = rxHeader.StdId;
		data->IDE = CAN_ID_STD;
	} else {
		char msg[32];
		sprintf(msg, "%lu\r\n", rxHeader.StdId);
		LOGS((uint8_t*)msg, strlen(msg));
	}
	data->DLC = rxHeader.DLC;
	data->Fifo = 0;

	processCanMsg(data);
	uint8_t fill1 = HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO1);
	if(fill1 == 0){
		return;
	}
}

uint16_t getPv(uint8_t *arr) {
    // Assuming len is the length of the array arr
    uint16_t hex_number = 0;

    // Combine first two bytes into a 16-bit hexadecimal number
    hex_number = (arr[0] << 8) | arr[1];

    return hex_number;
}

uint16_t getMcCAN(uint8_t *arr) {
    // Assuming len is the length of the array arr
    uint16_t hex_number = 0;

    // Combine first two bytes into a 16-bit hexadecimal number
    hex_number = (arr[2] << 8) | arr[1];

    return hex_number;
}

uint16_t getPower(uint8_t *arr) {
    // Assuming len is the length of the array arr
    uint16_t hex_number = 0;

    // Combine first two bytes into a 16-bit hexadecimal number
    hex_number = (arr[2] << 8) | arr[3];

    return hex_number;
}

uint16_t getAverageTemp(uint8_t *arr) {
    // Assuming len is the length of the array arr
    uint16_t hex_number = 0;

    // Combine first two bytes into a 16-bit hexadecimal number
    hex_number = (arr[4] << 8);

    return hex_number;
}

void processCanMsg(canData *data){
	if(data->ID == 0x1806E5F4){
		char *id = "0x1806E5F4!\r\n";
		LOGS((uint8_t*)id,strlen(id));
	}
	else if(data->ID == 0x1806E7F4){
		char msg[100];
		bmsDataObj.pv = getPv(data->data);
		bmsDataObj.kwPower = getPower(data->data);
		bmsDataObj.relayState = data->data[4];
		bmsDataObj.soc = data->data[5];
		sprintf(msg,"pv:%d.%d\r\nkwpwr:%d.%d\r\nrelayState:%d\r\nsoc:%d\r\n",bmsDataObj.pv/10,bmsDataObj.pv%10, bmsDataObj.kwPower/10,bmsDataObj.kwPower%10,bmsDataObj.relayState,bmsDataObj.soc);
		LOGS((uint8_t*)msg,strlen(msg));
	}
	else if (data->ID == 0x1806E9F4) {
		char *id = "0x1806E9F4!\r\n";
		LOGS((uint8_t*)id,strlen(id));
	}
	else if(data->ID == 0x181){
		if(data->data[0]==SPEED_RPMMAX_INT){
			uint16_t rpm = getMcCAN(data);
			char rpmDI[32];
			sprintf(rpmDI,"%d",rpm);
			LOGS(rpmDI,strlen(rpmDI));
		}
		else if(data->data[0]==SPEED_ACTUAL){
			uint16_t speed = getMcCAN(data);
			char speedActualDI[32];
			sprintf(speedActualDI,"%d",speed);
			LOGS(speedActualDI,strlen(speedActualDI));
		}
	}
}


