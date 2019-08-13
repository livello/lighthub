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
#include "options.h"
#include "abstractout.h"

#define S_NOTFOUND  0
#define S_SETnCMD 0
#define S_CMD 1
#define S_SET 2
#define S_TEMP 3
#define S_MODE 4
#define S_SETPOINT 5
#define S_POWER 6
#define S_VOL 7
#define S_HEAT 8
#define S_HSV 9
#define S_RGB 10
#define S_RPM 11

#define CH_DIMMER 0   //DMX 1 ch
#define CH_RGBW   1   //DMX 4 ch
#define CH_RGB    2   //DMX 3 ch
#define CH_PWM    3   //PWM output directly to PIN
#define CH_MODBUS 4   //Modbus AC Dimmer
#define CH_THERMO 5   //Simple ON/OFF thermostat
#define CH_RELAY  6   //ON_OFF relay output
#define CH_GROUP   7  //Group pseudochannel
#define CH_VCTEMP  8  //Vacom PID regulator
#define CH_VC      9  //Vacom modbus motor regulator
#define CH_AC_HAIER 10  //AC Haier
#define CH_SPILED 11
#define CH_WHITE   127//

#define CMD_NUM 0
#define CMD_UNKNOWN  -1
#define CMD_JSON -2
#define CMD_RGB  -3
#define CMD_HSV  -4

#define CMD_ON  1
#define CMD_OFF 2
#define CMD_RESTORE 3 //on only if was turned off by CMD_HALT
#define CMD_TOGGLE 4
#define CMD_HALT 5    //just Off
#define CMD_XON 6     //just on
#define CMD_XOFF 7    //off only if was previously turned on by CMD_XON
#define CMD_UP 8      //increase
#define CMD_DN 9      //decrease
#define CMD_SET 0xe
#define CMD_CURTEMP 0xf
#define CMD_MASK 0xf
#define FLAG_MASK 0xf0


#define SEND_COMMAND 16
#define SEND_PARAMETERS 32
#define SEND_RETRY 64
#define SEND_DEFFERED 128


//#define CMD_REPORT 32

#define I_TYPE 0 //Type of item
#define I_ARG  1 //Chanel-type depended argument or array of arguments (pin, address etc)
#define I_VAL  2 //Latest preset (int or array of presets)
#define I_CMD  3 //Latest CMD received
#define I_EXT  4 //Chanell-depended extension - array

#define MODBUS_CMD_ARG_ADDR 0
#define MODBUS_CMD_ARG_REG 1
#define MODBUS_CMD_ARG_MASK 2
#define MODBUS_CMD_ARG_MAX_SCALE 3
#define MODBUS_CMD_ARG_REG_TYPE 4

#define MODBUS_COIL_REG_TYPE 0
#define MODBUS_DISCRETE_REG_TYPE 1
#define MODBUS_HOLDING_REG_TYPE 2
#define MODBUS_INPUT_REG_TYPE 3




#include "aJSON.h"

extern aJsonObject *items;
extern short thermoSetCurTemp(char *name, float t);

int txt2cmd (char * payload);

typedef union
{
  long int aslong;
  struct
      {
        int16_t h;
        int8_t  s;
        int8_t  v;
      };
} HSVstore;


typedef union
{
  long int aslong;
  struct
      {
        int8_t  r;
        int8_t  g;
        int8_t  b;
        int8_t  w;
      };
} RGBWstore;

class Item
{
  public:
  aJsonObject *itemArr, *itemArg,*itemVal;
  uint8_t itemType;
  abstractOut * driver;


  Item(char * name);
  Item(aJsonObject * obj);
  ~Item();

  boolean isValid ();
  boolean Setup();
  virtual int Ctrl(short cmd, short n=0, int * Parameters=NULL, boolean send=true, int suffixCode=0, char* subItem=NULL);
  virtual int Ctrl(char * payload, boolean send=true, char * subItem=NULL);

  int getArg(short n=0);
  //boolean getEnableCMD(int delta);
  //int getVal(short n); //From VAL array. Negative if no array
  long int getVal(); //From int val OR array
  uint8_t getCmd();
  void setCmd(uint8_t cmdValue);
  short getFlag   (short flag=FLAG_MASK);
  void setFlag   (short flag);
  void clearFlag (short flag);
  //void setVal(uint8_t n, int par);
  void setVal(long int par);
  //void copyPar (aJsonObject *itemV);
  inline int On (){return Ctrl(CMD_ON);};
  inline int Off(){return Ctrl(CMD_OFF);};
  inline int Toggle(){return Ctrl(CMD_TOGGLE);};
  int Poll();
  int SendStatus(int sendFlags);

  protected:
  short cmd2changeActivity(int lastActivity, short defaultCmd = CMD_SET);
  int VacomSetFan (int8_t  val, int8_t  cmd=0);
  int VacomSetHeat(int addr, int8_t  val, int8_t  cmd=0);
  int modbusDimmerSet(int addr, uint16_t _reg, int _regType, int _mask, uint16_t value);
  void mb_fail();
  int isActive();
  void Parse();
  int checkModbusDimmer();
  int checkModbusDimmer(int data);
  boolean checkModbusRetry();
  void sendDelayedStatus();

  int checkFM();

};


/*

class PooledItem : public Item
{
  public:
  virtual int onContactChanged() = 0;
  virtual void Idle ();
  protected:
  int PoolingInterval;
  unsigned long next;
  virtual int Pool() =0;

};





class Vacon : public Item
{
public:
int Pool ();
virtual int Ctrl(short cmd, short n=0, int * Par=NULL);
protected:
};

*/
