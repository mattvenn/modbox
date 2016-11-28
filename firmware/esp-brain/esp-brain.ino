/*
current issues

message timeouts if knobs turned too fast, reset lcd fixes but lcd stays running even when message timeouts are ahppening

I think attinys are running at 1mhz, increase this to 8 - done
avrdude -p t88 -P /dev/ttyACM0 -c STK500 -B10 -v  -U lfuse:w:0xee:m

looks like esp hangs with too many mqtt messages - find a way to print out queued messages, or flush? find a way to do rate limiting.



*/
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include "secrets.h"

WiFiClient wclient;
PubSubClient client(wclient);

int num_modules = 0;
typedef struct {
    int id;
    uint8_t * last_msg;
    uint8_t * msg;
    int modchange_msglen;
} MODULE;

// global list of modules
#define MAX_MODULES 50
MODULE * modules;

#define TICK 1000
unsigned long last_tick = 0;

//#define DEBUG_MQTT
//#define DEBUG_I2C

//client callback for MQTT subscriptions
void callback(char* topic, byte* payload, unsigned int len) 
{
  #ifdef DEBUG_MQTT
  Serial.print("topic");
  Serial.println(topic);
  #endif
  if(strcmp(topic,"/modbox/setmod") == 0)
  {
      #ifdef DEBUG_MQTT
      Serial.print("passing msg through [");
      for(int i = 0; i<len; i++)
      {
          if(payload[i] <= 0xF)
              Serial.print("0");
          Serial.print(payload[i], HEX);
      }
      Serial.println("]");
      #endif
      // pass messages straight through to the module
      Wire.beginTransmission(payload[0]);
      Wire.write(payload, len);
      Wire.endTransmission();
  }
  else if(strcmp(topic, "/modbox/reset") == 0)
  {
      ESP.reset();
  }
  else if(strcmp(topic, "/modbox/register") == 0)
  {
      // add the module details to the list to check
      if(num_modules >= MAX_MODULES)
        return;
      #ifdef DEBUG_MQTT
      Serial.println(num_modules);
      Serial.print("registering new module [");
      for(int i = 0; i<len; i++)
      {
          if(payload[i] <= 0xF)
              Serial.print("0");
          Serial.print(payload[i], HEX);
      }
      Serial.println("]");
      #endif
      modules[num_modules].id = payload[0];
      modules[num_modules].modchange_msglen = payload[1];
      modules[num_modules].last_msg = (uint8_t*)malloc(sizeof(uint8_t)*payload[1]);
      modules[num_modules].msg = (uint8_t*)malloc(sizeof(uint8_t)*payload[1]);
      num_modules++;
  }
  else
    Serial.println("no handler for that topic");
}

void setup() 
{
  Wire.begin(); // join i2c bus (address optional for master)
  Wire.setClock(100000);
  Wire.setClockStretchLimit(2000);    // in µs, needed for i2c to work between esp & tiny

  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();

  // allocate memory for modules
  modules = (MODULE*)malloc(MAX_MODULES * sizeof(MODULE));

  // client callback for MQTT subscriptions
  client.setCallback(callback);
  client.setServer(host, 1883);
}

void loop() 
{
  if(millis() - last_tick > TICK)
  {
    Serial.println(last_tick);
    last_tick = millis();
  }
  if (WiFi.status() != WL_CONNECTED) 
  {
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, pass);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      return;
    Serial.println("WiFi connected");
    WiFi.mode(WIFI_STA);
  }
  else if(WiFi.status() == WL_CONNECTED) 
  {
      //not connected - then connect & subscribe
      if(!client.connected())
      {
          if (client.connect("modbox"))
          {
            Serial.println("connected to MQTT");
            client.subscribe("/modbox/setmod"); 
            client.subscribe("/modbox/register"); 
            client.subscribe("/modbox/reset"); 
            client.publish("/modbox/start", "");
          }
      }

    // TODO think this is interfering with the callback now the messagse are long enough. need to be interruptible or the callback needs to wait
    for(int i = 0; i < num_modules; i++)
    {
        MODULE mod = modules[i];
        if(mod.modchange_msglen > 0)
        {
 //         Serial.print("requesting update from module id:");
//          Serial.println(mod.id);
          Wire.requestFrom(mod.id, mod.modchange_msglen); 
          delay(1);
          if(Wire.available())
          {
              int j = 0;
              while(Wire.available() && j < mod.modchange_msglen)
                mod.msg[j++] = Wire.read();

              if(j != mod.modchange_msglen)
              {
                #ifdef DEBUG_I2C
                Serial.print("not enough bytes received: ");
                Serial.println(j);
                #endif
                continue;
              }

              if(mod.msg[0] != mod.id)
              {
                Serial.print("wrong ID received: ");
                Serial.print(mod.msg[0]);
                Serial.print(" expected ");
                Serial.println(mod.id);
                #ifdef DEBUG_I2C
                for(int j=0; j<mod.modchange_msglen; j++)
                {
                   if(mod.msg[j] <= 0xF)
                      Serial.print("0");
                   Serial.print(mod.msg[j], HEX);
                }
                Serial.println("");
                #endif
                continue;
              }

              // must be memcmp because strncmp will stop at 0x00
              if(memcmp(mod.last_msg, mod.msg, mod.modchange_msglen) != 0)
              {
                 #ifdef DEBUG_I2C
                 Serial.print("module id ["); Serial.print(mod.id, HEX); Serial.print("] changed ["); 
                 #endif
                 for(int j=0; j<mod.modchange_msglen; j++)
                 {
                    mod.last_msg[j] = mod.msg[j];
                    #ifdef DEBUG_I2C
                    if(mod.msg[j] <= 0xF)
                        Serial.print("0");
                    Serial.print(mod.msg[j], HEX);
                    #endif
                 }
                 #ifdef DEBUG_I2C
                 Serial.println("]");
                 #endif
                 client.publish("/modbox/modchange", mod.msg, mod.modchange_msglen);
              }
              else
              {
                //Serial.println("no change in module");
              }
          }
          else
            Serial.println("message timeout");
        }
    }
    // delay between talking to modules
    //delay(1);
    client.loop();
  }
  //client.loop();
  // putting this in seems to solve the errors on i2c
  delay(10);
}
