#define ID 1
#define STATE_MSG_LEN 2
#define SETMOD_MSG_LEN 2

#define LED 25 //PA2
#define BUTTON 12

#include <Wire.h>

byte modchange_msg[STATE_MSG_LEN];
byte setmod_msg[SETMOD_MSG_LEN];

void get_setmod_msg(int len)
{
    // test len
    if(len != SETMOD_MSG_LEN)
        return;
    // read the message
    for(int i=0; i < SETMOD_MSG_LEN; i++)
        setmod_msg[i] = Wire.read();
    // check id
    if(setmod_msg[0] != ID)
        return;
    // update module
    if(setmod_msg[1] == 1)
        digitalWrite(LED, HIGH);
    else
        digitalWrite(LED, LOW);
}

void send_modchange_msg() 
{
    Wire.write(modchange_msg, STATE_MSG_LEN); 
}

void setup()
{
    pinMode(LED,OUTPUT);
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);

    pinMode(BUTTON,INPUT);
    digitalWrite(BUTTON,HIGH);

    Wire.begin(ID);                // join i2c bus with address #8
    modchange_msg[0] = ID;
    modchange_msg[1] = 0;
    Wire.onReceive(get_setmod_msg); // register event
    Wire.onRequest(send_modchange_msg); // register event
}

void loop()
{
    if(digitalRead(BUTTON) == LOW)
        modchange_msg[1] = 0x01;
    else
        modchange_msg[1] = 0x00;
}
