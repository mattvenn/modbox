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
    String last_msg;
    int msg_len;
} MODULE;

// global list of modules
#define MAX_MODULES 50
MODULE * modules;

//client callback for MQTT subscriptions
void callback(char* topic, byte* payload, unsigned int len) 
{
  Serial.print("topic");
  Serial.println(topic);
  if(topic == "setmod")
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
  if(topic == "register")
  {
      // add the module details to the list to check
      if(num_modules >= MAX_MODULES)
        return;
      Serial.println("registering new module");
      modules[num_modules].id = payload[0];
      modules[num_modules].msg_len = payload[1];
      modules[num_modules].last_msg = "";
      num_modules++;
  }
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
    WiFi.begin(ssid, password);

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
          }
      }

    for(int i = 0; i < num_modules; i++)
    {
        MODULE mod = modules[i];
        if(mod.msg_len > 0)
        {
          Wire.requestFrom(mod.id, mod.msg_len); 
          if(Wire.available() == mod.msg_len)
          {
              String msg;
              while(Wire.available())
                msg.concat(Wire.read());
                
              if(mod.last_msg != msg)
              {
                 Serial.print("module changed, sending state [");
                 Serial.print(msg);
                 Serial.println("]");
                 mod.last_msg = msg;
                 char msg_str[mod.msg_len];
                 msg.toCharArray(msg_str, mod.msg_len);
                 client.publish("/modbox/modchange", msg_str);
              }
              else
                Serial.println("no change in module");
          }
          else
            Serial.println("message timeout");
        }
    }
  }
  client.loop();
}
