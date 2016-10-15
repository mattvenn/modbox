#include "button.h"

ButtonHW::ButtonHW(char id)
{
    this->id = id;
}

char ButtonHW::get_id()
{
    return this->id;
}

void ButtonHW::setup()
{
    pinMode(LED,OUTPUT);
    digitalWrite(LED, true);
    delay(100);
    digitalWrite(LED, false);

    pinMode(BUTTON,INPUT);
    digitalWrite(BUTTON,true);
}

void ButtonHW::update()
{
    // TODO debounce
    if(digitalRead(BUTTON))
        this->button = true;
    else 
        this->button = false;
}

void ButtonHW::encode_msg()
{
   this->state[0] = this->button; 
}

void ButtonHW::decode_msg()
{
    if(this->message[0])
        digitalWrite(LED, true);
    else
        digitalWrite(LED, false);
}
//virtual
void ButtonHW::send_msg()
{
    this->encode_msg();
    Wire.write(this->state, STATE_LEN);
}

//virtual
void ButtonHW::get_msg(int length)
{
    for(int i=0; i<UPDATE_LEN; i++)
        this->message[i] = Wire.read();
    this->decode_msg();
} 

