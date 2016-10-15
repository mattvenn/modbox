#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include "secrets.h"

IPAddress server(192,168,1,241);
WiFiClient wclient;
PubSubClient client(wclient, server);

#include "Button.h"

Button butt(8, client);

//client callback for MQTT subscriptions
void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print(topic);
  //register a module
  /*
  if(topic == "/modbox/register")
  {
    int id = payload[0];
    char * type = payload[1];
    if type == knob: 
        modules = knob(id, client);
  }
  */
  //update a module
  if(topic == "/modbox/update")
  {
    int id = payload[0];
    if(butt.get_id() == id)
        butt.update(payload, length);
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

  //client callback for MQTT subscriptions
  client.set_callback(callback);
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
            client.subscribe("/modbox/mod"); 
          }
      }

      // if got an update, publish it
  /*
    for module in modules:
        module.update()
        if module.changed():
            module.publish_change()
  */

  if(butt.changed())
    butt.publish();

  client.loop();
}
