#include "button.h"
#include <Wire.h>
#define ID 8

ButtonHW button(ID);

void receive_callback(int len)
{
    button.get_msg(len);
}

void request_callback()
{
    button.send_msg();
}

void setup()
{
    button.setup();
    Wire.begin(button.get_id());
    Wire.onRequest(request_callback);
    Wire.onReceive(receive_callback);
}

void loop()
{
    button.update();
}
