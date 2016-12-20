http://redux.js.org/docs/basics/Reducers.html

# for remote control

## Actions

* knob1 inc
* knob1 dec
* knob2 inc
* knob2 dec
* button1 release
* button2 release

## State 

Initial state shown in ()

* current volume = (read mpd current volume)
* what menu page id are we on (play random album):
    * volume,
    * play random album,
    * playback control,
    * play fip
* lcd lines:
    * 0
    * 1
* leds:
    * knob1
    * knob2
    * button1
    * button2
* current playing track - a string (read mpd current track)
* playback menu id (play)
    * play
    * pause
    * stop 
* playback status (read mpd current status ):

# Reducers

* knob1 inc -> increment menu page id
* knob1 dec -> decrement menu page id
* knob2 inc,
    * if menu == vol -> inc vol
    * if menu == playback -> inc playback menu id
* knob2 dec,
    * if menu == vol -> dec vol
    * if menu == playback -> dec playback menu id
* button1 release
    * if menu == playback -> change playback status to menu id

# Subscribe

    for display in lcd0, lcd1, knob1leds ... etc:
        if display != last display:
            send update
    write the state?
