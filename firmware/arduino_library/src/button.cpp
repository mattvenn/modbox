// h file
#include "button.h"

Button::Button(char id, PubSubClient & client)
{
    this->client = &client;
    this->id = id;
}

void Button::update(byte *data, unsigned int length)
{
    Wire.beginTransmission(this->id);
    // TODO skip a byte
    Wire.write(data, length);
    Wire.endTransmission();
}

boolean Button::changed()
{
  Wire.requestFrom(this->id, UPDATE_LEN); 
  if (Wire.available() == UPDATE_LEN)
  {
    char c = Wire.read();
    if((boolean)c != this->button)
    {
        this.button = (boolean)c;
        return true;
    }
    return false;
}

boolean Button::publish()
{
      Serial.print("publish button = ");
      Serial.println(this->button, BIN);
      return this->client.publish("/modbox/update/" + this->id, String(this->button));
}

char Button::get_id()
{
    return this->id;
}
