/*
 * spwf01sa_WiFi.c
 *
 *  Created on: 2015 nov. 5
 *      Author: kecsger12
 */


/*================================[Includes] ======================================================================================*/

#include "spwf01sa_WiFi.h"


/*================================[Macros]==============================================================*/

#define PUCLIC_KEY			VGvKypXX74umKnMGagVQ
#define PRIVATE_KEY			9Yx4X2ggJehka0welb8R

/*================================[Internal Functions]====================================================*/

static int dec2bcdhex(int x);

/*================================[Internal constants]==============================================================*/

int timeOut_cnt = 0;

/*================================[Function definitions]==============================================================*/

uint8_t WiFi_Init()
{
	uint8_t retVal;

	//WiFi_InitGPIO();
	UART_Init();

	//WiFi_nReset(0);
	//WiFi_nReset(1);

	ILI9341_Puts(2, 15, "Connecting to network...", &TM_Font_7x10, ILI9341_COLOR_BLACK, ILI9341_COLOR_YELLOW);
	//WiFi_ConnetToNetwork();

	/* FONTOS!!! varni kell egy picit az eszkoz felallasara !!!
	 *
	 * Ujrainditas utan: <cr><lf>+WIND:2:Reset<cr><lf> -> kell megkapnom utana
	 *
	 * vagy WAIT ~16 sec
	 *
	 * vagy lekerem a statust, megnezem a WiFi statuszat, ameddig nem HandShake complete/Ready to transmit, varok
	 *
	 */
	//HAL_Delay(16000);

	//ILI9341_Fill(2,5, 180, 15, ILI9341_COLOR_WHITE); /* Clear previous text */

	//WiFi_Enable();
	//WiFi_InitSettings();
	//WiFi_GetStatus();
	//WiFi_ScanNetworks();

	WiFi_isComWorking();

/*
	while(WiFi_Send_PING() == TimeOut)
	{
		timeOut_cnt++;
		if(timeOut_cnt == 10)
		{
			timeOut_cnt = 0;
			return retVal = TimeOut;
		}
	}
*/

	//WiFi_POST_ThermoStatus(isOn, isHeating, isReady, tempAct1, tempAct2, mode);
	//WiFi_GET_ThermoStatus(10000, isOn, isHeating, isReady, mode);

	//WiFi_GET_TempDesired(mode, 42000);

	return retVal = Success;
}

void WiFi_InitSettings()
{
	/* Enable LED blinking */
	UART_TransmitString("AT+S.SCFG=blink_led,0\r\n");

	/* Set serial port speed */
	//UART_TransmitString("AT+S.SCFG=console1_speed,921600\r\n");

}

uint8_t WiFi_Send_PING()
{
	char RxBuff[100] = {0};
	InternetState_t InternetState;

	UART_TransmitString("AT+S.PING=www.google.com\r\n");

	UART_RecieveData2((char*)&RxBuff, 200);


	/* If PING Success, print ONLINE to screen*/
	if(SearchSubstring(&RxBuff, "\r\nOK\r\n") == 1)
	{
		ILI9341_Puts(2, 25, "ONLINE", &TM_Font_7x10, ILI9341_COLOR_GREEN, ILI9341_COLOR_YELLOW);
		InternetState = Online;
	}

	/* If PING Success, print ONLINE to screen*/
	if(SearchSubstring(&RxBuff, "Timed out\r\n") == 1)
	{
		ILI9341_Puts(2, 25, "TimeOut...", &TM_Font_7x10, ILI9341_COLOR_GREEN, ILI9341_COLOR_YELLOW);
		InternetState = TimeOut;
	}


	/* If PING Error, print message to screen*/
	if(SearchSubstring(&RxBuff, "ERROR") == 1)
	{
		ILI9341_Puts(2, 25, (char*)&RxBuff, &TM_Font_7x10, ILI9341_COLOR_RED, ILI9341_COLOR_YELLOW);
		InternetState = ERROR;
	}

	return InternetState;
}


/*---------------------------------[HTTP GET requests]-------------------------------------------------*/

