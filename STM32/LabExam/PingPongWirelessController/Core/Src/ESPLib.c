/*
 * ESPLib.c
 *
 *  Created on: Feb 14, 2022
 *      Author: Can Güveren
 */

#include "ESPLib.h"

// Instantiate the global variables to resolve L6218E errors
uint8_t uartRxData = 0;
uint16_t DataCounter = 0;
volatile uint8_t RxInterruptFlag = 0;
volatile uint8_t uartTimeCounter = 0;
volatile uint8_t uartPacketComplatedFlag = 0;

volatile uint8_t temp_mqttMsgLen = 0;
volatile uint8_t temp_mqttTopicLen = 0;

char Buffer[BUFFERSIZE];
char rxBuffer[BUFFERSIZE];
char mqttBuffer[BUFFERSIZE];
char mqttPacket[BUFFERSIZE];
/* ========================================== */
/* CHANGE END                                 */
/* ========================================== */



static UART_HandleTypeDef esp_uart;

/*Responses and Commands*/
char *OK = "OK\r\n";
char *WIFIDISCONNECT = "OK\r\nWIFI DISCONNECT\r\n";
char *WIFICONNECT = "WIFI CONNECTED\r\nWIFI GOT IP\r\n";
char *SEND_OK = "SEND OK\r\n";
char *ERROR_ = "ERROR\r\n";
char *CIPSEND_RESPONSE = "OK\r\n> ";
char *PORTCONNECT = "CONNECT\r\n";
char *WIFICONNECTED = "WIFI CONNECTED\r\n";
static char *FAILED_ = "FAIL\r\n";
static char *PROMPT = ">";
static char *BUSY = "...\r\n";
static uint8_t espReportedFailure = 0;


/* Reads replies byte by byte and returns as soon as they end with the expected
   or failure text, instead of waiting for a fixed number of bytes. */
static funcState_t waitForResponse(char *expected, char *failure, uint32_t timeout)
{
	uint16_t count = 0;
	uint16_t expectedSize = strlen(expected);
	uint16_t failureSize = strlen(failure);
	uint16_t busySize = strlen(BUSY);
	uint32_t start = HAL_GetTick();
	uint8_t data;

	espReportedFailure = 0;
	memset(Buffer, 0, BUFFERSIZE);

	while((HAL_GetTick() - start) < timeout)
	{
		if(HAL_UART_Receive(&esp_uart, &data, 1, 1) != HAL_OK)
		{
			continue;
		}

		if(count == BUFFERSIZE)		//Buffer is full, so keep only the newest bytes.
		{
			memmove(Buffer, &Buffer[1], BUFFERSIZE - 1);
			count--;
		}
		Buffer[count++] = data;

		if((count >= expectedSize) && (memcmp(&Buffer[count - expectedSize], expected, expectedSize) == 0))
		{
			return funcOk;
		}

		if((count >= failureSize) && (memcmp(&Buffer[count - failureSize], failure, failureSize) == 0))
		{
			espReportedFailure = 1;
			return funcErr;
		}

		if((count >= busySize) && (memcmp(&Buffer[count - busySize], BUSY, busySize) == 0))	//"busy p..." or "busy s..."
		{
			espReportedFailure = 1;
			HAL_Delay(100);
			return funcErr;
		}
	}

	return funcErr;
}

static funcState_t sendCommand(char *cmd, uint8_t cmdSize, char *expected, char *failure, uint32_t timeout)
{
	if(HAL_UART_Transmit(&esp_uart, (uint8_t *)cmd, cmdSize, 100) != HAL_OK)
	{
		return funcErr;
	}

	return waitForResponse(expected, failure, timeout);
}


funcState_t ESP8266_Init(espMode_t Mode, UART_HandleTypeDef UARTHandle)
{
	funcState_t checkFunc;
	char cmd[20];
	uint8_t cmdSize;

	memcpy(&esp_uart, &UARTHandle, sizeof(UARTHandle));

	do{
	checkFunc = sendCommand("AT\r\n", strlen("AT\r\n"), OK, ERROR_, 200);	//If OK response, ESP8266 is working.
	}while(checkFunc != funcOk);

	do{
	memset(cmd, 0, sizeof(cmd));
	cmdSize = sprintf(cmd, "AT+CWMODE=%d\r\n", Mode);
	checkFunc = sendCommand(cmd, cmdSize, OK, ERROR_, 200);
	}while(checkFunc != funcOk);

	return funcOk;
}


