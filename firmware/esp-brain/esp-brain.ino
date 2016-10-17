#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include "secrets.h"

IPAddress server(192,168,1,106);
WiFiClient wclient;
PubSubClient client(wclient, server);

char button = 0;

//client callback for MQTT subscriptions
void callback(const MQTT::Publish& pub) {
  Serial.print(pub.topic());
  Serial.print("=");
  Serial.println(pub.payload_string());
  if(pub.topic() == "/modbox/led")
  {
    int value = pub.payload_string().toInt();
    //send it on the the led
      Serial.print("send i2c = ");
      Serial.println(value);
      Wire.beginTransmission(8); // transmit to device #8
      Wire.write(value);       
      Wire.endTransmission();    // stop transmitting
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
            client.subscribe("/modbox/led"); 
            client.publish("/modbox/startup", String(millis()));
          }
      }
      // if got an update, publish it
  Wire.requestFrom(8, 1); 
  if (Wire.available() == 1) { // slave may send less than requested
    char c = Wire.read(); // receive a byte as character
    if(c != button)
    {
        button = c;
      Serial.print("publish button = ");
      Serial.println(button, HEX);
      client.publish("/modbox/button", String(button, HEX));
      }
  }
  }
  client.loop();
}
