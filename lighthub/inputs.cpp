/* Copyright © 2017-2018 Andrey Klimov. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Homepage: http://lazyhome.ru
GIT:      https://github.com/anklimov/lighthub
e-mail    anklimov@gmail.com

*/

#include "inputs.h"
#include "aJSON.h"
#include "item.h"
#include <PubSubClient.h>
#include <DHT.h>

extern PubSubClient mqttClient;
//DHT dht();

Input::Input(char * name) //Constructor
{
  if (name)
       inputObj= aJson.getObjectItem(inputs, name);
  else inputObj=NULL;

     Parse();
}


Input::Input(int pin) //Constructor
{
 // TODO
}


 Input::Input(aJsonObject * obj) //Constructor
{
  inputObj= obj;
  Parse();

}


boolean Input::isValid ()
{
 return  (pin && store);
}

void Input::Parse()
{
    store = NULL;
    inType = 0;
    pin = 0;

    if (inputObj && (inputObj->type == aJson_Object)) {
        aJsonObject *s;

        s = aJson.getObjectItem(inputObj, "T");
        if (s) inType = s->valueint;

        pin = atoi(inputObj->name);

        s = aJson.getObjectItem(inputObj, "S");
        if (!s) {
            Serial.print(F("In: "));
            Serial.print(pin);
            Serial.print(F("/"));
            Serial.println(inType);
            aJson.addNumberToObject(inputObj, "S", 0);
            s = aJson.getObjectItem(inputObj, "S");
        }

        if (s) store = (inStore *) &s->valueint;
    }
}

int Input::poll() {
    if (!isValid()) return -1;
    if (inType & IN_PUSH_ON)
        contactPoll();
    else if (inType & IN_DHT22)
        dht22Poll();
    return 0;
}

void Input::dht22Poll() {
    if(store->nextPollMillis>millis())
        return;
    Serial.print(" dhtpoll ");
    if(!store->PollDelaySeconds) {
        aJsonObject *pollDelay = aJson.getObjectItem(inputObj, "D");
        if(pollDelay){
            store->PollDelaySeconds = atoi(pollDelay->valuestring);
        }
        if(!store->PollDelaySeconds||store->PollDelaySeconds<=1)
            store->PollDelaySeconds = DHT_POLL_DELAY_DEFAULT;
    }

    DHT dht(pin, DHT22);
    float currentTemp = dht.readTemperature();
    float currentHumidity = dht.readHumidity();
    if(currentTemp!=store->currentValue||currentHumidity!=store->currentValueExtra) {
        store->currentValue = currentTemp;
        store->currentValueExtra = currentHumidity;
        onDht22Changed(currentTemp,currentHumidity);
    }


    store->nextPollMillis = millis() + store->PollDelaySeconds*1000 ;
}

void Input::contactPoll() {
    boolean currentInputState;
    uint8_t inputPinMode, inputOnLevel;
    if (inType & IN_ACTIVE_HIGH) {
        inputOnLevel = HIGH;
        inputPinMode = INPUT;
    } else {
        inputOnLevel = LOW;
        inputPinMode = INPUT_PULLUP;
    }
    pinMode(pin, inputPinMode);
    currentInputState = (digitalRead(pin) == inputOnLevel);
    if (currentInputState != store->currentValue) // value changed
    {
        if (store->bounce) store->bounce = store->bounce - 1;
        else //confirmed change
        {
            onContactChanged(currentInputState);
            store->currentValue = currentInputState;
        }
    } else // no change
        store->bounce = SAME_STATE_ATTEMPTS;
}

void Input::onContactChanged(int val)
{
  Serial.print(F("IN:"));  Serial.print(pin);Serial.print(F("="));Serial.println(val);
  aJsonObject * item = aJson.getObjectItem(inputObj,"item");
  aJsonObject * scmd = aJson.getObjectItem(inputObj,"scmd");
  aJsonObject * rcmd = aJson.getObjectItem(inputObj,"rcmd");
  aJsonObject * emit = aJson.getObjectItem(inputObj,"emit");

  if (emit)
  {

  if (val)
            {  //send set command
               if (!scmd) mqttClient.publish(emit->valuestring,"ON",true); else  if (strlen(scmd->valuestring)) mqttClient.publish(emit->valuestring,scmd->valuestring,true);
            }
       else
            {  //send reset command
              if (!rcmd) mqttClient.publish(emit->valuestring,"OFF",true);  else  if (strlen(rcmd->valuestring)) mqttClient.publish(emit->valuestring,rcmd->valuestring,true);
            }
  }

  if (item)
  {
  Item it(item->valuestring);
  if (it.isValid())
      {
       if (val)
            {  //send set command
               if (!scmd) it.Ctrl(CMD_ON,0,NULL,true); else if   (strlen(scmd->valuestring))  it.Ctrl(scmd->valuestring,true);
            }
       else
            {  //send reset command
               if (!rcmd) it.Ctrl(CMD_OFF,0,NULL,true); else if  (strlen(rcmd->valuestring))  it.Ctrl(rcmd->valuestring,true);
            }
      }
  }
}

void Input::onDht22Changed(float temp,float humidity) {
    Serial.print(F("IN:"));  Serial.print(pin);Serial.print(F(" DHT22 type. T="));Serial.print(store->currentValue);
    Serial.print(F("°C H="));Serial.print(store->currentValueExtra);Serial.print(F("%"));
    aJsonObject * item = aJson.getObjectItem(inputObj,"item");
    aJsonObject * emit = aJson.getObjectItem(inputObj,"emit");
    if (emit && temp && humidity) {
        publishMqtt(temp, emit, "T");
        publishMqtt(humidity, emit, "H");
    }
}

void Input::publishMqtt(float value, const aJsonObject *emit, const char* addr) const {
    char valstr[10] = "NIL";
    char addrstr[32] = "NIL";
    sprintf(valstr, "%2.1f", value);
    sprintf(addrstr, "%s%s", emit->valuestring,addr);
    mqttClient.publish(addrstr, valstr);
}
