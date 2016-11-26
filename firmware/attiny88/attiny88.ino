#include <Wire.h>

#define BIT_SET(a,b) ((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))
#define BIT_CHECK(a,b) ((a) & (1<<(b)))

// TODO set ID should be eeprom
#define ID 1
// 1 byte for id, 1 byte for each knob, 1 byte for both buttons
#define MODCHANGE_MSG_LEN 4 
// 1 byte for id, 1 byte for each set of knob 7 leds, 1 byte for both but leds
#define SETMOD_MSG_LEN 4

// 0
const int knob1_sw_led = 25;
// 1 -> 7
const int knob1_leds[7] = {8, 7, 6, 5, 15, 14, 26};
// 8
const int knob2_sw_led = 23;
// 9 -> 15
const int knob2_leds[7] = {2, 20, 19, 18, 17, 24, 16};

const int led_pwm = 9;

const int enc2_a = 0; 
const int enc2_b = 1;
const int enc2_s = 12;

const int enc1_a = 3;
const int enc1_b = 4;
const int enc1_s = 11;

int lastEncoded_1;
int lastEncoded_2;
uint8_t encoderValue_1;
uint8_t encoderValue_2;

byte modchange_msg[MODCHANGE_MSG_LEN];
byte setmod_msg[SETMOD_MSG_LEN];

// TODO check if ID is required in the message
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

    // update module : knob leds
    for(int i = 0; i < 7; i++)
    {
        if(BIT_CHECK(setmod_msg[1], i))
            digitalWrite(knob1_leds[i], true);
        else
            digitalWrite(knob1_leds[i], false);

        if(BIT_CHECK(setmod_msg[2], i))
            digitalWrite(knob2_leds[i], true);
        else
            digitalWrite(knob2_leds[i], false);
    }

    // but leds
    if(BIT_CHECK(setmod_msg[3], 0))
        digitalWrite(knob1_sw_led, true);
    else
        digitalWrite(knob1_sw_led, false);

    if(BIT_CHECK(setmod_msg[3], 1))
        digitalWrite(knob2_sw_led, true);
    else
        digitalWrite(knob2_sw_led, false);
}

void send_modchange_msg() 
{
    Wire.write(modchange_msg, MODCHANGE_MSG_LEN); 
}

void setup()
{
    // setup pins
    for(int pin = 0; pin < 7; pin ++)
    {
        pinMode(knob1_leds[pin], OUTPUT);
        pinMode(knob2_leds[pin], OUTPUT);
        // have them all on to start
        digitalWrite(knob1_leds[pin], HIGH);
        digitalWrite(knob2_leds[pin], HIGH);
    }
    pinMode(knob1_sw_led, OUTPUT);
    pinMode(knob2_sw_led, OUTPUT);

    pinMode(enc1_a, INPUT);
    pinMode(enc1_b, INPUT);
    pinMode(enc1_s, INPUT);
    digitalWrite(enc1_s, true);

    pinMode(enc2_a, INPUT);
    pinMode(enc2_b, INPUT);
    pinMode(enc2_s, INPUT);
    digitalWrite(enc2_s, true);

    // pwm freq
    TCCR1B = _BV(CS10) | _BV(WGM12);

    // fade in
    for(int i = 0; i < 100; i ++)
    {
        analogWrite(led_pwm, i);
        delay(1);
    }

    // join i2c bus
    Wire.begin(ID);
    modchange_msg[0] = ID;
    modchange_msg[1] = 0;
    Wire.onReceive(get_setmod_msg);
    Wire.onRequest(send_modchange_msg);
}

void loop()
{
    // buttons
    if(digitalRead(enc1_s) == false)
        BIT_SET(modchange_msg[3],0);
    else
        BIT_CLEAR(modchange_msg[3],0);

    if(digitalRead(enc2_s) == false)
        BIT_SET(modchange_msg[3],1);
    else
        BIT_CLEAR(modchange_msg[3],1);

    // encoders
    // https://thewanderingengineer.com/2013/05/05/rotary-encoder-with-the-attiny85/
    int MSB = digitalRead(enc1_a); //MSB = most significant bit
    int LSB = digitalRead(enc1_b); //LSB = least significant bit
 
    int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
    int sum  = (lastEncoded_1 << 2) | encoded; //adding it to the previous encoded value
 
    if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
        encoderValue_1 ++;
    if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
        encoderValue_1 --;
 
    lastEncoded_1 = encoded; //store this value for next time

    modchange_msg[1] = encoderValue_1;

    MSB = digitalRead(enc2_a); //MSB = most significant bit
    LSB = digitalRead(enc2_b); //LSB = least significant bit
 
    encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
    sum  = (lastEncoded_2 << 2) | encoded; //adding it to the previous encoded value
 
    if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
        encoderValue_2 ++;
    if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
        encoderValue_2 --;
 
    lastEncoded_2 = encoded; //store this value for next time

    modchange_msg[2] = encoderValue_2;
}
