//
// Copyright 2022, Raj Bose
//
// This file is part of Nabby-tiny. This is the doorbell unit.
// Nabby-tiny is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// Nabby-tiny is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with Nabby-tiny. If not, see <https://www.gnu.org/licenses/>.
//..

#ifndef NABBYS_H
#define NABBYS_H

#include <map>
#include <WiFi.h>
// #include <WiFiMulti.h>
// #include <AsyncUDP.h>
// #include <ESPmDNS.h>

struct NBdata
{
    int aap;
    int beer;
};

class NabbyContainer
{
public:
    NabbyContainer(int jk)
    {
   //    Nabbys = std::map<IPAddress, NBdata>;
    }
    ~NabbyContainer()
    {
    }

void addNabby(IPAddress ipa, int aap, int beer);
void removeNabby(IPAddress ipa);
bool existNabby(IPAddress ipa);
NBdata findNabby(IPAddress ipa);
int countNabbys(void);

private:
 std::map<IPAddress, NBdata> Nabbys;
};

#endif