void WiFi_GET_ThermoStatus(int minutes, uint8_t isOn, uint8_t isHeating, uint8_t isReady, char *mode)
{
	char TxBuffer[200];
	char RxBuff[2000] = {0};
	int i;

	snprintf(TxBuffer, \
			sizeof(TxBuffer), \
			"AT+S.HTTPGET=data.sparkfun.com,/output/VGvKypXX74umKnMGagVQ.json?gt[timestamp]=now-%dminute\r\n",
			minutes);

	UART_TransmitString((char*)&TxBuffer);
	UART_RecieveData((char*)&RxBuff);


	/* MEGOLDAS:
	 *
	 *  Addig novelgetem a buffer pointert, amig azt nem kapom, amit latni szeretnek
	 *
	 *  */
	char *JSONBuf;
	JSONBuf = strstr(RxBuff, "[{");

	for(i=0; i<15;i++)
		JSONBuf++;
	isHeating = *JSONBuf == '1'? 1 : 0;

	for(i=0; i<11;i++)
			JSONBuf++;
	isOn = *JSONBuf == '1'? 1 : 0;

	for(i=0; i<14;i++)
			JSONBuf++;
	isReady = *JSONBuf == '1'? 1 : 0;

	for(i=0; i<11;i++)
			JSONBuf++;
	mode = *JSONBuf == 'm'? "manual" : "auto";
}

void WiFi_GET_TempDesired(char * mode, int minutes)
{
	char TxBuffer[200];
	char RxBuff[800] = {0};
	uint8_t option;

	uint8_t i = 0;
	uint8_t counterGET = 0;

	uint8_t tempBuff[4];

	if(mode == "manual")
		option = 0;
	if(mode == "auto")
		option = 1;


	switch (option)
	{
		case 0:
			snprintf(TxBuffer, \
						sizeof(TxBuffer), \
						"AT+S.HTTPGET=data.sparkfun.com,/output/5JxQnDyXVGfLoz19nyDy.json?gt[timestamp]=now-%dminute\r\n", minutes);
			break;

		case 1:
			snprintf(TxBuffer, \
					sizeof(TxBuffer), \
					"AT+S.HTTPGET=data.sparkfun.com,/output/n1yOG7ma3dc6Ym8aRd1J.json?gt[timestamp]=now-%dminute\r\n", minutes);
			break;
	}

	//UART_TransmitString((char*)&TxBuffer);
	//UART_TransmitString("AT+S.HTTPGET=data.sparkfun.com,/output/5JxQnDyXVGfLoz19nyDy.json\r\n");
	//UART_RecieveData2((char*)&RxBuff, 200);



	UART_TransmitString("AT+S.HTTPGET=data.sparkfun.com,/output/5JxQnDyXVGfLoz19nyDy.json\r\n");
	UART_RecieveData2((char*)&RxBuff, 800);

	char *strTmp1 = strstr((char*)&RxBuff, "[{\"tempdesired\":\"");



	if(*strTmp1 == '\0')
		return;

	else
	{

		for(i=0; i<17; i++)
			strTmp1++;



		tempBuff[0] = *strTmp1++;
		tempBuff[1] = *strTmp1++;
		tempBuff[2] = *++strTmp1;

		tempDesired = 10*(tempBuff[0]-48) + (tempBuff[1]-48) + ((tempBuff[2]-48)/10.0);



	}



	//ILI9341_Puts(2, 5, strTmp1, &TM_Font_7x10, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE);
}

/* This is done by sending a GET request to a web server.
 * The message which is returned by the web server containt the exact time
 * so this can be parsed out
 */

