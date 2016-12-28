#!/usr/bin/python
import paho.mqtt.client as mqtt
from modules import Button, Knobs, LCD
import socket # for error handling
import struct
import json
import time
import sys
import initial_state #import read, write
from store import store
from actions import actions
from functools import partial
from initial_state import playback_items

from mpd import MPDClient, ProtocolError, ConnectionError
import random

import logging

log_format = logging.Formatter('%(asctime)s - %(name)-16s - %(levelname)-8s - %(message)s')
# configure the client logging
log = logging.getLogger('')
# has to be set to debug as is the root logger
log.setLevel(logging.INFO)

# create console handler and set level to info
ch = logging.StreamHandler()
# create formatter for console
ch.setFormatter(log_format)
log.addHandler(ch)


def on_connect(client, userdata, flags, rc):
    log.info("Connected with result code "+str(rc))

def on_publish(client, userdata, mid):
    log.debug("sent msg id %s to client" % (mid))

def on_message(client, userdata, message):
    log.debug("Received message on topic " + message.topic)
    if message.topic == '/modbox/start':
        for module in modules:
            module.register()
        for module in modules:
            module.post_register()
        handle_changes(force_update=True)
        # TODO needs 2
        handle_changes(force_update=True)

    elif message.topic == '/modbox/modchange':
        log.debug("mod change message")
        # get the id
        mod_id, = struct.unpack("<B", message.payload[0])
        # find the right module
        for module in modules:
            if module.id == mod_id:
                module.action(message.payload)
                break
        else:
            print("no module with that id")
    elif message.topic == '/modbox/battery':
        v_in = int(message.payload) * 1.0 / 1023
        R1 = 2400.0
        R2 = 10000.0
        batt_level = v_in / (R1 / (R1+R2))
        batt_level = round(batt_level, 2)
        store.dispatch(actions.update_battery(batt_level))
    else:
        print("no handler for that topic")

# open config
with open("config.json") as fh:
    config = json.load(fh)

client = mqtt.Client()
client.on_connect = on_connect
client.on_publish = on_publish
client.on_message = on_message

# build a list of module objects
modules = []
for module in config:
    if module['type'] == 'button':
        modules.append(Button(module['id'], client))
    elif module['type'] == 'knobs':
        modules.append(Knobs(module['id'], client))
    elif module['type'] == 'lcd':
        modules.append(LCD(module['id'], client))


client.connect("127.0.0.1", 1883, 60)
time.sleep(0.1)
client.subscribe("/modbox/start")  # when modbox starts, this will cause all modules to be registered
client.subscribe("/modbox/modchange") # whenever a control changes
client.subscribe("/modbox/battery") # whenever a control changes
client.loop_start()

# mpd client
mpdclient = None

def connect_mpd():
    global mpdclient
    mpdclient = MPDClient()
    mpdclient.connect("192.168.1.241", 6600)  

connect_mpd()

playlists = [pl['playlist'] for pl in mpdclient.listplaylists()] 

store.dispatch(actions.init(initial_state.read(playlists)))

# force a reset
client.publish("/modbox/reset", "", qos=2)

display = ['','']

def handle_changes(force_update = False):
    try:
        mpdclient.ping()
    except ConnectionError:
        log.warning("mpd connection error")
        connect_mpd()
    except socket.error:
        log.warning("socket error: reconnecting to mpd")
        connect_mpd()

    state = store.get_state()
    log.debug(state)

    if state['display'][0] != display[0] or force_update:
        lcd_updated = True
        display[0] = state['display'][0]
        modules[1].update(state['display'][0],0)

    # hack because 2 consecutive lcd messages can break screen
    # should only do this if state actually changed
    modules[0].update(state['knob1_leds'], state['knob2_leds'], state['but1_led'], state['but2_led'])

    if state['display'][1] != display[1] or force_update:
        display[1] = state['display'][1]
        modules[1].update(state['display'][1],1)
    initial_state.write(state)

    if state['change_playback']:
        if playback_items[state['playback_menu_id']] == 'clear':
            mpdclient.clear()
        if playback_items[state['playback_menu_id']] == 'stop':
            mpdclient.stop()
        if playback_items[state['playback_menu_id']] == 'play':
            mpdclient.play()
        if playback_items[state['playback_menu_id']] == 'skip':
            mpdclient.next()
        #print(client.next()) # print result of the command "find any house"
        store.dispatch(actions.playback_changed(mpdclient.status()['state']))
        
    if state['change_playlist']:
        mpdclient.clear()
        if state['add_menu_items'][state['add_menu_id']] == 'random album':
            try:
                albums = mpdclient.list('album')
                log.debug("found %d albums" % len(albums))
                album = random.choice(albums)
                log.info("adding random album %s" % album)
                mpdclient.findadd('album', album)
            except ProtocolError:
                log.warning("mpd protocol error")
                pass

        else:
            playlist_name = state['add_menu_items'][state['add_menu_id']] 
            log.info("adding %s" % playlist_name)
            mpdclient.load(playlist_name)

        mpdclient.play()

        try:
            currently_playing = mpdclient.currentsong()['album']
        except KeyError:
            currently_playing = 'no playlist'

        store.dispatch(actions.playlist_changed(currently_playing))

    if state['change_volume']:
        mpdclient.setvol(state['volume'])
        store.dispatch(actions.volume_changed())
       
        

store.subscribe(partial(handle_changes))

while True:
    time.sleep(0.5)
