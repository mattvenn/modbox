/*

new features
+ wifi status on led

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

#define TICK 5000
#define SHUTDOWN 10000
unsigned long last_tick = 0;
#define I2C_CHECK 20
unsigned long last_i2c_check = 0;

#define DEBUG_MQTT
#define DEBUG_I2C

const int button_led = 15;
const int usb_on = 14;
const int button = 16;
const int power_on = 13;
const int i2c_bus_power = 12;
const int ADC = A0;

char message_buff[100];
double last_updated = 0;

//client callback for MQTT subscriptions
void callback(char* topic, byte* payload, unsigned int len) 
{
  digitalWrite(button_led, false);
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
  digitalWrite(button_led, true);
//  last_updated = millis(); // TODO fix server unnecessary updates
}

void setup() 
{
    pinMode(power_on, OUTPUT);
    digitalWrite(power_on, true);

    pinMode(i2c_bus_power, OUTPUT);
    digitalWrite(i2c_bus_power, true); // low to power on

    pinMode(button_led, OUTPUT);
    pinMode(usb_on, INPUT);
    pinMode(button, INPUT);

  Wire.begin(); // join i2c bus (address optional for master)
  Wire.setClock(50000);
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

    Serial.print("i2c bus...");
    digitalWrite(i2c_bus_power, false); // low to power on
    Serial.println("on");
}


void loop() 
{
    /* shutdown code */
    if(digitalRead(usb_on) == false) // powered from battery
        if(millis() - last_updated > SHUTDOWN)
        {
            Serial.println("shutdown");
            digitalWrite(power_on, false);
        }

  if(millis() - last_tick > TICK)
  {
    Serial.println(millis() - last_updated);
    Serial.println(digitalRead(usb_on));
    last_tick = millis();
    /*
    Serial.print("adc=");
    Serial.println(analogRead(ADC));
    Serial.println(client.connected());
    */

    String pubString = String(analogRead(ADC));
    pubString.toCharArray(message_buff, pubString.length()+1);
    client.publish("/modbox/battery", message_buff);
  }
  if (WiFi.status() != WL_CONNECTED) 
  {
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) 
    {
       delay(50);
       digitalWrite(button_led, false);
       delay(50);
       digitalWrite(button_led, true);
    }

    Serial.println("WiFi connected");
    WiFi.mode(WIFI_STA);
    Serial.println(host);
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

    if(millis() - I2C_CHECK > last_i2c_check)
    {
        last_i2c_check = millis();
    for(int i = 0; i < num_modules; i++)
    {
        MODULE mod = modules[i];
        if(mod.modchange_msglen > 0)
        {
 //         Serial.print("requesting update from module id:");
//          Serial.println(mod.id);
          Wire.requestFrom(mod.id, mod.modchange_msglen); 
          delay(1); // also opportunity for esp to catch up on wifi
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
                 digitalWrite(button_led, false);
                 client.publish("/modbox/modchange", mod.msg, mod.modchange_msglen);
                 digitalWrite(button_led, true);
                 last_updated = millis();
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
  }
  // putting this in seems to solve the errors on i2c
    client.loop();
}
