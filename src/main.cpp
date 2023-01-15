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

#include <map>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Nabbys.h>

#define VERSION "15Januari2023b DEV"
String version;

// #define MP3_SERIAL_SPEED 9600  // DFPlayer Mini suport only 9600-baud
// #define MP3_SERIAL_TIMEOUT 100 // average DFPlayer response timeout 100msec..200msec
uint8_t response = 0;
// #define RXD2 16
// #define TXD2 17

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
#define TOPLINE  60   // the top of the line, bounding the box with the bouncing ball

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

NabbyContainer allNabbys(0);

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

  Serial.print("Connecting Wifi - ");
  while (wifiMulti.run() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP addr: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("Nabby-mini-doorbell")) // Host name
  {
    MDNS.addService("doorbell", "udp", 1235); // Announce service on port x
    Serial.println("MDNS responder started");
  }
  Serial.printf("UDP server on port %d\n", 1235);
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
 // Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.print("\nNabby-tiny-doorbell is starting\n");

  connectWifi(); // connect to WiFi access point

  // Add the parser commands to the DynamicCommandParser
  dcp_ser.addParser((char *)"inf", getInfo);
  dcp_ser.addParser((char *)"mvp", multipleVariableParser);
  dcp_ser.addParser((char *)"mdns", scanMDNSservices);
  dcp_ser.addParser((char *)"rng", soundDoorbell);
  printParserCommands();

  dcp_udp.addParser((char *)"inf", getInfo);
  dcp_udp.addParser((char *)"mdns", scanMDNSservices);
  dcp_udp.addParser((char *)"rng", soundDoorbell);

  // start MDNS server
  //  if (MDNS.begin("doorbell Nabby sender")) {
  //   Serial.println("mDNS responder started");
  // }

  IPAddress IPAddress1(192, 168, 178, 13);
  IPAddress IPAddress2(192, 168, 178, 12);
  IPAddress IPAddress3(192, 168, 178, 28);

  struct NBdata nabbydata;

  allNabbys.addNabby(IPAddress1, 11, 12);
  allNabbys.addNabby(IPAddress2, 21, 22);
  allNabbys.addNabby(IPAddress3, 31, 32);

  Serial.printf("start find %d\n", micros());
  nabbydata = allNabbys.findNabby(IPAddress3);
  Serial.printf("end find %d\n", micros());
  Serial.printf("IPAddress3 ==> port = %d, beer= %d\n", nabbydata.port, nabbydata.beer);

  Serial.printf("Nr Nabbys = %d\n", allNabbys.countNabbys());
  allNabbys.removeNabby(IPAddress3);
  Serial.printf("Nr Nabbys = %d\n", allNabbys.countNabbys());
    allNabbys.removeNabby(IPAddress3);
  Serial.printf("Nr Nabbys = %d\n", allNabbys.countNabbys());
  nabbydata = allNabbys.findNabby(IPAddress3);
  Serial.printf("IPAddress3 ==> port = %d, beer= %d\n", nabbydata.port, nabbydata.beer);


  // initialize 1.8" TFT display
  tft.initR(INITR_BLACKTAB); // initialize ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(2);
  tft.setTextSize(2);
  tft.setTextColor(ST7735_YELLOW);
  tft.setCursor(10, 5);
  tft.println("Hallo Raj ");

  tft.setTextSize(1);
  tft.setTextColor(ST7735_YELLOW);
  tft.setCursor(15, 25);
  tft.println(WiFi.SSID());
  tft.setCursor(15, 35);
  tft.println(WiFi.localIP());
  tft.setCursor(15,45);
  tft.print("Nr Nabbys: ");
  tft.println(allNabbys.countNabbys());

  tft.drawLine(1, TOPLINE, 125, TOPLINE, ST7735_RED);
  tft.drawLine(125, TOPLINE, 125, 155, ST7735_RED);
  tft.drawLine(125, 155, 1, 155, ST7735_RED);
  tft.drawLine(1, 155, 1, TOPLINE, ST7735_RED);
  Serial.println("end of setup()");
}

int xCircle = 10;
int yCircle = 60;
int xDir = 1;
int yDir = 1;
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
    tft.drawCircle(xCircle, yCircle, 3, ST7735_BLACK);
    xCircle += xDir;
    if (xCircle > 120)
      xDir = -1;
    if (xCircle < 8)
      xDir = 1;
    yCircle += yDir;
    if (yCircle > 149)
      yDir = -1;
    if (yCircle < TOPLINE+5)
      yDir = 1;

    tft.drawCircle(xCircle, yCircle, 3, ST7735_YELLOW);
  }
}
