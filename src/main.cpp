//
// Nabby-tiny-doorbell ESP32 based by RKB
// Board used: ESP32Dev module. Baudrate: 115200
// (COM4 on ACER and HP, right-usb-poort; not all usb-cables are OK)
//
// Copyright 2022, Raj Bose
//
// This file is part of Nabby-tiny. This is the doorbell unit.
// Nabby-tiny is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// Nabby-tiny is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with Nabby-tiny. If not, see <https://www.gnu.org/licenses/>.
//..

#include <HardwareSerial.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <AsyncUDP.h>
#include <Secret.h>
#include <ESPmDNS.h>

#include <DynamicCommandParser.h>
#include <Parsers.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define VERSION "1Januari2023b DEV"
String version;

#define MP3_SERIAL_SPEED 9600  // DFPlayer Mini suport only 9600-baud
#define MP3_SERIAL_TIMEOUT 100 // average DFPlayer response timeout 100msec..200msec
// DFPlayer mp3;                  // connect DFPlayer RX-pin to GPIO15(TX) & DFPlayer TX-pin to GPIO13(RX)
uint8_t response = 0;
#define RXD2 16
#define TXD2 17

// the 1.8 inch TFT display connections
// Display controller: ST7735
// 1.8" SPI TFT LCD
//     LED        - 3V3
//     SCK        - GPIO 18
//     SDA (MOSI) -  GPIO 23
//     A0(DC)     - GPIO 13
//     RESET      -  GPIO 14
//     CS         - GPIO 12
//     GND        - 0V
//     VCC        - 3V3
// (MISO - pin 19 is not used)

#define TFT_CS 12  // Chip select line for TFT display
#define TFT_DC 13  // Data/command line for TFT
#define TFT_RST 14 // Reset line for TFT (or connect to +5V)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Initialize the command parsers using the start, end, delimiting characters
// A seperate parser is instantiated for UDP. This is strictly not neccesry, but had adavateges like:
//    - the udp parser can have different commands (or delimiting chars) then the serial parser, or a subset/superset of commands
//    - with one parser there is a very small chance serial commands mix up with udp commands. Seperation resolves this
// Downside is memory space.
//
DynamicCommandParser dcp_ser('/', 0x0D, ','); // parser for serial
DynamicCommandParser dcp_udp('/', 0x0D, ','); // parser for udp

AsyncUDP udp;
WiFiMulti wifiMulti;

void connectWifi()
{
  WiFi.mode(WIFI_STA); // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.disconnect();
  delay(100);
  /*
    int n = WiFi.scanNetworks(); // WiFi.scanNetworks will return the number of networks found
    Serial.print("WiFi scan done: ");
    if (n == 0)
    {
      Serial.println("no networks found");
    }
    else
    {
      Serial.printf("nr. networks = %d\n", n);
      for (int i = 0; i < n; ++i)
      {
        // Print SSID and RSSI for each network found
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
        delay(10);
      }
    }
    */

  wifiMulti.addAP(SSID1, PSW1);
  wifiMulti.addAP(SSID2, PSW2);
  wifiMulti.addAP(SSID3, PSW3);

  Serial.print("Connecting Wifi -");
  while (wifiMulti.run() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to network");

  Serial.print("WiFi connected: ");
  Serial.print(WiFi.SSID());
  Serial.print(";   IP addr: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("Nabby-mini-doorbell")) // Host name
  {
    MDNS.addService("doorbell", "udp", 1000); // Announce service on port x
    Serial.println("MDNS responder started");
  }
  Serial.printf("UDP server on port %d\n", 1000);
} // end of connectWifi()

void handleUdp()
{
  if (udp.listen(1235))
  {
    udp.onPacket([](AsyncUDPPacket packet)
                 {
                   //  Serial.printf("udp rcv: %s\n",(char *)packet.data());
                   dcp_udp.append(&packet);
                   //                  dcp_udp.append(&packet);
                   //      String reply = dcp_udp.append(myString);
                 });
  }
}

long int myTime;

void setup()
{
  myTime = 0;
  version = VERSION;
  Serial.begin(115200); // debug interface
                        //  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // MP3 interface
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial.print("\nNabby-tiny is starting\n");

  //  Serial.println("Serial2 Txd is on pin: " + String(TXD2));
  //  Serial.println("Serial2 Rxd is on pin: " + String(RXD2));

  /*
    mp3.begin(Serial2, MP3_SERIAL_TIMEOUT, DFPLAYER_MINI, false); // DFPLAYER_MINI see NOTE, false=no response from module after the command

    delay(2000);
    mp3.stop(); // if player was runing during ESP8266 reboot
    delay(100);
    mp3.reset(); // reset all setting to default
    delay(100);
    mp3.setSource(2); // 1=USB-Disk, 2=TF-Card, 3=Aux, 4=Sleep, 5=NOR Flash
   // delay(500);
    mp3.wakeup(2); // exit standby mode & initialize sourse 1=USB-Disk, 2=TF-Card, 3=Aux, 5=NOR Flash
    delay(500);
    mp3.setVolume(15); // 0..30, module persists volume on power failure
   // delay(500);
    mp3.playTrack(2); // play track #1, donâ€™t copy 0003.mp3 and then 0001.mp3, because 0003.mp3 will be played firts
   // delay(500);
  */
  // Serial2.begin(9600, SERIAL_8N1, 16, 17);  // MP3 interface

  connectWifi(); // connect to WiFi access point

  // Add the parser commands to the DynamicCommandParser
  dcp_ser.addParser("inf", getInfo);
  dcp_ser.addParser("mvp", multipleVariableParser);
  dcp_ser.addParser("tra", selectTrack);
  dcp_ser.addParser("vol", setVolume);
  printParserCommands();

  dcp_udp.addParser("inf", getInfo);
  // initialize TFT display
  // Initialize 1.8" TFT
  tft.initR(INITR_BLACKTAB); // initialize ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);

  tft.setRotation(2);
  tft.setTextSize(2);
  tft.setTextColor(ST7735_YELLOW);
  tft.setCursor(10, 5);
  tft.println("Hallo Raj ");

  Serial.println("end of setup()");
}

int xCircle = 0;
int xDir = 1;
char c;
void loop()
{
  // Serial2.print(".");
  // Serial.printf("i: %d\n", i);
  while (Serial.available() > 0)
  {
    c = Serial.read();
    Serial.print(c);
    dcp_ser.appendChar(c);
  }
  handleUdp(); // handle and parse commands received via UDP
  
  if ((millis() - myTime) > 15)
  {
    myTime = millis();
    tft.drawCircle(xCircle, 30, 3, ST7735_BLACK);
    xCircle += xDir;
    if (xCircle > 125)
      xDir = -1;
    if (xCircle < 1)
      xDir = 1;
    tft.drawCircle(xCircle, 30, 3, ST7735_YELLOW);
  }
}