void WiFi_GET_TimeAndDate()
{
	char TxBuffer[100];
	char RxBuff[2000] = {0};
	int i=0;
	uint8_t tempBuf[4];

	uint8_t min, hour, day, month, year, seconds;

	UART_TransmitString("AT+S.HTTPGET=data.sparkfun.com,/output/5JxQnDyXVGfLoz19nyDy\r\n");
	//UART_RecieveData((char*)&RxBuff);
	UART_RecieveData2((char*)&RxBuff, 400);

	//ILI9341_Puts(5, 75, &RxBuff, &TM_Font_7x10, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE);


	/* #1: Only need string after "Date:" */
	char *strTmp1 = strstr((char*)&RxBuff, "Date: ");

	if(*strTmp1 == '\0')
		RTC_CalendarConfig(0x01, 0x01, 0x01, 0x00, 0x01, 0x01);
	else
	{

		for(i=0; i<11; i++)
			strTmp1++;

		/* DAY */
		tempBuf[0] = *strTmp1++;
		tempBuf[1] = *strTmp1++;
		day = 10*(tempBuf[0]-48) + (tempBuf[1]-48);

		strTmp1++;

		/* MONTH */
		if(*strTmp1 == 'F')
			{ month = 0x02; strTmp1++; strTmp1++; strTmp1++; strTmp1++; }
		if(*strTmp1 == 'S')
		{ month = 0x09; strTmp1++; strTmp1++; strTmp1++; strTmp1++; }
		if(*strTmp1 == 'O')
		{ month = 0x10; strTmp1++; strTmp1++; strTmp1++; strTmp1++; }
		if(*strTmp1 == 'N')
		{ month = 0x11; strTmp1++; strTmp1++; strTmp1++; strTmp1++; }
		if(*strTmp1 == 'D')
		{ month = 0x12; strTmp1++; strTmp1++; strTmp1++; strTmp1++; }
		if(*strTmp1 == 'J')
		{
			strTmp1++;

			if(*strTmp1 == 'a')
				{ month = 0x01;  strTmp1++; strTmp1++; strTmp1++; }
			else if(++*strTmp1 == 'n')
				{ month = 0x06;  strTmp1++; strTmp1++; strTmp1++; }
			else
				{ month = 0x07;  strTmp1++; strTmp1++; strTmp1++; }
		}
		if(*strTmp1 == 'M')
		{
			*strTmp1++;

			if(++*strTmp1 == 'r')
				{ month = 0x03;  strTmp1++; strTmp1++; }
			else
				{ month = 0x05;  strTmp1++; strTmp1++; }
		}
		if(*strTmp1 == 'A')
		{
			strTmp1++;

			if(*strTmp1 == 'u')
				{ month = 0x09;  strTmp1++; strTmp1++; strTmp1++; }
			else
				{ month = 0x04;  strTmp1++; strTmp1++; strTmp1++; }
		}



		/* YEAR */
		tempBuf[0] = *strTmp1++;
		tempBuf[1] = *strTmp1++;
		tempBuf[2] = *strTmp1++;
		tempBuf[3] = *strTmp1++;
		year = 10*(tempBuf[2]-48) + (tempBuf[3]-48); // csak az also 2 kell

		strTmp1++;

		/* HOUR */
		tempBuf[0] = *strTmp1++;
		tempBuf[1] = *strTmp1++;
		hour = 10*(tempBuf[0]-48) + (tempBuf[1]-48) +1; //GMT+1 idozona miatt

		strTmp1++;

		/* MIN */
		tempBuf[2] = *strTmp1++;
		tempBuf[3] = *strTmp1++;
		min = 10*(tempBuf[2]-48) + (tempBuf[3]-48);

		strTmp1++;

		/* SECONDS */
		tempBuf[2] = *strTmp1++;
		tempBuf[3] = *strTmp1;
		seconds = 10*(tempBuf[2]-48) + (tempBuf[3]-48);

		year = dec2bcdhex(year);
		day = dec2bcdhex(day);
		hour = dec2bcdhex(hour);
		min = dec2bcdhex(min);
		seconds = dec2bcdhex(seconds);

		RTC_CalendarConfig(year, month, day, hour, min, seconds);

	}

}

/*---------------------------------[HTTP POST requests]-------------------------------------------------*/

void WiFi_POST_ThermoStatus(uint8_t isOn, uint8_t isHeating, uint8_t isReady, double tempAct1, double tempAct2, char* mode)
{
	char TxBuffer[200];
	char RxBuff[1000] = {0};

	snprintf(TxBuffer, \
			sizeof(TxBuffer), \
			"AT+S.HTTPPOST=data.sparkfun.com,/input/VGvKypXX74umKnMGagVQ?private_key=9Yx4X2ggJehka0welb8R,isheating=%d&ison=%d&isready=%d&mode=%s&tempact1=%.1f&tempact2=%.1f\r\n)", \
			isHeating, isOn, isReady, mode, tempAct1, tempAct2);


	/* RequestBin to test POST request */
	//UART_TransmitString("AT+S.HTTPPOST=requestb.in,/1g4jpoe1,name=Geri&subject=test\r\n");

	UART_TransmitString((char*)&TxBuffer);
	//UART_RecieveData((char*)&RxBuff);

	UART_RecieveData2((char*)&RxBuff, 800);

	//ILI9341_Puts(2, 5, (char*)&RxBuff, &TM_Font_7x10, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE);

}

