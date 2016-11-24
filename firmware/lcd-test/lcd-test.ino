
/*

pd2 -> 2/int0
pd1 -> 1
pd0 -> 0
pc5 -> scl
pc4 -> sda
pc3 -> 20/A3
pc2 -> 19/A2
pc1 -> 18/A1
pc0 -> 17/A0
pc7 -> 16
pa0 -> 23/A6
pa1 -> 24/A7
pb5 -> 13/sck
pb4 -> 12/miso
pb3 -> 11/mosi
pb2 -> 10/ss
pb1 -> 9
pb0 -> 8
pd7 -> 7
pd6 -> 6
pd5 -> 5
pb7 -> 15
pb6 -> 14
pa3 -> 26
pa2 -> 25
pd4 -> 4
pd3 -> 3/int1

*/
// include the library code:
#include <LiquidCrystal.h>


const int led_pwm = 9;
const int rs = 14;
const int en = 15;
const int d4 = 5;
const int d5 = 6;
const int d6 = 7;
const int d7 = 8;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int sw = 19;

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

    pinMode(sw, INPUT);
//    digitalWrite(sw, false);

}

int count = 0;
int id = 0;

void loop()
{
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
    lcd.setCursor(0, 1);
    lcd.print(millis() / 10);
    lcd.setCursor(8, 1);
    lcd.print("id:");
    lcd.print(id);
}
