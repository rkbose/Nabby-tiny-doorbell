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
#include "Parsers.h"
#include <AsyncUDP.h>
#include "arduino.h"
#include <Adafruit_ST7735.h>

// extern DFPlayer mp3;
extern String version;
extern Adafruit_ST7735 tft;

void printParserCommands(void)
{
  Serial.print("\n   ===> Commands:\n");
  Serial.print("  /inf      --- shows information\n");
  Serial.print("  /mvp,x,x  --- dummy command\n");
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
      Serial.print("   ===> Software version Nabby-tiny: ");
      Serial.print(version);
      tft.setTextSize(1);
      tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
      tft.setCursor(5, 70);
      tft.printf("Run time: %d mSec\n", millis());
    }
  }
  snprintf(buffer, 100, "Nabby-tiny Software version: {%s}   [INF done]", version.c_str());
  return (buffer);
}

