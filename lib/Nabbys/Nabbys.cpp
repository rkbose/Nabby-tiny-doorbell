// Copyright 2022, Raj Bose
//
// This file is part of Nabby-tiny. This is the doorbell unit.
// Nabby-tiny is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// Nabby-tiny is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with Nabby-tiny. If not, see <https://www.gnu.org/licenses/>.
//..

#include <HardwareSerial.h>
#include <string.h>
#include <Nabbys.h>


void NabbyContainer::addNabby(IPAddress ipa, int aap, int beer)
{ struct NBdata nabbyTemp;
  nabbyTemp.aap = aap;
  nabbyTemp.beer = beer;

  Nabbys.insert({ipa, nabbyTemp});
}  

void NabbyContainer::removeNabby(IPAddress ipa)
{
 Nabbys.erase(ipa);
}

bool NabbyContainer::existNabby(IPAddress ipa)
{
  auto a = Nabbys.find(ipa);
  if (a==Nabbys.end()) return(false);
  else return (true);
}

int NabbyContainer::countNabbys(void)
{
 return(Nabbys.size());
}

NBdata NabbyContainer::findNabby(IPAddress ipa)
{
  NBdata result;
  auto a = Nabbys.find(ipa);
  
  if (a != Nabbys.end())
   {
    result.aap = a->second.aap;
    result.beer = a->second.beer;
   }
  else result.aap = result.beer = 0;
  return(result);  
}


//  Serial.printf("value IPAddress2= %d\n", Nabbys[IPAddress2]);
  /*
    Serial.println("Iterator");
    std::map<IPAddress, NBdata>::iterator it;
    for (it = Nabbys.begin(); it != Nabbys.end(); it++)
    {
      Serial.print(IpAddress2String(it->first));
      Serial.print(": ");
      Serial.printf("aap: %d\n", it->second.aap);
      //Serial.println();
    }
    */