uint8_t WiFi_POST_TempDesired(double tempDesired, char* mode)
{
	char TxBuffer[200];
	char RxBuff[1000] = {0};
	uint8_t option;

	if(mode == "manual")
		option = 0;
	if(mode == "auto")
		option = 1;

	switch (option)
	{
		case 0:
			snprintf(TxBuffer, \
					sizeof(TxBuffer), \
					"AT+S.HTTPPOST=data.sparkfun.com,/input/5JxQnDyXVGfLoz19nyDy?private_key=7Bx9bE7alqUDM6PJpkZk,tempdesired=%.1f\r\n)", \
					tempDesired);
			break;

		case 1:
			snprintf(TxBuffer, \
					sizeof(TxBuffer), \
					"AT+S.HTTPPOST=data.sparkfun.com,/input/n1yOG7ma3dc6Ym8aRd1J?private_key=Mol8qXV2xNSjJZpVbw8W,tempdesired=%.1f\r\n)", \
					tempDesired);
			break;
	}

	UART_TransmitString(&TxBuffer);

/*
	//UART_RecieveData((char*)&RxBuff);
	UART_RecieveData2((char*)&RxBuff, 800);

	if(SearchSubstring((char*)&RxBuff, "OK") != 1)
	{
		return 0;
	}
*/

	return 1;

	//ILI9341_Puts(2, 5, (char*)&RxBuff, &TM_Font_7x10, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE);

}

/*---------------------------------[HTTP DELETE all requests]-------------------------------------------*/


void WiFi_HTTP_DELETE()
{
	char RxBuff[100] = {0};
	UART_TransmitString("AT+S.HTTPGET=data.sparkfun.com,/input/VGvKypXX74umKnMGagVQ/clear?private_key=9Yx4X2ggJehka0welb8R\r\n");

	//UART_RecieveData((char*)&RxBuff);
	UART_RecieveData2((char*)&RxBuff, 100);
	ILI9341_Puts(2, 5, (char*)&RxBuff, &TM_Font_7x10, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE);
}

void WiFi_HTTP_PUT()
{

}


void WiFi_ConnetToNetwork()
{

	/* Set the SSID to which network I want to connect */
	//UART_TransmitString("AT+S.SSIDTXT=Torzonborz\r\n");
	UART_TransmitString("AT+S.SSIDTXT=MySzifon\r\n");
	HAL_Delay(10);


	/* Set the password of the network (if WPA) */
	//UART_TransmitString("AT+S.SCFG=WiFi_wpa_psk_text,balatongyorok\r\n");
	UART_TransmitString("AT+S.SCFG=WiFi_wpa_psk_text,Phee5xah\r\n");
	HAL_Delay(10);


	/* Set the privacy mode:	 *
	 * 0: none
	 * 1: WEP - Wired Equivalent Privacy
	 * 2: WPA - WiFi Protected Access
	 */
	UART_TransmitString("AT+S.SCFG=WiFi_priv_mode,2\r\n");
	HAL_Delay(10);


	/* WiFi Radio Mode
	 * 1: STA - Station
	 * 2: IBSS - Independent BSS
	 * 3: MiniAP
	 */
	UART_TransmitString("AT+S.SCFG=WiFi_mode,1\r\n");
	HAL_Delay(10);


	/* Use of DHCP Server
		 * 0: Off - in STA mode ip_ipaddr, ip_netmask and ip_gw must be properly set to connect to the AP
		 * 1: On - in STA mode ipaddr,netmask and gw will beprovided by the AP
		 * 2: On&Customize - in MiniAP mode: user can customize the ip_ipaddr of the MiniAP,
		 * 					the ip_address of the client is automatically assigned bythe MiniAP
		 */
	UART_TransmitString("AT+S.SCFG=ip_use_dhcp,1\r\n");
	HAL_Delay(10);


	/* Save configured settings to RAM */
	WiFi_SaveSettings();
	HAL_Delay(10);

	/* Power Mode:
	 * 0: switch to active state,
	 * 1: keep current settings,
	 * 2: switch to powersave state,
	 * 3: switch to sleep state
	 * 4: switch to stadby state
	 *
	 * 	  ...and reset device
	 */
	UART_TransmitString("AT+CFUN=1\r\n");

	/* Waint for LinkUp status */
	//WiFi_ConnectionReady();     -> Not wotking, when device reboots, UART comm is not ON
}

