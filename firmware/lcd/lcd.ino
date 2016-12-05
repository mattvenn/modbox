#include <LiquidCrystal.h>
#include <Wire.h>

#define BIT_SET(a,b) ((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))
#define BIT_CHECK(a,b) ((a) & (1<<(b)))

// TODO set ID should be eeprom
#define ID 2
// 1 byte for id, 1 byte for control, 16 bytes for a row
#define SETMOD_MSG_LEN 18

const int led_pwm = 9;
const int rs = 14;
const int en = 15;
const int d4 = 5;
const int d5 = 6;
const int d6 = 7;
const int d7 = 8;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte setmod_msg[SETMOD_MSG_LEN];

const int sw = 19;
int messages = 0;
int errors = 0;

volatile bool update = false;


// TODO check if ID is required in the message
void get_setmod_msg(int len)
{
    // test len
    if(len != SETMOD_MSG_LEN)
    {
        errors ++;
        return;
    }
    // read the message
    for(int i=0; i < SETMOD_MSG_LEN; i++)
        setmod_msg[i] = Wire.read();

    // check id
    if(setmod_msg[0] != ID)
    {
        errors ++;
        return;
    }
    update = true;
}

void setup()
{
    TCCR1B = _BV(CS10) | _BV(WGM12);

    lcd.begin(16,2);
    lcd.print("modbox - lcd");

    // fade in
    for(int i = 0; i < 100; i ++)
    {
        analogWrite(led_pwm, i);
        delay(1);
    }

    // id set switch - sometimes presses itself
    pinMode(sw, INPUT);

    // join i2c bus
    Wire.begin(ID);

    Wire.onReceive(get_setmod_msg);
}


void loop()
{
/*
    if(digitalRead(sw))
    {
        count ++;
        if(count > 200)
            count = 0;
        id = count / 50;
        // capacitance on pin keeps it pressed, so turn it into a low output for a moment
        pinMode(sw, OUTPUT);
        digitalWrite(sw, false);
        pinMode(sw, INPUT);
    }
    */
    /*
    lcd.setCursor(0, 1);
    lcd.print(millis());
    delay(1000 / 8);
    */

    if(update)
    {
        update = false;
        int row = setmod_msg[1];
        messages ++;

        lcd.setCursor(0, row);
        for(int i=2; i< SETMOD_MSG_LEN; i++)
            lcd.print((char)setmod_msg[i]);
    }
}