funcState_t ESP8266_Reset(void)
{
	funcState_t checkFunc;
	char cmd[10];

	do{
    memset(cmd, 0, sizeof(cmd));
	checkFunc = sendCommand("AT+RST\r\n", strlen("AT+RST\r\n"), OK, ERROR_, 200);
	}while(checkFunc != funcOk);

	return funcOk;
}

funcState_t ESP8266_wifiConnect(char *SSID, char *Password)
{
	funcState_t checkFunc;
	char cmd[100];
	uint8_t cmdSize;

	do{
	checkFunc = sendCommand("AT+CWQAP\r\n", strlen("AT+CWQAP\r\n"), OK, ERROR_, 1000);	//If the device is connected to a network, the connection is disconnected.
	}while(checkFunc != funcOk);

	do{
	memset(cmd, 0, sizeof(cmd));
	cmdSize = sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, Password);
	checkFunc = sendCommand(cmd, cmdSize, OK, FAILED_, 20000);		//Returns as soon as the ESP replies OK.
	}while(checkFunc != funcOk);

	return funcOk;
}

funcState_t ESP8266_portConnect(char *type, char *remoteIP, char *remotePort)
{
	funcState_t checkFunc;
	char cmd[100];
	uint8_t cmdSize;

	sendCommand("AT+CIPCLOSE\r\n", strlen("AT+CIPCLOSE\r\n"), OK, ERROR_, 1000);	//ERROR is fine here. It means no connection was open.

	do{
	memset(cmd, 0, sizeof(cmd));
	cmdSize = sprintf(cmd, "AT+CIPSTART=\"%s\",\"%s\",%s\r\n", type, remoteIP, remotePort);
	checkFunc = sendCommand(cmd, cmdSize, OK, ERROR_, 20000);
	if(checkFunc != funcOk)
	{
		HAL_Delay(1000);
	}
	}while(checkFunc != funcOk);

	return funcOk;
}

funcState_t ESP8266_sendMessage(char *msg, uint8_t msgSize)
{
	funcState_t checkFunc = funcErr;
	char cmd[100];
	uint8_t cmdSize;
	uint8_t attempt;
	uint8_t staleByte;

	HAL_UART_AbortReceive_IT(&esp_uart);

	memset(cmd, 0, sizeof(cmd));
	cmdSize = sprintf(cmd, "AT+CIPSEND=%d\r\n", msgSize);

	/* Only retry when the ESP clearly rejected the command. If it hasn't replied, it may
	   still be waiting for data, and a second command would be sent to the broker as data. */
	for(attempt = 0; attempt < 3; attempt++)
	{
		if(attempt > 0)
		{
			HAL_Delay(50);
		}

		while(HAL_UART_Receive(&esp_uart, &staleByte, 1, 0) == HAL_OK)		//Clear any old reply, such as a late '>'.
		{
		}

		checkFunc = sendCommand(cmd, cmdSize, PROMPT, ERROR_, 1000);

		if((checkFunc == funcOk) || (espReportedFailure == 0))
		{
			break;
		}
	}

	/* If there was no reply, the prompt may be late or lost, so send the packet to finish
	   the command. If the ESP never got the command, the next send gets ERROR and retries. */
	if((checkFunc == funcOk) || (espReportedFailure == 0))
	{
		checkFunc = sendCommand(msg, msgSize, SEND_OK, FAILED_, 1000);		//Returns as soon as the ESP replies SEND OK.
	}

	HAL_UART_Receive_IT(&esp_uart, &uartRxData, 1);

	return checkFunc;
}

funcState_t checkResponse(char *response)
{
	uint8_t i;
	funcState_t checkState;
	uint8_t responseSize;
	uint8_t sizeBuffer;

	responseSize = strlen(response);

	sizeBuffer = strlen(Buffer);
	memcpy(rxBuffer, &Buffer[sizeBuffer - responseSize], responseSize);

	for(i = 0; i < responseSize; i++)
	{
		if(rxBuffer[i] != response[i])
		{
			checkState = funcErr;
			break;
		}
		else
		{
			checkState = funcOk;
		}
	}

	memset(rxBuffer, 0, BUFFERSIZE);
	return checkState;
}


/******************************************************************************************************************************/
/********************************************************MQTT Functions********************************************************/
/******************************************************************************************************************************/