void WiFi_ConnectionReady()
{
	char RxBuff[1500];
	uint8_t isReady = 0;

	/* When connected, the 'WiFi_state' is either:
	 * 7: Joined
	 * 9: 802.11 handshake complete
	 * 10: Ready to transmit (i.e: "Link Up") */
	const char* joined = "WiFi_state=7";
	const char* handshake = "WiFi_state=9";
	const char* linkUp = "WiFi_state=10";

	while(isReady == 0)
	{
		WiFi_GetStatus((char*)&RxBuff);

		if(SearchSubstring((char*)&RxBuff, linkUp) == 1)
			isReady = 1;
		HAL_Delay(10);
	}
}

void WiFi_ScanNetworks()
{
	char RxBuff[2000] = {0};

	UART_TransmitString("AT+S.SCAN\r\n");

	UART_RecieveData((char*)&RxBuff);
	ILI9341_Puts(2, 5, (char*)&RxBuff, &TM_Font_7x10, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE);
}

uint8_t WiFi_isComWorking()
{
	char RxBuff[10] = {0};
	uint8_t retVal = 2;

	UART_TransmitString("AT\r");
	UART_RecieveData2((char*)&RxBuff, 10);

	/* If 'OK' not found, look for 'ERROR' */

	if(SearchSubstring(&RxBuff, "\r\nOK\r\n") == 0)
	{
		if (SearchSubstring(&RxBuff, "ERROR") == 1)
		{
			ILI9341_Puts(2, 5, (char*)&RxBuff, &TM_Font_7x10, ILI9341_COLOR_RED, ILI9341_COLOR_YELLOW);
			retVal = 0;
		}
	}
	else
	{
		ILI9341_Puts(2, 5, "WiFi UART comm OK", &TM_Font_7x10, ILI9341_COLOR_BLUE, ILI9341_COLOR_YELLOW);
		retVal = 1;
	}

	return retVal;
}

void WiFi_RestDeftSettings()
{
	UART_TransmitString("AT&F\r\n");
}

/* Save settings to non-volatile RAM */
void WiFi_SaveSettings()
{
	UART_TransmitString("AT&W\r\n");
}

void WiFi_GetStatus(char* RxBuff)
{
	//char RxBuff[2000] = {0};

	UART_TransmitString("AT+S.STS\r\n");
	UART_RecieveData(&RxBuff);

	//ILI9341_Puts(2, 5, &RxBuff, &TM_Font_7x10, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE);
}


void WiFi_GetAllConfigValues()
{
	char RxBuff[2000] = {0};

	UART_TransmitString("AT&V\r\n");

	UART_RecieveData((char*)&RxBuff);
	ILI9341_Puts(2, 5, (char*)&RxBuff, &TM_Font_7x10, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE);
}

void WiFi_GetSSID()
{
	char RxBuff[200] = {0};

	UART_TransmitString("AT+S.SSIDTXT\r\n");

	UART_RecieveData((char*)&RxBuff);
	ILI9341_Puts(2, 5, (char*)&RxBuff, &TM_Font_7x10, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE);
}

void WiFi_Enable()
{
	char RxBuff[10] = {0};

	UART_TransmitString("AT+S.WiFi=1\r\n");

	UART_RecieveData((char*)&RxBuff);
	ILI9341_Puts(2, 5, (char*)&RxBuff, &TM_Font_7x10, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE);
}

void WiFi_nReset(uint8_t BitVal)
{
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, BitVal);
}

void WiFi_InitGPIO()
{
	/* WiFi /RESET pin config */
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOD_CLK_ENABLE();

	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Pin = GPIO_PIN_14;

	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

void WiFi_PrintAnswer()
{
	char RxBuff[100] = {0};

	UART_RecieveData((char*)&RxBuff);
	ILI9341_Puts(2, 5, (char*)&RxBuff, &TM_Font_7x10, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE);
}

/*
 * reVal	1: string found
 * 			0: not found
 */

uint8_t SearchSubstring(char *String, const char* subString)
{
	uint8_t retVal;

	/* strstr function give back a pointer of the substring foundS	 *
	 * if pointer is NULL, substring has not been found
	 */

	if(strstr(String, subString) == NULL)
		retVal = 0;
	else
		retVal = 1;

	return retVal;
}

static int dec2bcdhex(int x)
{
	uint8_t bcd = 0;

	int temp = x/10;
	int maradek = x - (temp*10);

	bcd = (temp*16) + maradek;

	return bcd;
}
