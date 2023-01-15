//
// Copyright 2022, Raj Bose
//
// This file is part of Nabby-tiny. This is the doorbell unit.
// Nabby-tiny is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// Nabby-tiny is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with Nabby-tiny. If not, see <https://www.gnu.org/licenses/>.
//..

#include <HardwareSerial.h>
// #include <dfplayer.h>

#include <AsyncUDP.h>
#include "arduino.h"
#include <Adafruit_ST7735.h>
#include <ESPmDNS.h>
#include "Parsers.h"
#include <Nabbys.h>
extern String version;
extern Adafruit_ST7735 tft;
extern NabbyContainer allNabbys;

void printParserCommands(void)
{
  Serial.print("\n   ===> Commands:\n");
  Serial.print("  /inf      --- shows information\n");
  Serial.print("  /mdns     --- prints IP addresses of mydoorbell services\n");
  Serial.print("  /mvp,x,x  --- dummy command\n");
}

String IpAddress2String(const IPAddress &ipAddress) // convert IP addr to string
{
  return String(ipAddress[0]) + String(".") +
         String(ipAddress[1]) + String(".") +
         String(ipAddress[2]) + String(".") +
         String(ipAddress[3]);
}

/**************************************************************************/
/*
   Parser for multiplevariable commands. This is a dummy and test command currently
    NOTE:
    - On serial stream will print xx
    - On UDP stream: will return xx
*/
/**************************************************************************/
String multipleVariableParser(char **values, int valueCount, bool udppackets)
{
  Serial.println("   ===> multipleVariableParser:");
  for (int i = 1; i < valueCount; i++)
  {
    Serial.print("  values[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.println(values[i]);
    int jj;
    sscanf(values[i], "%d", &jj);
    Serial.printf("the value is %d\n", jj);
  }
  return ("MVP is done   ");
}

/**************************************************************************/
/*
    Parser for the getInfo command
    NOTE:
    - On serial stream will print the SW version
    - On UDP stream: will return SW version
*/
/**************************************************************************/
String getInfo(char **values, int valueCount, bool udppackets)
{
  char buffer[100];
  if (valueCount > 1)
    Serial.println("   ===> getInfo does not accept parameters.");
  else
  {
    if (!udppackets)
    {
      Serial.print("   ===> Software version Nabby-tiny-doorbell: ");
      Serial.print(version);
    }
  }
  snprintf(buffer, 100, "Nabby-tiny Software version: {%s}   [INF done]", version.c_str());
  return (buffer);
}

/**************************************************************************/
/*
    Parser for the MDNS services scanner command
    NOTE:
    - On serial stream will print the IP addresses
    - On UDP stream:
*/
/**************************************************************************/
String scanMDNSservices(char **values, int valueCount, bool udppackets)
{
  // find Nabbys IP addresses
  char buffer[100];
  int n;
  if (valueCount > 1)
    Serial.println("   ===> getInfo does not accept parameters.");
  else
  {
    Serial.println("Sending mDNS query");
    n = MDNS.queryService("mydoorbell", "udp"); // Send query for mydoorbell services
    Serial.println("mDNS query done");
    snprintf(buffer, 100, "MDNS query to doorbell sent: {%d}   [mdns done]", n);

    //    tft.setCursor(15, 45);
    //    tft.print("                   ");
    tft.fillRect(15, 45, 100, 10, ST7735_BLACK);
    tft.setCursor(15, 45);
    tft.print("Nr Nabb: ");
    tft.println(allNabbys.countNabbys());

    if (n == 0)
    {
      Serial.println("no MDNS 'mydoorbell' services found");
    }
    else
    {
      Serial.printf("Found %d 'mydoorbell' services\n", n);
      for (int i = 0; i < n; i++)
      {
        Serial.printf("IPAddress[%d]: ", i);
        Serial.println(IpAddress2String(MDNS.IP(i)));
        //   Serial.printf("IP address: %s\n", IpAddress2String(MDNS.IP(i)));
      }
    }
  }
  return(buffer);
}