void MQTT_connectBroker(MQTT_InitTypeDef_t *MQTTConnect)
{
	char *protocolName = "MQTT";

	uint16_t protocolLenght  = strlen(protocolName); //protocolLenght 2 byte. "MQTT" 4 byte.
	uint16_t clientIDLenght = strlen(MQTTConnect->clientID);

	uint8_t mqttLenght;

	memset(mqttPacket, 0, sizeof(mqttPacket));

	if(MQTTConnect->connectFlag == (PasswordFlag | UserNameFlag))
	{
		uint16_t usernameLenght = strlen(MQTTConnect->username);
		uint16_t passwordLenght = strlen(MQTTConnect->password);
		uint8_t remainingLenght = 2 + protocolLenght + 4 + 2 + clientIDLenght + 2 + usernameLenght + 2 + passwordLenght;

		mqttLenght = sprintf(mqttPacket, "%c%c%c%c%s%c%c%c%c%c%c%s%c%c%s%c%c%s", (char)MQTTconnect, (char)remainingLenght, (char)(protocolLenght >> 8), (char)(protocolLenght & 0x00FF),
							 protocolName,(char)MQTTlevel,(char)MQTTConnect->connectFlag, (char)(MQTTConnect->keepAlive >> 8), (char)MQTTConnect->keepAlive, (char)(clientIDLenght >> 8), (char)(clientIDLenght & 0x00FF),
							 MQTTConnect->clientID, (char)(usernameLenght >> 8), (char)usernameLenght, MQTTConnect->username, (char)(passwordLenght >> 8), (char)passwordLenght, MQTTConnect->password);
	}
	else if(MQTTConnect->connectFlag == PasswordFlag)
	{
		uint16_t passwordLenght = strlen(MQTTConnect->password);
		uint8_t remainingLenght = 2 + protocolLenght + 4 + 2 + clientIDLenght + 2 + passwordLenght;

		mqttLenght = sprintf(mqttPacket, "%c%c%c%c%s%c%c%c%c%c%c%s%c%c%s", (char)MQTTconnect, (char)remainingLenght, (char)(protocolLenght >> 8), (char)(protocolLenght & 0x00FF),
							 protocolName,(char)MQTTlevel,(char)MQTTConnect->connectFlag, (char)(MQTTConnect->keepAlive >> 8), (char)MQTTConnect->keepAlive, (char)(clientIDLenght >> 8), (char)(clientIDLenght & 0x00FF),
							 MQTTConnect->clientID, (char)(passwordLenght >> 8), (char)passwordLenght, MQTTConnect->password);
	}
	else if(MQTTConnect->connectFlag == UserNameFlag)
	{
		uint16_t usernameLenght = strlen(MQTTConnect->username);
		uint8_t remainingLenght = 2 + protocolLenght + 4 + 2 + clientIDLenght + 2 + usernameLenght;

		mqttLenght = sprintf(mqttPacket, "%c%c%c%c%s%c%c%c%c%c%c%s%c%c%s", (char)MQTTconnect, (char)remainingLenght, (char)(protocolLenght >> 8), (char)(protocolLenght & 0x00FF),
							 protocolName,(char)MQTTlevel,(char)MQTTConnect->connectFlag, (char)(MQTTConnect->keepAlive >> 8), (char)MQTTConnect->keepAlive, (char)(clientIDLenght >> 8), (char)(clientIDLenght & 0x00FF),
							 MQTTConnect->clientID, (char)(usernameLenght >> 8), (char)usernameLenght, MQTTConnect->username);
	}
	else
	{
		uint8_t remainingLenght = 2 + protocolLenght + 4 + 2 + clientIDLenght;

		mqttLenght = sprintf(mqttPacket, "%c%c%c%c%s%c%c%c%c%c%c%s", (char)MQTTconnect, (char)remainingLenght, (char)(protocolLenght >> 8), (char)(protocolLenght & 0x00FF),
							 protocolName,(char)MQTTlevel,(char)MQTTConnect->connectFlag, (char)(MQTTConnect->keepAlive >> 8), (char)MQTTConnect->keepAlive, (char)(clientIDLenght >> 8), (char)(clientIDLenght & 0x00FF),
							 MQTTConnect->clientID);
	}

	ESP8266_sendMessage(mqttPacket, mqttLenght);

}

void MQTT_publishTopic(char *topic, char *msg, uint8_t QoS)
{
	uint8_t mqttLenght;

	uint16_t topicLenght = strlen(topic);
	uint16_t msgLenght = strlen(msg);

	uint8_t remainingLenght = 2 + topicLenght + msgLenght;

	if((uint16_t)(4 + topicLenght + msgLenght) >= sizeof(mqttPacket))		//The packet would overflow mqttPacket.
	{
		return;
	}

	memset(mqttPacket, 0, sizeof(mqttPacket));

	mqttLenght = sprintf(mqttPacket, "%c%c%c%c%s%s", (char)(MQTTpublish | QoS), (char)remainingLenght, (char)topicLenght >> 8, (char)topicLenght, topic, msg);

	ESP8266_sendMessage(mqttPacket, mqttLenght);
}

