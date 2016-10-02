/*
 * Copyright (c) 2013 Nimbits Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either expressed or implied.  See the License for the specific language governing permissions and limitations under the License.
 */


#ifndef _Nimbits_h
#define _Nimbits_h
#include "Arduino.h"
#include "PString.h"
#include <EthernetClient.h>

#define MAXURL 30
#define MAXTIME 1000

String floatToString(double number, uint8_t digits);

class Nimbits {
  public:
    Nimbits(String instance, String ownerEmail, String accessKey);
    String getTime();
    void addEntityIfMissing(char *key, char *name, char *parent, int entityType, char *settings);
    void recordValue(double value,int decpoint, String note, String pointId);
	void recordtest(char *packet);
  private:
	char nURL[MAXURL];

};




#endif /* _Nimbits_h */