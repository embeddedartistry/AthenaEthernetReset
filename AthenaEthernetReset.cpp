/*
 * AthenaEthernetReset.cpp - Athena Bootloader Reset Server library
 * Copyright (c) 2012 Stylianos Tsampas.  All right reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "AthenaEthernetReset.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

void AthenaEthernetReset::stdResponse(const char* msg)
{
	_client.println("HTTP/1.1 200 OK");
	_client.println("Content-Type: text/html");
	_client.println("Connnection: close");
	_client.println();
	_client.println("<!DOCTYPE HTML>");
	_client.println("<html>");
	_client.println(msg);
	_client.println("</html>");
}

void AthenaEthernetReset::watchdogReset()
{
	delay(10);
	_client.stop();
	wdt_disable();
	wdt_enable(WDTO_2S);
	while(1)
		;
}

/******************************************************************************
 * Constructors / Destructors
 ******************************************************************************/
AthenaEthernetReset::AthenaEthernetReset(int port)
{
	_server = new EthernetServer(port);
	String path = EEPROM.readPass();
	path.toCharArray(_path, 20);
}

/******************************************************************************
 * User API
 ******************************************************************************/

void AthenaEthernetReset::begin()
{
	Ethernet.begin(EEPROM.readMAC(), EEPROM.readIP(), EEPROM.readGW(), EEPROM.readGW(),
				   EEPROM.readSN());

	_server->begin();
	ETHERNET_DEBUG(Serial.print("Server is at "); Serial.println(Ethernet.localIP());
				   Serial.print("Gw at "); Serial.println(EEPROM.readGW());
				   Serial.print("Password: "); Serial.println(_path);)
}

void AthenaEthernetReset::check()
{
	/* 25 is the the maximum command lenth plus
	 * the standart GET and HTTP prefix and postfix */
	char http_req[strlen(_path) + 25];
	_client = _server->available();
	if(_client)
	{
		ETHERNET_DEBUG(Serial.println("new reset client");)
		while(_client.connected())
		{
			if(_client.available())
			{
				char c;
				char* url = http_req;
				while((c = _client.read()) != '\n')
				{
					*url = c;
					url++;
				}
				ETHERNET_DEBUG(*url = '\0';)
				url = http_req + 5;
				_client.flush();
				ETHERNET_DEBUG(Serial.println(url);)
				if(!strncmp(url, _path, strlen(_path)))
				{
					url += (strlen(_path) + 1);
					if(!strncmp(url, "reset", 5))
					{
						stdResponse("Arduino will be doing a normal reset in 2 seconds");
						watchdogReset();
					}
					else if(!strncmp(url, "reprogram", 9))
					{
						stdResponse("Arduino will reset for reprogramming in 2 seconds");
						EEPROM.writeImgBad();
						watchdogReset();
					}
					else
						stdResponse("Wrong command");
				}
				else
					stdResponse("Wrong path");
				break;
			}
		}
		delay(10);
		_client.stop();
		ETHERNET_DEBUG(Serial.println("reset client disonnected");)
	}
}
