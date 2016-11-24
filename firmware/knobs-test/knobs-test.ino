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

// 0
const int knob1_sw_led = 25;
// 1 -> 7
const int knob1_leds[7] = {26, 14, 15, 5, 6, 7, 8};
// 8
const int knob2_sw_led = 23;
// 9 -> 15
const int knob2_leds[7] = {16, 24, 17, 18, 19, 20, 2};

const int led_pwm = 9;

const int enc2_a = 0; 
const int enc2_b = 1;
const int enc2_s = 12;

const int enc1_a = 3;
const int enc1_b = 4;
const int enc1_s = 11;

void setup()
{
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

    TCCR1B = _BV(CS10) | _BV(WGM12);

    // fade in
    for(int i = 0; i < 100; i ++)
    {
        analogWrite(led_pwm, i);
        delay(1);
    }
}

bool val = true;
int lastEncoded_1;
int lastEncoded_2;
uint16_t encoderValue_1;
uint16_t encoderValue_2;

void loop()
{
    /*
    //led test
    for(int pin = 0; pin < 7; pin ++)
    {
        digitalWrite(knob1_leds[pin], val);
        digitalWrite(knob2_leds[pin], val);
        delay(100);
    }
    digitalWrite(knob1_sw_led, val);
    delay(100);
    digitalWrite(knob2_sw_led, val);
    delay(100);
    val = ! val;
    */

    //switch test
    /*
    if(digitalRead(enc1_s) == false)
        digitalWrite(knob1_sw_led, true);
    else
        digitalWrite(knob1_sw_led, false);

    if(digitalRead(enc2_s) == false)
        digitalWrite(knob2_sw_led, true);
    else
        digitalWrite(knob2_sw_led, false);

    if(digitalRead(enc1_a))
        digitalWrite(knob1_leds[1], true);
    else
        digitalWrite(knob1_leds[1], false);

    if(digitalRead(enc1_b))
        digitalWrite(knob1_leds[2], true);
    else
        digitalWrite(knob1_leds[2], false);

    if(digitalRead(enc2_a))
        digitalWrite(knob2_leds[1], true);
    else
        digitalWrite(knob2_leds[1], false);

    if(digitalRead(enc2_b))
        digitalWrite(knob2_leds[2], true);
    else
        digitalWrite(knob2_leds[2], false);
    */
    
    // encoder test
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
    for(int i = 0; i < 7; i++)
    {
        if((1 << i) >  encoderValue_1)
            digitalWrite(knob1_leds[i], true);
        else
            digitalWrite(knob1_leds[i], false);
    }

    MSB = digitalRead(enc2_a); //MSB = most significant bit
    LSB = digitalRead(enc2_b); //LSB = least significant bit
 
    encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
    sum  = (lastEncoded_2 << 2) | encoded; //adding it to the previous encoded value
 
    if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
        encoderValue_2 ++;
    if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
        encoderValue_2 --;
 
    lastEncoded_2 = encoded; //store this value for next time
    for(int i = 0; i < 7; i++)
    {
        if((1 << i) >  encoderValue_2)
            digitalWrite(knob2_leds[i], true);
        else
            digitalWrite(knob2_leds[i], false);
    }

    // encoder buttons
    if(digitalRead(enc1_s) == false)
        digitalWrite(knob1_sw_led, true);
    else
        digitalWrite(knob1_sw_led, false);

    if(digitalRead(enc2_s) == false)
        digitalWrite(knob2_sw_led, true);
    else
        digitalWrite(knob2_sw_led, false);
}
