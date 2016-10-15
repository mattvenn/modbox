#ifndef button_h
#define button_h

#include <Wire.h>
#include <Arduino.h>

// fix these names
#define UPDATE_LEN 1
#define STATE_LEN 1

//pins
#define LED 25 //PA2
#define BUTTON 12

class ButtonHW
{
    private:
        void encode_msg();
        void decode_msg();
        uint8_t state[STATE_LEN];
        char message[UPDATE_LEN];
        char id;
        boolean button;
    public:
        ButtonHW(char id);
        void get_msg(int length);
        void send_msg();
        void update();
        void setup();
        char get_id();
};

#endif
