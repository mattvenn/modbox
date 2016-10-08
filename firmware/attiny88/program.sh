avrdude -p t88 -P /dev/ttyACM0 -c STK500 -B 10  -v -U flash:w:$1:i
