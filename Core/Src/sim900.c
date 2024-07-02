/*
 * sim900.c
 *
 *  Created on: Feb 22, 2024
 *      Author: stella
 */
#include "sim900.h"

#include "main.h"
#include "usart.h"

#include <string.h>

#define PHONE_NUMBER "+385993809757"

const char ATcomm[] = "AT\r\n";
const char ATCREGcomm[] = "AT+CREG?\r\n";
const char ATCMGFcomm[] = "AT+CMGF=1\r\n";
const char ATCMGScomm[] = "AT+CMGS=\"" PHONE_NUMBER "\"\r\n";

const char* ATcommResp[] = {"OK", NULL};
const char* ATCREGcommResp[] = {"+CREG: 1,1", "OK", NULL};
const char* ATCMGFcommResp[] = {"OK", NULL};
const char* ATCMGScommResp[] = {">", NULL};
const char* smsMessageResp[] = {"OK", NULL};

const char messageSMS[] = "CO levels too high!\x1A";

char commReadBuff[40];  //velicina odgovora, sprema odgovore modula za obradu


#define sendComm(comm) (simUartWrite((comm), sizeof(comm)-1))
#define readResp(respBuff, timeout) (simUartRead((respBuff), sizeof(respBuff), (timeout)))


static bool checkResp(const char* expectedResp[], uint32_t timeout)
{
	bool retval = true;				// Provjerava je li ocekivani odgovor jednak primljenom

	readResp(commReadBuff, timeout);

	for(uint32_t i = 0; expectedResp[i]; i++)
	{
		if( !strstr(commReadBuff, expectedResp[i]) )
		{
			retval = false;
			break;
		}
	}

	if( retval )
	{
		uartPrintf("Got from SIM: %s\n", commReadBuff);
	}

	return retval;
}


void sim900Init()
{
	sendComm(ATcomm);
	while(!checkResp(ATcommResp, 1000))
	{
		uartPrintf("Waiting for AT\r\n");
		HAL_Delay(1000);
		sendComm(ATcomm);
	}

	sendComm(ATCREGcomm);
	while(!checkResp(ATCREGcommResp, 1000))
	{
		uartPrintf("Waiting for AT+CREG\r\n");
		HAL_Delay(1000);
		sendComm(ATCREGcomm);
	}

	sendComm(ATCMGFcomm);
	while(!checkResp(ATCMGFcommResp, 1000))
	{
		uartPrintf("Waiting for AT+CMGF\r\n");
	}

	uartPrintf("SIM900 init passed!\r\n");
}

// ako AT komande ne prolaze, vracaj ove odgovore

bool sim900SendSMS()
{
	sendComm(ATCMGScomm);
	if( !checkResp(ATCMGScommResp, 5000) )
	{
		uartPrintf("Couldn't enter send mode, aborting send\r\n");
		return false;
	}

	sendComm(messageSMS);
	if (!checkResp(smsMessageResp, 10000))
	{
		uartPrintf("Message took too long to send, reset SIM900\r\n");
		return false;
	}

	uartPrintf("SIM900 message sent!\r\n");
	return true;
}