void MQTT_subscribeTopic(char *topic, uint8_t QoS)
{
	uint8_t mqttLenght;

	uint16_t topicLenght = strlen(topic);

	uint8_t remainingLenght = 5 + topicLenght;

	uint16_t packetID = 0x01;
	memset(mqttPacket, 0, sizeof(mqttPacket));

	mqttLenght = sprintf(mqttPacket, "%c%c%c%c%c%c%s%c", (char)MQTTsubscribe, (char)remainingLenght, (char)packetID >> 8, (char)packetID, (char)topicLenght >> 8, (char)topicLenght, topic, QoS);

	ESP8266_sendMessage(mqttPacket, mqttLenght);
}


void MQTT_unsubscribeTopic(char *topic)
{
	uint8_t mqttLenght;

	uint16_t topicLenght = strlen(topic);

	uint8_t remainingLenght = 4 + topicLenght;

	uint16_t packetID = 0x01;

	memset(mqttPacket, 0, 100);

	mqttLenght = sprintf(mqttPacket, "%c%c%c%c%c%c%s", (char)MQTTunsub, (char)remainingLenght, (char)packetID >> 8, (char)packetID, (char)topicLenght >> 8, (char)topicLenght, topic);

	ESP8266_sendMessage(mqttPacket, mqttLenght);
}

void MQTT_disconnectBroker(void)
{
	uint8_t mqttLenght;
	uint8_t remainingLenght = 0;

	memset(mqttPacket, 0, 100);

	mqttLenght = sprintf(mqttPacket, "%c%c", (char)MQTTdisconnect, (char)remainingLenght);

	ESP8266_sendMessage(mqttPacket, mqttLenght);

}

void MQTT_pingReq()
{
	uint8_t mqttLenght;
	uint8_t remainingLenght = 0;

	memset(mqttPacket, 0, 100);

	mqttLenght = sprintf(mqttPacket, "%c%c", (char)MQTTpingreq, (char)remainingLenght);

	ESP8266_sendMessage(mqttPacket, mqttLenght);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	RxInterruptFlag = SET;
	mqttBuffer[DataCounter++] = uartRxData;
	if(DataCounter >= 300)
	{
		DataCounter  = 0;
	}

	HAL_UART_Receive_IT(&esp_uart , &uartRxData , 1);
	uartTimeCounter = 0;
}


void MQTTDataHandler(char *mqttTopic, char *mqttMsg)
{
	uint8_t mqttMsgLen, digit_mqttMsgLen;
	uint16_t mqttTopicLen;
	uint8_t colonIndex;
	uint8_t i;
	uint8_t digitNumber = 0;

	if(uartPacketComplatedFlag == SET)     //Data receiving is finished
	{
		uartPacketComplatedFlag = RESET;

		for(i = 0; i < DataCounter; i++)
		{
			if(mqttBuffer[i] == ':')
			{
				colonIndex = i;
				break;
			}
		}

		memset(mqttTopic, 0, temp_mqttTopicLen);
		memset(mqttMsg, 0, temp_mqttMsgLen);

		mqttMsgLen = mqttBuffer[colonIndex + 2];
		digit_mqttMsgLen = mqttMsgLen + 2;

	    while(digit_mqttMsgLen > 0){
	    	digit_mqttMsgLen = digit_mqttMsgLen / 10;
	        digitNumber++;
	    }

		mqttTopicLen = (mqttBuffer[10 + digitNumber] << 8 ) | (mqttBuffer[11 + digitNumber]);
		memcpy(mqttMsg, &mqttBuffer[15 + digitNumber], mqttMsgLen - mqttTopicLen - 2);
		memcpy(mqttTopic, &mqttBuffer[12 + digitNumber], mqttTopicLen);
		memset(mqttBuffer, 0, DataCounter);

		temp_mqttMsgLen = mqttMsgLen;
		temp_mqttTopicLen = mqttTopicLen;
		DataCounter = 0;

	}
}

/* This function should be called in systick timer */
void MQTTTimer(void)
{
	if(RxInterruptFlag == SET)
	{
		if(uartTimeCounter++ > 100)
		{

			RxInterruptFlag = RESET;
			uartTimeCounter = 0;
			uartPacketComplatedFlag = SET;
		}
	}
}
