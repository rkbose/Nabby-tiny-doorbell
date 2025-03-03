//
// Copyright 2022, Raj Bose
//
// This file is part of Nabby-tiny. This is the doorbell unit.
// Nabby-tiny is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// Nabby-tiny is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with Nabby-tiny. If not, see <https://www.gnu.org/licenses/>.
//

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

/**************************************************************************/
/*
   Prints explanation on all commands possible on serial.
*/
/**************************************************************************/
void printParserCommands(void)
{
  Serial.printf("\n   ===> Commands:\n");
  Serial.print("      /inf         --- shows information\n");
  Serial.print("      /?           --- print commands\n");
  Serial.print("      /mdns        --- scans mydoorbell services and prints found IP addresses\n");
  Serial.print("      /rng         --- sends ring cmd to all found doorbell services\n");
  Serial.print("      /chk,x,x,x,x --- check if IP address is registered\n");
  Serial.print("      /mvp,x,x     --- dummy command");
}

/**************************************************************************/
/*
   Converts IP address to string.
*/
/**************************************************************************/
String IpAddress2String(const IPAddress &ipAddress) 
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
  Serial.printf("\n   ===> multipleVariableParser:\n");
  for (int i = 1; i < valueCount; i++)
  {
    Serial.printf("      values[%d]: %s\n", i, values[i]);
    int jj;
    sscanf(values[i], "%d", &jj);
    Serial.printf("      the int value is %d\n", jj);
  }
  return ("   MVP is done   ");
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
    Serial.printf("\n   ===> getInfo does not accept parameters.");
  else
  {
    if (!udppackets)
    {
      Serial.printf("\n   ===> Software version Nabby-tiny-doorbell: ");
      Serial.print(version);
    }
  }
  snprintf(buffer, 100, "Software version Nabby-tiny-doorbell: {%s}   [INF done]", version.c_str());
  return (buffer);
}

/**************************************************************************/
/*
    Parser for the printcmd command
    NOTE:
    - On serial stream will print the command list
    - On UDP stream: will return SW version
*/
/**************************************************************************/
String printcmds(char **values, int valueCount, bool udppackets)
{
  char buffer[100];
  if (valueCount > 1)
    Serial.printf("\n   ===> printcmds does not accept parameters.");
  else
  {
    if (!udppackets)
    {
      printParserCommands();
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
    Serial.printf("\n   ===> scanMDNSservices does not accept parameters.");
  else
  {
    Serial.printf("\n   ===> Sending mDNS query\n");
    n = MDNS.queryService("mydoorbell", "udp"); // Send query for mydoorbell services
    Serial.printf("        mDNS query sent\n");
    snprintf(buffer, 100, "MDNS query sent. Nabbys found: %d", n);

    if (n == 0)
    {
      Serial.println("         no MDNS 'mydoorbell' services found");
    }
    else
    {
      Serial.printf("      Found %d 'mydoorbell' services\n", n);
      allNabbys.removeAll(); // remove all Nabbys from list
      tft.fillRect(15, 45, 100, 50, ST7735_BLACK);
      tft.setCursor(15, 45);
      tft.printf("Nr Nabbys: %d\n", n);

      for (int i = 0; i < n; i++)
      {
        Serial.printf("      IPAddress[%d]: ", i);
        Serial.print(IpAddress2String(MDNS.IP(i)));
        Serial.printf("   - %d \n", MDNS.port(i));
        allNabbys.addNabby(MDNS.IP(i), MDNS.port(i), 0);
        tft.printf("     ");
        tft.println(IpAddress2String(MDNS.IP(i)));
      }
    }
  }
  // tft.fillRect(15, 45, 100, 10, ST7735_BLACK);
  // tft.setCursor(15, 45);
  // tft.print("Nr Nabbys: ");
  // tft.println(allNabbys.countNabbys());
  // tft.print(IpAddress2String(MDNS.IP(0)));
  return (buffer);
}

/**************************************************************************/
/*
    Parser for the soundDoorbell command
    NOTE:
    - On serial stream will print a confirmation
    - On UDP stream: will return a confirmation
*/
/**************************************************************************/
String soundDoorbell(char **values, int valueCount, bool udppackets)
{
  char buffer[100];
  int n = allNabbys.soundBell();
  Serial.printf("\n   ===> Sent command to %d doorbell(s)", n);
  snprintf(buffer, 100, "Sent command to %d doorbell(s)", n);
  return (buffer);
}

/**************************************************************************/
/*
   Parser for checkIpAddress command. This will checkk if the IP address
   supplied by 4 parameters is registred in the Nabby list.
    NOTE:
    - On serial stream will print YES or NO
    - On UDP stream: will return YES or NO
*/
/**************************************************************************/
String checkIpAddress(char **values, int valueCount, bool udppackets)
{
  if (valueCount != 5)
  {
    Serial.printf("\n   ===> checkIpAddress requires 4 parameters");
    return ("   checkIpAddress is done   ");
  }

  int octet1, octet2, octet3, octet4;
  sscanf(values[1], "%d", &octet1);
  sscanf(values[2], "%d", &octet2);
  sscanf(values[3], "%d", &octet3);
  sscanf(values[4], "%d", &octet4);
  IPAddress IPA(octet1, octet2, octet3, octet4);
  if (allNabbys.existNabby(IPA))
   Serial.printf("\n   ===> IP address %s found", IPA.toString().c_str()); 
  else
Serial.printf("\n   ===> IPA address %s not found",IPA.toString().c_str()); 
  return ("   checkIpAddress is done");
}
