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

//client callback for MQTT subscriptions
void callback(char* topic, byte* payload, unsigned int len) 
{
  Serial.print("topic");
  Serial.println(topic);
  if(strcmp(topic,"/modbox/setmod") == 0)
  {
      Serial.print("passing msg through [");
      for(int i = 0; i<len; i++)
          Serial.print(payload[i], HEX);
      Serial.println("]");
      // pass messages straight through to the module
      Wire.beginTransmission(payload[0]);
      Wire.write(payload, len);
      Wire.endTransmission();
  }
  else if(strcmp(topic, "/modbox/register") == 0)
  {
      // add the module details to the list to check
      if(num_modules >= MAX_MODULES)
        return;
      Serial.println(num_modules);
      Serial.print("registering new module [");
      for(int i = 0; i<len; i++)
          Serial.print(payload[i], HEX);
      Serial.println("]");
      modules[num_modules].id = payload[0];
      modules[num_modules].modchange_msglen = payload[1];
      modules[num_modules].last_msg = (uint8_t*)malloc(sizeof(uint8_t)*payload[1]);
      modules[num_modules].msg = (uint8_t*)malloc(sizeof(uint8_t)*payload[1]);
      num_modules++;
      Serial.println("done");
  }
  else
    Serial.println("no handler for that topic");
}

void setup() 
{
  Wire.begin(); // join i2c bus (address optional for master)
  //  Wire.setClock(50000);
  Wire.setClockStretchLimit(1500);    // in µs, needed for i2c to work on esp

  WiFi.mode(WIFI_STA);
  Serial.begin(9600);
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
            client.publish("/modbox/start", "");
          }
      }

    for(int i = 0; i < num_modules; i++)
    {
        MODULE mod = modules[i];
        if(mod.modchange_msglen > 0)
        {
 //         Serial.print("requesting update from module id:");
//          Serial.println(mod.id);
          Wire.requestFrom(mod.id, mod.modchange_msglen); 
          if(Wire.available() == mod.modchange_msglen)
          {
              int j = 0;
              while(Wire.available() && j < mod.modchange_msglen)
                mod.msg[j++] = Wire.read();

              if(j != mod.modchange_msglen)
              {
                Serial.print("not enough bytes received: ");
                Serial.println(j);
                continue;
              }

              if(mod.msg[0] != mod.id)
              {
                Serial.print("wrong ID received");
                continue;
              }
                
              if(strncmp((char*)mod.last_msg, (char*)mod.msg, mod.modchange_msglen) != 0)
              {
                 Serial.print("module id ["); Serial.print(mod.id, HEX); Serial.print("] changed ["); 
                 for(int j=0; j<mod.modchange_msglen; j++)
                 {
                    mod.last_msg[j] = mod.msg[j];
                    Serial.print(mod.msg[j], HEX);
                 }
                 Serial.println("]");
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
  }
  client.loop();
}
